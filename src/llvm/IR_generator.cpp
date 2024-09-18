/**
 * Copyright (c) 2023 Rusheng Xia <xrsh_2004@163.com>
 * CA Programming Language and CA Compiler are licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 */

#include "llvm/BinaryFormat/Dwarf.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <assert.h>
#include <cassert>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <sys/time.h>
#include <utility>
#include <vector>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "ca_parser.h"
#include "ca_types.h"
#include "type_system.h"
#include "type_system_llvm.h"
#include "ca_runtime.h"
#include "config.h"
#include "symtable.h"
#include "symtable_cpp.h"

// llvm section
#include "ir1.h"
#include "jit1.h"
#include "dwarf_debug.h"
#include "IR_generator.h"

#define MANGLED_NAME_PREFIX "_CA$"

BEGIN_EXTERN_C
#include "ca.tab.h"
CompileEnv genv;
END_EXTERN_C

using namespace llvm;

enum OperandType {
  OT_Const,
  OT_Calc,
  OT_Load,
  OT_Store,    // not used yet
  OT_Alloc,
  OT_CallInst, // use as return value
  OT_PHINode,  // not used yet
  OT_HeapAlloc,
};

struct CalcOperand {
  CalcOperand() : type(OT_Alloc), operand(nullptr), catype(nullptr) {}

  CalcOperand(OperandType t, llvm::Value *v, CADataType *dt)
      : type(t), operand(v), catype(dt) {}

  OperandType type;
  CADataType *catype;
  llvm::Value *operand;
};

enum LexicalType {
  LT_Global,
  LT_Block,
  LT_Module,
  LT_Function,
  LT_Struct,
  LT_Num,
};

struct LexicalScope {
  LexicalScope() : discope(nullptr), difn(nullptr),
		   lexical_id(0), lexical_type(LT_Global)
  {
    u.function_name = 0;
  }

  llvm::DIScope *discope;
  llvm::DISubprogram *difn;
  SymTable *symtable; // for storing corresponding SymTable object
  int lexical_id; // the globally unique id for this compile unit, used to create unique global function (or may be variable) name
  LexicalType lexical_type;
  union {
    typeid_t function_name;
    typeid_t module_name;
    typeid_t struct_name;
  } u;
};

struct LoopControlInfo {
  enum LoopType {
    LT_Loop,
    LT_While,
    LT_For,
  };

  LoopControlInfo(LoopType looptype, int name, llvm::BasicBlock *condbb,
                  llvm::BasicBlock *outbb)
      : looptype(looptype), name(name), condbb(condbb), outbb(outbb) {}

  LoopType looptype;
  int name;
  llvm::BasicBlock *condbb;
  llvm::BasicBlock *outbb;
};

struct FnDebugInfo {
  DISubprogram *disp;
};

ir_codegen::IR1 ir1;

extern SymTable g_root_symtable;
extern ASTNode *main_fn_node;

// first walk for iterating function prototype into llvm object
// second walk for iterating all tree nodes
static int walk_pass = 0;
static std::unique_ptr<dwarf_debug::DWARFDebugInfo> diinfo;
static std::unique_ptr<jit_codegen::JIT1> jit1;
static ExitOnError exit_on_error;
static bool g_with_ret_value = false;
static int curr_lexical_count = 0;

// llvm section

// note: rust not support different type variable to do
// calculation, so add `as` to convert between type
// 1. mathimatic calculation must need the same type or error
// 2. the literal type have 2 forms:
//   a. with postfix to indicate it's type,
//   b. without postfix, just consider it's value when do calculation. example
//   432432 means integer value (any integer value), -43433 mean an Neg bound to
//   43433, 33.33 mean f64 value (default is f64 not f32 value)
// 
//   
// 
// So I need to seperate this two condition
//

static llvm::Function *g_box_fn = nullptr;
static llvm::Function *g_drop_fn = nullptr;
static llvm::Function *main_fn = nullptr;

// handle when processing current function, the top level function is main function
static llvm::Function *curr_fn = nullptr;
static ASTNode *curr_fn_node = nullptr;
struct CurrFnInfo {
  CurrFnInfo(llvm::Function *fn, ASTNode *node) :
    llvm_fn(fn), fn_node(node) {
  }

  llvm::Function *llvm_fn;
  ASTNode *fn_node;
};

static std::vector<CurrFnInfo> curr_fn_stack;
static llvm::BasicBlock *main_bb = nullptr;
static llvm::DIFile *diunit = nullptr;
static std::vector<std::unique_ptr<CalcOperand>> oprand_stack;

// for storing defined BasicBlock, or pre-define BasicBlock in GOTO statement
static std::map<std::string, BasicBlock *> label_map;
static std::map<std::string, ASTNode *> function_map;

// TODO: should here using a current debug info instead of the map, it no need
// to use a map here, because it only used when function define, just like
// curr_fn
static std::map<Function *, std::unique_ptr<FnDebugInfo>> fn_debug_map;

std::vector<std::unique_ptr<LexicalScope>> lexical_scope_stack;
static LexicalScope *curr_lexical_scope = nullptr;
static LexicalScope *root_lexical_scope = nullptr;

struct GenericTypeVarInfo {
  GenericTypeVarInfo(typeid_t *datatype_addr, typeid_t old_value,
                     typeid_t new_value)
    : datatype_addr(datatype_addr), old_value(old_value), new_value(new_value)
  {
  }

  typeid_t *datatype_addr;
  typeid_t old_value;
  typeid_t new_value;
};

typedef std::map<typeid_t *, GenericTypeVarInfo> generic_type_var_set_t;

static std::vector<std::pair<SymTableAssoc *, generic_type_var_set_t>> generic_type_stack;

// for handling function parameter checking
static std::unordered_map<typeid_t, void *> g_function_post_check_map;

static std::vector<std::unique_ptr<LoopControlInfo>> g_loop_controls;

const static char *box_fn_name = "GC_malloc";
const static char *drop_fn_name = "GC_free";

static int walk_stack(ASTNode *p);

std::vector<ASTNode *> *arrayexpr_deref(CAArrayExpr obj);
std::vector<void *> *structexpr_deref(CAStructExpr obj);

static void init_printf_fn() {
  // TODO: add grammar for handling extern functions instead hardcoded here
  Function *printf_fn = ir1.module().getFunction("printf");
  if (!printf_fn) {
    auto param_names = std::vector<const char *>(1, "s");
    printf_fn = ir1.gen_extern_fn(ir1.int_type<int>(), "printf",
				  std::vector<Type *>(1, ir1.intptr_type<char>()),
				  &param_names,	true);
    printf_fn->setCallingConv(CallingConv::C);

    //AttrListPtr func_printf_PAL;
    //printf_fn->setAttributes(func_printf_PAL);
  }
}

static void init_box_fn() {
  // void *malloc(size_t size);
  Function *box_fn = ir1.module().getFunction(box_fn_name);
  if (!box_fn) {
    auto param_names = std::vector<const char *>(1, "size");
    box_fn = ir1.gen_extern_fn(ir1.voidptr_type(), box_fn_name,
			       std::vector<Type *>(1, ir1.int_type<size_t>()),
			       &param_names, false);
    box_fn->setCallingConv(CallingConv::C);

    //AttrListPtr func_printf_PAL;
    //box_fn->setAttributes(func_printf_PAL);
  }

  g_box_fn = box_fn;
}

static void init_drop_fn() {
  // void free(void *ptr);
  Function *drop_fn = ir1.module().getFunction(drop_fn_name);
  if (!drop_fn) {
    auto param_names = std::vector<const char *>(1, "ptr");
    drop_fn = ir1.gen_extern_fn(ir1.void_type(), drop_fn_name,
			       std::vector<Type *>(1, ir1.voidptr_type()),
			       &param_names, false);
    drop_fn->setCallingConv(CallingConv::C);

    //AttrListPtr func_printf_PAL;
    //drop_fn->setAttributes(func_printf_PAL);
  }

  g_drop_fn = drop_fn;
}

static void initialize_inner_functions() {
  init_printf_fn();
  //init_box_fn();
  //init_drop_fn();
}

static Value *llvmcode_box(Value *sizev, Type *type = nullptr) {
  std::vector<Value *> params(1, sizev);
  if (!g_box_fn)
    init_box_fn();

  Value *callret = ir1.builder().CreateCall(g_box_fn, params, "heap");
  if (type) {
    callret = ir1.gen_cast_value(ICO::BitCast, callret, type, "ptrcast");
  }

  return callret;
}

static Value *llvmcode_box(size_t size, Type *type = nullptr) {
  Value *sizev = ir1.gen_int<size_t>(size);
  return llvmcode_box(sizev, type);
}

static Value *llvmcode_box(Value *sizev, CADataType *catype = nullptr) {
  Type *type = nullptr;
  if (catype)
    type = llvmtype_from_catype(catype);

  return llvmcode_box(sizev, type);
}

static Value *llvmcode_box(size_t size, CADataType *catype = nullptr) {
  Value *sizev = ir1.gen_int<size_t>(size);
  return llvmcode_box(sizev, catype);
}

static Value *llvmcode_drop(Value *ptr) {
  if (!g_drop_fn)
    init_drop_fn();

  ptr = ir1.gen_cast_value(ICO::BitCast, ptr, ir1.voidptr_type(), "ptrcast");
  std::vector<Value *> params(1, ptr);
  Value *callret = ir1.builder().CreateCall(g_drop_fn, params);
  return callret;
}

// There are 2 options to implement the inner slice type
// 1. use a record (struct) Value
// 2. use 2 (or more) separate Value
// Here just select 1, because all needed information is packed into a record,
// and the content of record:
// The record content have 2 options
// option 1:
// struct {
//    ptr: *T, // ptr is the start address of the memory allocated by alloca or
//    heap offset: usize, // the offset of start address of slice: slice_offset
//    = ptr + offset len: usize,    // the length of the slice
// }
//
// option 2: the pointer is just the start address of slice not the allocated
// address, don't know if it can work
// struct {
//    ptr: *T,       // the ptr is the start address of slice directly
//    len: usize,    // the length of the slice
// }
//
static void aux_copy_llvmvalue_to_store(Type *type, Value *dest, Value *src, const char *name);
static Value *llvmcode_create_slice(Value *start, Value *offset, Value *len, CADataType *item_catype) {
  Value *slice_start = nullptr;

  if (offset) {
    std::vector<Value *> slice_idxv(1, offset);
    slice_start = ir1.builder().CreateInBoundsGEP(start, slice_idxv);
  } else {
    slice_start = start;
  }

  CADataType *slice_catype = slice_create_catype(item_catype);

  StructType *slice_type = static_cast<StructType *>(llvmtype_from_catype(slice_catype));

  // allocate new array and copy related elements to the array
  AllocaInst *slice_value = ir1.gen_entry_block_var(curr_fn, slice_type, "slice_value");

  Value *idxv0 = ir1.gen_int((int)0);
  std::vector<Value *> idxv(2, idxv0);

  // get elements address of slice_value
  Value *slice_start_dest = ir1.builder().CreateGEP(slice_value, idxv);
  Type *lefttype = slice_type->getStructElementType(0);

  slice_start = ir1.gen_cast_value(ICO::BitCast, slice_start, lefttype);
  aux_copy_llvmvalue_to_store(lefttype, slice_start_dest, slice_start, "slice_start");

  Value *idxvi = ir1.gen_int(1);
  idxv[1] = idxvi;

  Value *slice_len_dest = ir1.builder().CreateGEP(slice_value, idxv);
  Type *righttype = slice_type->getStructElementType(1);
  aux_copy_llvmvalue_to_store(righttype, slice_len_dest, len, "slice_len");

  return slice_value;
}

static void llvmcode_printf(Function *fn, const char *format, ...) {
  Constant *llvmformat = ir1.get_global_string(format);
  std::vector<Value *> params(1, llvmformat);

  Value *vv = nullptr;
  va_list ap;
  va_start(ap, format);
  while ((vv  = va_arg(ap, Value *)) != nullptr)
    params.push_back(vv);
  va_end(ap);

  ir1.builder().CreateCall(fn, params, "n");
}

static void llvmcode_printf_primitive(Function *fn, CADataType *catype, Value *v) {
  const char *format = get_printf_format(catype->type);
  v = tidy_value_with_arith(v, catype->type);
  llvmcode_printf(fn, format, v, nullptr);
}

static std::unique_ptr<CalcOperand> pop_right_operand(const char *name = "load", bool load = true) {
  std::unique_ptr<CalcOperand> o = std::move(oprand_stack.back());
  oprand_stack.pop_back();

  Value *v;
  if (load && o->type == OT_Alloc) {
    v = ir1.builder().CreateLoad(o->operand, name);
    o->type = OT_Load;
    o->operand = v;
  }

  return std::move(o);
}

static std::pair<Value *, CADataType *> pop_right_value(const char *name = "load", bool load = true) {
  std::unique_ptr<CalcOperand> o = std::move(oprand_stack.back());
  oprand_stack.pop_back();

  Value *v;
  if (load && o->type == OT_Alloc) {
    v = ir1.builder().CreateLoad(o->operand, name);
  } else {
    v = o->operand;
  }

  return std::make_pair(v, o->catype);
}

static int enable_debug_info() { return genv.emit_debug; }
static void varshielding_rotate_variable(CAVariableShielding *shielding, bool is_back = false);

static void init_fn_param_info(Function *fn, ST_ArgList &arglist, SymTable *st, int startrow) {
  int row = startrow;
  int scope_row = row;
  DISubprogram *disp = nullptr;

  if (enable_debug_info()) {
    DIScope *scope = diunit;
    llvm::DISubroutineType *fnty = diinfo->fn_type_di(fn->arg_size(), diunit);

    disp = diinfo->dibuilder->createFunction(scope, fn->getName(), StringRef(),
					     diunit, row, fnty, scope_row,
					     DINode::FlagPrototyped,
					     DISubprogram::SPFlagDefinition);
    fn->setSubprogram(disp);

    // push current scope
    diinfo->lexical_blocks.push_back(disp);

    // Unset the location for the prologue emission (leading instructions with no
    // location in a function are considered part of the prologue and the debugger
    // will run past them when breaking on a function)
    diinfo->emit_location(-1);

    auto dbginfo = std::make_unique<FnDebugInfo>();
    dbginfo->disp = disp;
    fn_debug_map.insert(std::make_pair(fn, std::move(dbginfo)));
  }

  int i = 0;
  for (auto &arg: fn->args()) {
    int argn = arglist.argnames[i];
    STEntry *entry = sym_getsym(st, argn, 0);

    /* should equal to arg.getName().str().c_str() */
    const char *name = symname_get(argn);

    // get the argument from call parameter into this field
    if (entry->sym_type != Sym_Variable)
      yyerror("symbol type is not variable: (%d != %d)", entry->sym_type, Sym_Variable);
#if 1 // TODO: check if can use function parameter directly without copying the parameter
    varshielding_rotate_variable(&entry->u.varshielding);
    CADataType *dt = catype_get_by_name(st, entry->u.varshielding.current->datatype);
    CHECK_GET_TYPE_VALUE(curr_fn_node, dt, entry->u.varshielding.current->datatype);

    Type *type = llvmtype_from_catype(dt);
    AllocaInst *slot = ir1.gen_var(type, name, &arg);

    if (enable_debug_info()) {
      DIType *ditype = diinfo->get_ditype(catype_get_type_name(dt->signature)); // get_type_string(dt->type)
      DILocalVariable *divar = diinfo->dibuilder->createParameterVariable(disp, arg.getName(), i, diunit, row, ditype, true);

      const DILocation *diloc = DILocation::get(disp->getContext(), row, 0, disp);
      diinfo->dibuilder->insertDeclare(slot, divar, diinfo->dibuilder->createExpression(),
				       diloc, ir1.builder().GetInsertBlock());
    }
#else
    Value *slot = &arg;
#endif
    // save the value into symbol table
    entry->u.varshielding.current->llvm_value = static_cast<void *>(slot);
    //varshielding_rotate_variable(&entry->u.varshielding, true);

    ++i;
  }
}

static DIType *ditype_get_or_create_from_catype(CADataType *catype, DIScope *scope);
static DIType *ditype_create_from_catype(CADataType *catype, DIScope *scope) {
  const char *name = nullptr;
  switch(catype->type) {
  case SLICE:
  case STRUCT: {
    const char *structname = symname_get(catype->struct_layout->name);

    // TODO: how to set the line number
    int lineno = 5;
    std::vector<Metadata *> fields;

    DINodeArray difields = diinfo->dibuilder->getOrCreateArray(fields);

    // not use lexical scope for struct definition, just use function or null when globally defined structure
    DICompositeType *pty = diinfo->dibuilder->createStructType(scope, structname, diunit, lineno, catype->size * 8, 0,
							       DINode::DIFlags::FlagZero, nullptr, difields);
    for (int i = 0; i < catype->struct_layout->fieldnum; ++i) {
      CAStructField &field = catype->struct_layout->fields[i];
      DIType *ditype = ditype_get_or_create_from_catype(field.type, scope);
      const char *fieldname = catype->struct_layout->type ? nullptr : symname_get(field.name);

      uint64_t offsetbit = field.offset * 8;
      uint64_t fieldsizebit = field.type->size * 8;
      DIDerivedType *dfield = diinfo->dibuilder->createMemberType(pty, fieldname, diunit, lineno, fieldsizebit,
								  0, offsetbit, DINode::DIFlags::FlagZero, ditype);
      fields.push_back(dfield);
    }

    difields = diinfo->dibuilder->getOrCreateArray(fields);
    pty->replaceElements(difields);
    return pty;
  }
  case ARRAY: {
    assert(catype->array_layout->dimension == 1);
    DIType *kernelty = ditype_get_or_create_from_catype(catype->array_layout->type, scope);
    DISubrange *subr = diinfo->dibuilder->getOrCreateSubrange(0, catype->array_layout->dimarray[0]);
    DINodeArray na = diinfo->dibuilder->getOrCreateArray(subr);
    uint64_t sizeinbit = catype->array_layout->dimarray[0] * catype->array_layout->type->size * 8;
    DICompositeType *pty = diinfo->dibuilder->createArrayType(sizeinbit, 0, kernelty, na);
    return pty;
  }
  case POINTER: {
    assert(catype->pointer_layout->dimension == 1);
    DIType *pointeety = ditype_get_or_create_from_catype(catype->pointer_layout->type, scope);
    name = catype_get_type_name(catype->signature);
    DIDerivedType *pty = diinfo->dibuilder->createPointerType(pointeety, sizeof(void *), 0, None, name);
    return pty;
  }
  default: {
    name = catype_get_type_name(catype->signature);
    return diinfo->get_ditype(name);
  }
  }
}

static DIType *ditype_get_or_create_from_catype(CADataType *catype, DIScope *scope) {
  const char *typestr = catype_get_type_name(catype->signature);
 
  DIType *ditype = diinfo->get_ditype(typestr);
  if (ditype)
    return ditype;

  ditype = ditype_create_from_catype(catype, scope);
  diinfo->put_ditype(typestr, ditype);
  return ditype;
}

static void emit_global_var_dbginfo(const char *varname, CADataType *catype, int row) {
  DIType *ditype = ditype_get_or_create_from_catype(catype, nullptr);
  llvm::DIGlobalVariableExpression *digve =
    diinfo->dibuilder->createGlobalVariableExpression(diunit, varname, StringRef(), diunit, row, ditype, false);
}

static void emit_local_var_dbginfo(llvm::Function *fn, const char *varname,
				   CADataType *catype, llvm::Value *var, int row) {
  auto itr = fn_debug_map.find(fn);
  if (itr == fn_debug_map.end())
    yyerror("cannot find function '%s' in map", fn->getName().str().c_str());

  auto &dbginfo = *itr->second;
  DIType *ditype = ditype_get_or_create_from_catype(catype, dbginfo.disp);
  DILocalVariable *divar =
    diinfo->dibuilder->createAutoVariable(curr_lexical_scope->discope, varname, diunit, row, ditype, true);

  const DILocation *diloc = DILocation::get(dbginfo.disp->getContext(), row, 0, curr_lexical_scope->discope);
  diinfo->dibuilder->insertDeclare(var, divar, diinfo->dibuilder->createExpression(),
				   diloc, ir1.builder().GetInsertBlock());
}

static int is_valued_expr(int op) {
  return (op != DBGPRINT && op != RET && op != DBGPRINTTYPE);
}

static void walk_empty(ASTNode *p) {}

static Value *aux_set_zero_to_store(Type *type, Value *var) {
  Type *i8type = ir1.intptr_type<int8_t>();
  Value *i8var = ir1.builder().CreatePointerCast(var, i8type);
  TypeSize size = ir1.module().getDataLayout().getTypeAllocSize(type);
  Align align = (static_cast<AllocaInst *>(var))->getAlign();

  CallInst *ci = ir1.builder().CreateMemSet(i8var, ir1.gen_int((int8_t)0), size, align);
  return var;
}

static void aux_copy_llvmvalue_to_store(Type *type, Value *dest, Value *src, const char *name) {
  // TODO: how to make the array transfer short
  Type::TypeID id = type->getTypeID();
  if (id != Type::ArrayTyID && id != Type::StructTyID) {
    ir1.builder().CreateStore(src, dest, name);
    return;
  }

  Type *pint8type = ir1.intptr_type<int8_t>();
  // PointerType *pint8type = PointerType::getInt8PtrTy(ir1.ctx());
  Value *pint8_destvalue = ir1.builder().CreatePointerCast(dest, pint8type);
  Value *pint8_srcvalue = ir1.builder().CreatePointerCast(src, pint8type);

  TypeSize size = ir1.module().getDataLayout().getTypeAllocSize(type);
  Align align = (static_cast<AllocaInst *>(dest))->getAlign();

  ir1.builder().CreateMemCpy(pint8_destvalue, align, pint8_srcvalue, align, size);
}

static Value *walk_literal(ASTNode *p) {
  if (walk_pass == 1)
    return nullptr;

  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

  if (!p->litn.litv.fixed_type)
    inference_expr_type(p);

  CADataType *catype = catype_get_by_name(p->symtable, p->litn.litv.datatype);
  CHECK_GET_TYPE_VALUE(p, catype, p->litn.litv.datatype);

  Value *v = gen_literal_value(&p->litn.litv, catype, p->begloc);

  if (catype->type == ARRAY || catype->type == STRUCT || catype->type == SLICE) {
    Type *arraytype = llvmtype_from_catype(catype);
    Type::TypeID id = arraytype->getTypeID();
    Constant *complexconst = static_cast<Constant *>(v);
    v = ir1.gen_global_var(arraytype, "constarray", complexconst, true);
  }

  auto operands = std::make_unique<CalcOperand>(OT_Const, v, catype);
  oprand_stack.push_back(std::move(operands));

  return v;
}

static Value *get_deref_expr_value(ASTNode *expr) {
  STEntry *entry = expr->entry;
  if (entry) {
    assert(entry->u.varshielding.current->llvm_value != nullptr);
    Value *var = static_cast<Value *>(entry->u.varshielding.current->llvm_value);
    var = ir1.builder().CreateLoad(var, "derefo");
    return var;
  }

  walk_stack(expr);
  auto pair = pop_right_value("po", true);
  Value *var = pair.first;
  return var;
}

static Value *extract_value_from_array(ASTNode *node) {
  assert(node->type == TTE_ArrayItemLeft || node->type == TTE_ArrayItemRight);
  //STEntry *entry = sym_getsym(node->symtable, node->aitemn.varname, 1);
  //CADataType *arraycatype = catype_get_by_name(node->symtable, entry->u.var->datatype);
  // NEXT TODO: walk slice node when `arraynode` is slice type
  walk_stack(node->aitemn.arraynode);
  auto array_pair = pop_right_value("aname", false);
  CADataType *arraycatype = array_pair.second;
  Value *arrayvalue = array_pair.first;
  //CHECK_GET_TYPE_VALUE(node, arraycatype, entry->u.var->datatype);

  void *indices = node->aitemn.indices;
  
  // for normalized array item the size always be 1
  size_t size = vec_size(indices);
  assert(size == 1);
  assert(arraycatype->array_layout->dimension == 1);

  // Question: how to check the index is in scope of an array?
  // Answer: when the index is not constant, only can through runtime checking, e.g.
  // insert index scope checking code into generated code, (convert array bound into
  // llvm::Value object, and insert code to compare the index value and the bound value
  // print error or exit when out of bound

  // NEXT TODO: handle when `arraycatype->type` is slice
  if (arraycatype->type != ARRAY) {
    caerror(&(node->begloc), &(node->endloc),
            "type `%d` not an array on index `%d`", arraycatype->type, 0);
    return nullptr;
  }

  ASTNode *expr = (ASTNode *)vec_at(indices, 0);
  inference_expr_type(expr);
  walk_stack(expr);
  std::pair<Value *, CADataType *> pair = pop_right_value("item", true);
  CADataType *index_catype = pair.second;

  if (!catype_is_integer(index_catype->type) &&
      !catype_is_integer_range(index_catype)) {
    caerror(&(node->begloc), &(node->endloc),
            "array index type must be integer or range of integer, but find `%s` on `%d`",
            catype_get_type_name(index_catype->signature), 0);
    return nullptr;
  }

  std::vector<Value *> vindices;
  Value *arrayitemvalue = nullptr;
  if (catype_is_integer(index_catype->type)) {
    vindices.push_back(ir1.gen_int(0));
    vindices.push_back(pair.first);

    // arrayitemvalue: is an alloc memory address, so following can store value into it
    arrayitemvalue = ir1.builder().CreateInBoundsGEP(arrayvalue, vindices);
  } else {
    // condition of when index_catype->type is RANGE, the range type provide the limitation of
    // `start` and the length `end - start` when fetching value
    Value *offset_value = nullptr;
    Value *len_value = nullptr;
    Value *valueone = nullptr;
    Value *valuetwo = nullptr;

    int64_t array_len = (int64_t)arraycatype->array_layout->dimarray[0];

    tokenid_t index_type = tokenid_novalue;
    if (index_catype->range_layout->type == FullRange)
      index_type = I64;
    else if (index_catype->range_layout->type == RangeFrom)
      index_type = index_catype->range_layout->start->type;
    else
      index_type = index_catype->range_layout->end->type;

    switch (index_catype->range_layout->type) {
    case FullRange:
      // length is the array's length
      len_value = ir1.gen_int(array_len);
      break;
    case InclusiveRangeTo:
      // length is the corresponding value of `range_layout->end + 1`
      valueone = create_default_integer_value(index_catype->range_layout->range->type, 1);
      len_value = ir1.gen_add(pair.first, valueone, "slice_len");

      // NEXT TODO: generate range check code here array size > size(range_layout->end)
      // something like following code, but in llvm code style
      //if (len_value <= array_len) {
      // runtime_error();
      //}
      break;
    case RightExclusiveRangeTo:
      // length is the corresponding value of `range_layout->end`
      len_value = pair.first;

      // NEXT TODO: generate range check code here array size >= size(range_layout->end)
      break;
    case RangeFrom:
      // NEXT TODO: generate range check code here for following 3 enum variant array size >= size(range_layout->start)
      offset_value = pair.first;
      len_value = ir1.gen_int(array_len);
      len_value = ir1.gen_sub(len_value, pair.first, "slice_len");
      break;
    case InclusiveRange:
      // NEXT TODO: generate range check code here array size > size(range_layout->end) (for both start and end)
    case RightExclusiveRange:
      // NEXT TODO: generate range check code here array size >= size(range_layout->end) (for both start and end)
      offset_value = ir1.builder().CreateExtractValue(pair.first, 0);
      valuetwo = ir1.builder().CreateExtractValue(pair.first, 1);
      len_value = ir1.gen_sub(valuetwo, offset_value, "slice_len");
      if (index_catype->type == InclusiveRange) {
	valueone = create_default_integer_value(index_type, 1);
	len_value = ir1.gen_add(len_value, valueone, "slice_len");
      }

      break;

    default:
      break;
    }

    Instruction::CastOps castopt = gen_cast_ops_token(index_type, I64);

    len_value = ir1.gen_cast_value(castopt, len_value, ir1.int_type<int64_t>());

    Value *slice_value = llvmcode_create_slice(arrayvalue, offset_value, len_value, arraycatype->array_layout->type);
    arrayitemvalue = slice_value;
  }

  //Value *arrayvalue = static_cast<Value *>(entry->u.var->llvm_value);
  //Value *arrayvalue = pair.first;

  return arrayitemvalue;
}

static Value *extract_value_from_struct(ASTNode *node) {
  assert(node->type == TTE_StructFieldOpLeft || node->type == TTE_StructFieldOpRight);
  typeid_t structtype = get_expr_type_from_tree(node->sfopn.expr);
  CADataType *structcatype = catype_get_by_name(node->symtable, structtype);
  CHECK_GET_TYPE_VALUE(node, structcatype, structtype);

  if (!node->sfopn.direct) {
    if (structcatype->type != POINTER) {
      caerror(&(node->begloc), &(node->endloc), "get struct field indirectly need a pointer to struct type, but find `%s`",
	      catype_get_type_name(structcatype->signature));
      return nullptr;
    }

    assert(structcatype->pointer_layout->dimension == 1);
    structcatype = structcatype->pointer_layout->type;
  }

  if (structcatype->type != STRUCT) {
    caerror(&(node->begloc), &(node->endloc), "get struct field directly need a struct type, but find `%s`",
	    catype_get_type_name(structcatype->signature));
    return nullptr;
  }

  int fieldindex = 0;
  if (structcatype->struct_layout->type)
    fieldindex = node->sfopn.fieldname;
  else {
    for (; fieldindex < structcatype->struct_layout->fieldnum; ++fieldindex) {
      if (structcatype->struct_layout->fields[fieldindex].name == node->sfopn.fieldname)
	break;
    }
  }

  if (fieldindex == structcatype->struct_layout->fieldnum) {
    caerror(&(node->begloc), &(node->endloc), "cannot find field `%s` of struct `%s`",
	    symname_get(node->sfopn.fieldname), catype_get_type_name(structcatype->signature));
    return nullptr;   
  }

  walk_stack(node->sfopn.expr);
  auto pair = pop_right_value("sf", !node->sfopn.direct);
  if (!node->sfopn.direct)
    assert(pair.second->type == POINTER && pair.second->pointer_layout->type->signature == structcatype->signature);
  else
    assert(pair.second->signature == structcatype->signature);
  
  // extract field value from struct value
  std::vector<Value *> vindices;
  vindices.push_back(ir1.gen_int(0));
  vindices.push_back(ir1.gen_int(fieldindex));

  // structfieldvalue: is an alloc memory address, so following can store value into it
  Value *structfieldvalue = ir1.builder().CreateInBoundsGEP(pair.first, vindices);
  return structfieldvalue;
}

static inline bool is_create_global_var(STEntry *entry) {
  // if nomain specified then curr_fn and main_fn are all nullptr, so they are also equal
  // here determine if `#[scope(global)]` is specified
  return curr_fn == main_fn && (!main_fn || entry->u.varshielding.current->global);
}

static inline bool is_var_declare(ASTNode *p) {
  return p->type == TTE_Id && p->entry->u.varshielding.current->llvm_value == nullptr;
}

static Value *walk_id_defv_declare(ASTNode *p, CADataType *idtype, bool zeroinitial, Value *defval) {
  Value *var = nullptr;
  const char *name = symname_get(p->idn.i);
  Type *type = llvmtype_from_catype(idtype);

  STEntry *entry = p->entry;
  if (entry->sym_type != Sym_Variable) {
    caerror(&(entry->sloc), NULL, "'%s' Not a variable", name);
    return nullptr;
  }

  if (is_create_global_var(entry)) {
    var = ir1.gen_global_var(type, name, defval, false, zeroinitial);

    if (enable_debug_info())
      emit_global_var_dbginfo(name, idtype, p->endloc.row);
  } else {
    var = ir1.gen_entry_block_var(curr_fn, type, name, nullptr);

    if (zeroinitial)
      aux_set_zero_to_store(type, var);
    else if (defval)
      aux_copy_llvmvalue_to_store(type, var, defval, name);

    if (enable_debug_info())
      emit_local_var_dbginfo(curr_fn, name, idtype, var, p->endloc.row);
  }

  entry->u.varshielding.current->llvm_value = static_cast<void *>(var);
  return var;
}

static Value *inplace_assignop_assistant(ASTNode *p, CADataType *idtype, Type *type, int assignop, Value *vl, Value *vr) {
  // handling inside replace operation, e.g. a += 1;
  vl = ir1.builder().CreateLoad(vl);
  if (idtype->type == POINTER) {
    if (assignop == ASSIGN_SUB) {
      Value *z = ir1.gen_int((int64_t)0);
      vr = ir1.builder().CreateSExt(vr, ir1.int_type<int64_t>());
      vr = ir1.gen_sub(z, vr, "m");
    }

    Type *type = llvmtype_from_catype(idtype->pointer_layout->type);
    vl = ir1.builder().CreateGEP(type, vl, vr, "pop");
    return vl;
  }

  switch(assignop) {
  case ASSIGN_ADD:
    vr = ir1.gen_add(vl, vr);
    break;
  case ASSIGN_SUB:
    vr = ir1.gen_sub(vl, vr);
    break;
  case ASSIGN_MUL:
    vr = ir1.gen_mul(vl, vr);
    break;
  case ASSIGN_DIV:
    vr = ir1.gen_div(vl, vr);
    break;
  case ASSIGN_MOD:
    vr = ir1.gen_mod(vl, vr);
    break;
  case ASSIGN_BAND:
    vr = ir1.builder().CreateAnd(vl, vr, "band");
    break;
  case ASSIGN_BOR:
    vr = ir1.builder().CreateOr(vl, vr, "bor");
    break;
  case ASSIGN_BXOR:
    vr = ir1.builder().CreateXor(vl, vr, "bxor");
    break;
  case ASSIGN_SHIFTL:
    vr = ir1.builder().CreateZExtOrTrunc(vr, type);
    vr = ir1.builder().CreateShl(vl, vr, "shl");
    break;
  case ASSIGN_SHIFTR:
    vr = ir1.builder().CreateZExtOrTrunc(vr, type);
    if (catype_is_signed(idtype->type))
      vr = ir1.builder().CreateAShr(vl, vr, "ashr");
    else
      vr = ir1.builder().CreateLShr(vl, vr, "lshr");
    break;
  default:
    caerror(&(p->begloc), &(p->endloc), "unknown inside variable operator: `%d`", assignop);
    return nullptr;
  }

  return vr;
}

// generate variable, if in a function then it is a local variable, when not in
// a function but `-nomain` is specified then generate a global variable else
// also generate a global variable for other use
// `arrayleftvalue` for TTE_ArrayItemLeft type
static Value *walk_id_defv(ASTNode *p, CADataType *idtype, int assignop = -1, bool zeroinitial = false, Value *defval = nullptr) {
  Value *var = nullptr;
  const char *name = symname_get(p->idn.i);
  Type *type = llvmtype_from_catype(idtype);

  if (zeroinitial) {
    yyerror("assignment not support assigning zero value");
    return nullptr;
  }

  switch (p->type) {
  case TTE_Id:
    var = static_cast<Value *>(p->entry->u.varshielding.current->llvm_value);
    break;
  case TTE_DerefLeft:
    var = get_deref_expr_value(p->deleftn.expr);
    for (int i = 0; i < p->deleftn.derefcount - 1; ++i)
      var = ir1.builder().CreateLoad(var, "deref");
    break;
  case TTE_ArrayItemLeft:
    var = extract_value_from_array(p);
    break;
  case TTE_StructFieldOpLeft:
    var = extract_value_from_struct(p);
    break;
  default:
    break;
  }

  if (assignop != -1)
    defval = inplace_assignop_assistant(p, idtype, type, assignop, var, defval);

  if (defval)
    aux_copy_llvmvalue_to_store(type, var, defval, name);

  return var;
}

static typeid_t catype_get_core_type(typeid_t type) {
  // NEXT TODO: get the type's core type
  // 1. *AA, [AA;2], *[AA;3], [*[AA;3]; 2]
  // 2. new defined struct is not cored for generic type because the new defined struct type will create new symbol table

  // reference the catype_get_by_name
  return type;
}

static void generic_type_record_replace(typeid_t *datatype_addr, typeid_t old_value, typeid_t new_value) {
  if (generic_type_stack.empty())
    return;

  SymTableAssoc *assoc = generic_type_stack.back().first;
  if (!assoc)
    return;

  // to avoid the different value with the same addresses, like following, or it need compare the same address
  // {
  //   {datatype_addr = 0x5555556cd410, old_value = 201, new_value = 395},
  //   {datatype_addr = 0x5555556cdd70, old_value = -1, new_value = 21},
  //   {datatype_addr = 0x5555556cd410, old_value = 395, new_value = 395}
  // }
  if (old_value == new_value)
    return;

  if (generic_type_stack.back().second.find(datatype_addr) != generic_type_stack.back().second.end())
    return;

#if 0 // when enabled it will only record the necessary types of variable (the generic type or trait type),
  // but for now it will need realize function `catype_get_core_type`, so when function `catype_get_core_type`
  // is realized, it should reopened
  std::set<int> *ids = (std::set<int> *)assoc->extra_id_list;

  typeid_t old_value_core = catype_get_core_type(old_value);
  if (ids->find(old_value_core) == ids->end())
    return;
#endif

  generic_type_stack.back().second.insert(std::make_pair(datatype_addr, GenericTypeVarInfo(datatype_addr, old_value, new_value)));
}

static void generic_type_handle_replace(std::pair<SymTableAssoc *, generic_type_var_set_t> &assoc_pair) {
  if (!assoc_pair.first)
    return;

  for (auto iter = assoc_pair.second.begin(); iter != assoc_pair.second.end(); ++iter) {
    *iter->second.datatype_addr = iter->second.old_value;
  }
}

static Value *walk_id(ASTNode *p) {
  if (walk_pass == 1)
    return nullptr;

  CADataType *catype = catype_get_by_name(p->symtable, p->entry->u.varshielding.current->datatype);
  CHECK_GET_TYPE_VALUE(p, catype, p->entry->u.varshielding.current->datatype);

  generic_type_record_replace(&p->entry->u.varshielding.current->datatype, p->entry->u.varshielding.current->datatype, catype->signature);
 
  p->entry->u.varshielding.current->datatype = catype->signature;

  Value *var = nullptr;
  if (is_var_declare(p))
    //var = walk_id_defv_declare(p, idtype, false, nullptr);
    yyerror("walk id should cannot come here");
  else
    var = walk_id_defv(p, catype);

  auto operands = std::make_unique<CalcOperand>(OT_Alloc, var, catype);
  oprand_stack.push_back(std::move(operands));
  return var;
}

static BasicBlock *walk_label(ASTNode *p) {
  if (walk_pass == 1)
    return nullptr;

  if (!curr_fn)
    return nullptr;

  BasicBlock *bb;
  const char *label_name = symname_get(p->idn.i) + 2;
  auto itr = label_map.find(label_name);
  if (itr != label_map.end()) {
    bb = itr->second;
  } else {
    bb = ir1.gen_bb(label_name);
    label_map.insert(std::make_pair(label_name, bb));
  }

  if (enable_debug_info()) {
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

    auto itr = fn_debug_map.find(curr_fn);
    if (itr == fn_debug_map.end())
      caerror(&(p->begloc), &(p->endloc), "cannot find function '%s' in map", curr_fn->getName().str().c_str());

    auto &dbginfo = *itr->second;
    DILabel *dilabel = diinfo->dibuilder->createLabel(dbginfo.disp, label_name, diunit, p->endloc.row);

    const DILocation *diloc = DILocation::get(dbginfo.disp->getContext(), p->endloc.row, 0, curr_lexical_scope->discope);
    diinfo->dibuilder->insertLabel(dilabel, diloc, bb);
  }

  ir1.builder().CreateBr(bb);
  curr_fn->getBasicBlockList().push_back(bb);
  ir1.builder().SetInsertPoint(bb);
  return bb;
}

static void walk_label_goto(ASTNode *label) {
  if (walk_pass == 1)
    return;

  int i = label->idn.i;
  BasicBlock *bb;
  const char *label_name = symname_get(i) + 2;
  auto itr = label_map.find(label_name);
  if (itr != label_map.end()) {
    bb = itr->second;
  } else {
    bb = ir1.gen_bb(label_name);
    label_map.insert(std::make_pair(label_name, bb));
  }

  if (enable_debug_info())
    diinfo->emit_location(label->endloc.row, label->endloc.col, curr_lexical_scope->discope);
  ir1.builder().CreateBr(bb);

  // to avoid verify error of 'Terminator found in the middle of a basic block!'
  BasicBlock *extrabb = ir1.gen_bb("extra", curr_fn);
  ir1.builder().SetInsertPoint(extrabb);
}

static void walk_stmtlist(ASTNode *p) {
  for (int i = 0; i < p->stmtlistn.nstmt; ++i)
    walk_stack(p->stmtlistn.stmts[i]);
}

static void walk_break(ASTNode *p) {
  if (walk_pass == 1)
    return;

  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

  ir1.builder().CreateBr(g_loop_controls.back()->outbb);

  BasicBlock *extrabb = ir1.gen_bb("extra", curr_fn);
  ir1.builder().SetInsertPoint(extrabb);
}

static void walk_continue(ASTNode *p) {
  if (walk_pass == 1)
    return;

  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

  ir1.builder().CreateBr(g_loop_controls.back()->condbb);

  BasicBlock *extrabb = ir1.gen_bb("extra", curr_fn);
  ir1.builder().SetInsertPoint(extrabb);
}

static void walk_loop(ASTNode *p) {
  if (walk_pass == 1) {
    walk_stack(p->loopn.body);    
    return;
  }

  if (!curr_fn)
    return;

  BasicBlock *loopbb = ir1.gen_bb("loopbb");
  BasicBlock *endloopbb = ir1.gen_bb("endloopbb");

  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

  ir1.builder().CreateBr(loopbb);
  curr_fn->getBasicBlockList().push_back(loopbb);
  ir1.builder().SetInsertPoint(loopbb);

  g_loop_controls.push_back(std::make_unique<LoopControlInfo>(LoopControlInfo::LT_Loop, -1, loopbb, endloopbb));
  walk_stack(p->loopn.body);
  g_loop_controls.pop_back();

  ir1.builder().CreateBr(loopbb);

  curr_fn->getBasicBlockList().push_back(endloopbb);
  ir1.builder().SetInsertPoint(endloopbb);
}

static bool is_forstmt_pointer_var(ForStmtId forvar) {
  return forvar.vartype == '*';
}

static Value *aux_create_compare_value_for(ASTNode *p, CADataType *list_catype, CADataType *item_catype,
					   Value *curr_value, Value *end_cond_value) {
  if (list_catype->type == ARRAY) {
    return ir1.builder().CreateICmpULT(curr_value, end_cond_value);
  }

  // range catype
  Value *ltv = nullptr;
  switch (item_catype->type) {
  case I8:
  case I16:
  case I32:
  case I64:
    if (list_catype->range_layout->inclusive)
      ltv = ir1.builder().CreateICmpSLE(curr_value, end_cond_value);
    else
      ltv = ir1.builder().CreateICmpSLT(curr_value, end_cond_value);
    break;
  case U8:
  case U16:
  case U32:
  case U64:
    if (list_catype->range_layout->inclusive)
      ltv = ir1.builder().CreateICmpULE(curr_value, end_cond_value);
    else
      ltv = ir1.builder().CreateICmpULT(curr_value, end_cond_value);
    break;
  case BOOL:
  case F32:
  case F64:
  default:
    ltv = nullptr;
    caerror(&p->begloc, &p->endloc, "type `%s` not support step into next yet",
	    catype_get_type_name(item_catype->signature));
    break;
  }

  return ltv;
}

static void walk_for(ASTNode *p) {
  if (walk_pass == 1) {
    walk_stack(p->forn.body);
    return;
  }

  if (!curr_fn)
    return;

  BasicBlock *condbb = ir1.gen_bb("condbb");
  BasicBlock *loopbb = ir1.gen_bb("loopbb");
  BasicBlock *endloopbb = ir1.gen_bb("endloopbb");

  if (enable_debug_info())
    diinfo->emit_location(p->forn.listnode->endloc.row, p->forn.listnode->endloc.col, curr_lexical_scope->discope);

  // TODO: currently only support iterator array & range, later will support generator list e.g. (1..6)
  // the generator list also need allocate just like variable

  // prepare list nodes and the variable
  inference_expr_type(p->forn.listnode);
  walk_stack(p->forn.listnode);
  auto pair = pop_right_value("list", false);
  Value *lists = pair.first;

  CADataType *list_catype = pair.second;
  if (list_catype->type != ARRAY && list_catype->type != RANGE) {
    caerror(&(p->forn.listnode->begloc), &(p->forn.listnode->endloc),
	    "currently only support iterate array and range type in for statement, but find `%s`",
	    catype_get_type_name(list_catype->signature));
    return;
  }

  ForStmtId forvar = p->forn.var;
  STEntry *entry = sym_getsym(p->symtable, forvar.var, 0);
  if (!entry) {
    caerror(&p->forn.listnode->begloc, &p->forn.listnode->endloc, "cannot find variable `%s` in symbol table",
	    symname_get(forvar.var));
    return;
  }

  CAVariable *cavar = entry->u.varshielding.current;

  CADataType *itemcatype = nullptr;

  // the reference use the same type as value
  switch (list_catype->type) {
  case ARRAY:
    itemcatype = list_catype->array_layout->type;
    break;
  case RANGE:
    if (list_catype->range_layout->range->type == STRUCT)
      itemcatype = list_catype->range_layout->range->struct_layout->fields[0].type;
    else
      itemcatype = list_catype->range_layout->range;

    // TODO: check itemcatype, it should not be complex, the complex type may need a user defined comparing function
    if (!catype_is_integer(itemcatype->type)) {
      caerror(&p->forn.listnode->begloc, &p->forn.listnode->endloc, "type `%s` not support step into next yet",
	      catype_get_type_name(itemcatype->signature));
      return;
    }

    break;
  }

  if (is_forstmt_pointer_var(forvar)) {
    itemcatype = catype_make_pointer_type(itemcatype);
  }
  
  cavar->datatype = itemcatype->signature;

  // scanner index llvm value
  Value *valuezero = ir1.gen_int((size_t)0);
  Value *valueone = nullptr;

  // 1. initial condition and terminate condition
  Value *begin_cond_v = nullptr;
  Value *end_cond_v = nullptr;
  if (list_catype->type == ARRAY) {
    size_t listsize = list_catype->array_layout->dimarray[0];
    valueone = ir1.gen_int((size_t)1);

    begin_cond_v = valuezero;

    // list size llvm value
    end_cond_v = ir1.gen_int(listsize);
  } else { // range
    valueone = create_default_integer_value(itemcatype->type, 1);

#if 0
    Value *valuezero1 = ir1.gen_int(0);
    std::vector<Value *> idxv(2, valuezero1);
    begin_cond_v = ir1.builder().CreateGEP(lists, idxv);

    idxv[1] = valueone;
    end_cond_v = ir1.builder().CreateGEP(lists, idxv);
#else
    Value *listsv = ir1.builder().CreateLoad(lists);
    begin_cond_v = ir1.builder().CreateExtractValue(listsv, 0);
    end_cond_v = ir1.builder().CreateExtractValue(listsv, 1);
#endif

  }

  Type *item_type = llvmtype_from_catype(itemcatype);

  Type *index_type = nullptr;
  if (list_catype->type == ARRAY) {
    index_type = ir1.int_type<size_t>();
  } else {
    index_type = item_type;
  }

  // 2. value index slot
  Value *indexvslot = ir1.gen_entry_block_var(curr_fn, index_type, "idx", begin_cond_v);

  // 3. generate item variable value used in for body
  const char *itemname = symname_get(cavar->name);
  Value *itemvar = ir1.gen_entry_block_var(curr_fn, item_type, itemname, nullptr);
  if (enable_debug_info())
    emit_local_var_dbginfo(curr_fn, itemname, itemcatype, itemvar, p->forn.listnode->endloc.row);

  cavar->llvm_value = static_cast<void *>(itemvar);

  ir1.builder().CreateBr(condbb);

  // condition block
  curr_fn->getBasicBlockList().push_back(condbb);
  ir1.builder().SetInsertPoint(condbb);

  // branch according to the list nodes, when no node left then out else loop again
  // 4. load value index value
  Value *indexv = ir1.builder().CreateLoad(indexvslot, "idxv");

  // 5. compare condition index value with terminate condition
  Value *ltv = aux_create_compare_value_for(p, list_catype, itemcatype, indexv, end_cond_v);

  ir1.builder().CreateCondBr(ltv, loopbb, endloopbb);

  curr_fn->getBasicBlockList().push_back(loopbb);
  ir1.builder().SetInsertPoint(loopbb);

  // copy array item value into the variable
  
  // 6. get item value from list
  // TODO: handle the reference type variable
  Value *listitemv = nullptr;
  if (list_catype->type == ARRAY) {
    std::vector<Value *> idxv(2, valuezero);
    idxv[1] = indexv;
    Value *listitemvslot = ir1.builder().CreateInBoundsGEP(lists, idxv);
    bool iscomplextype = catype_is_complex_type(itemcatype);
    listitemv = listitemvslot;
    if (!iscomplextype && !is_forstmt_pointer_var(forvar))
      listitemv = ir1.builder().CreateLoad(listitemvslot);
  } else { // range
    listitemv = indexv;
  }

  // 7. copy value from list item to item variable
  aux_copy_llvmvalue_to_store(item_type, itemvar, listitemv, "auxi");

  // 8. get next index and store into slot
  // increment the index
  Value *indexloadv = ir1.builder().CreateLoad(indexvslot, "idxv");
  Value *incv = ir1.builder().CreateAdd(indexloadv, valueone);
  ir1.builder().CreateStore(incv, indexvslot);

  if (enable_debug_info())
    diinfo->emit_location(p->forn.body->begloc.row, p->forn.body->begloc.col, curr_lexical_scope->discope);

  g_loop_controls.push_back(std::make_unique<LoopControlInfo>(LoopControlInfo::LT_For, -1, condbb, endloopbb));
  walk_stack(p->forn.body);
  g_loop_controls.pop_back();

  if (enable_debug_info())
    diinfo->emit_location(p->forn.body->endloc.row, p->forn.body->endloc.col, curr_lexical_scope->discope);

  ir1.builder().CreateBr(condbb);

  curr_fn->getBasicBlockList().push_back(endloopbb);
  ir1.builder().SetInsertPoint(endloopbb);
}

// complex type, allocate, pointer, memcpy:
// box([1, 2]): allocated, not equal type, not need load, need memcpy
// a = [1, 2]; box(a), allocated, not equal type, not need load, need memcpy
// simple type:
// box(33): not need load, not need memcpy
// a = 33; box(a); need load, not need memcpy
static void walk_box(ASTNode *p) {
  if (walk_pass == 1)
    return;

  walk_stack(p->boxn.expr);
  auto pair = pop_right_value("willbox", false);
  Value *willv = pair.first;
  CADataType *pointeety = pair.second;
  CADataType *pointerty = catype_make_pointer_type(pointeety);
  pointerty->pointer_layout->allocpos = CAPointerAllocPos::PP_Heap;
  Type *type = llvmtype_from_catype(pointerty);

  // 1. invoke allocate memory function to allocate memory,
  Value *heapv = llvmcode_box(pointeety->size, type);

  // 2. invoke alloca to allocate pointer type and
  // 3. store the heap allocated address into the memory
  //Value *stackv = ir1.gen_entry_block_var(curr_fn, type, "bindptr", heapv);
  
  // 4. copy Value willv into heap allocated space
  Type *pointeellvmty = llvmtype_from_catype(pointeety);
  if (!catype_is_complex_type(pointeety) &&
      pointeellvmty != willv->getType()) {
    willv = ir1.builder().CreateLoad(willv, "willbox");
  }

  aux_copy_llvmvalue_to_store(pointeellvmty, heapv, willv, "binddata");
  
  // 5. return the pointer memory address
  //auto operands = std::make_unique<CalcOperand>(OT_Alloc, stackv, pointerty);
  auto operands = std::make_unique<CalcOperand>(OT_HeapAlloc, heapv, pointerty);
  oprand_stack.push_back(std::move(operands));
}

static void walk_drop(ASTNode *p) {
  if (walk_pass == 1)
    return;

  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

  STEntry *entry = sym_getsym(p->symtable, p->dropn.var, 1);
  if (!entry) {
    caerror(&p->begloc, &p->endloc, "cannot find variable `%s` in symbol table when dropping",
	    symname_get(p->dropn.var));
    return;
  }

  if (entry->sym_type != Sym_Variable) {
    caerror(&p->begloc, &p->endloc, "'%s' Not a variable when dropping", symname_get(p->dropn.var));
    return;
  }

  Value *boxedv = static_cast<Value *>(entry->u.varshielding.current->llvm_value);
  Value *heapv = ir1.builder().CreateLoad(boxedv, "heapv");
  llvmcode_drop(heapv);
}

static void walk_while(ASTNode *p) {
  if (walk_pass == 1) {
    walk_stack(p->whilen.body);
    return;
  }

  if (!curr_fn)
    return;

  BasicBlock *condbb = ir1.gen_bb("condbb");
  BasicBlock *whilebb = ir1.gen_bb("whilebb");
  BasicBlock *endwhilebb = ir1.gen_bb("endwhilebb");

  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

  ir1.builder().CreateBr(condbb);
  curr_fn->getBasicBlockList().push_back(condbb);
  ir1.builder().SetInsertPoint(condbb);

  inference_expr_type(p->whilen.cond);
  walk_stack(p->whilen.cond);
  auto pair = pop_right_value("cond");
  Value *cond = pair.first;
  if (pair.second->type != BOOL) {
    // when grammar also support other type compare, here should convert the other
    // type into bool type, like following, but need generate compare with right
    // type not hardcoded `int` type
    //cond = ir1.builder().CreateICmpNE(cond, ir1.gen_int(0), "if_cond_cmp");
    caerror(&(p->begloc), &(p->endloc), "condition only accept `bool` type, but find `%s`",
	    get_type_string(pair.second->type));
    return;
  }

  ir1.builder().CreateCondBr(cond, whilebb, endwhilebb);

  curr_fn->getBasicBlockList().push_back(whilebb);
  ir1.builder().SetInsertPoint(whilebb);

  g_loop_controls.push_back(std::make_unique<LoopControlInfo>(LoopControlInfo::LT_While, -1, condbb, endwhilebb));
  walk_stack(p->whilen.body);
  g_loop_controls.pop_back();

  // TBD: how to remove the stack element that never used?
  ir1.builder().CreateBr(condbb);

  curr_fn->getBasicBlockList().push_back(endwhilebb);
  ir1.builder().SetInsertPoint(endwhilebb);
}

static void walk_if_common(ASTNode *p) {
  if (walk_pass == 1) {
    ASTNode *firstbody = static_cast<ASTNode *>(vec_at(p->ifn.bodies, 0));
    walk_stack(firstbody);
    if (p->ifn.remain) { /* if else */
      walk_stack(p->ifn.remain);
    }

    return;
  }

  if (!curr_fn)
    return;

  int isexpr = p->ifn.isexpr;

  if (enable_debug_info())
    diinfo->emit_location(p->begloc.row, p->begloc.col, curr_lexical_scope->discope);

  Value *tmpv1 = nullptr;
  Value *tmpv2 = nullptr;
  // the clang always alloca in the header of the function, it may error
  // occurs when alloca in other blocks. Answer: not exactly
  Value *tmpc = nullptr; // for storing if expression temporary value
  if (isexpr)
    tmpc = ir1.gen_entry_block_var(curr_fn, ir1.int_type<int>(), "tmpc");

  BasicBlock *thenbb = ir1.gen_bb("thenbb");
  BasicBlock *outbb = ir1.gen_bb("outbb");
  BasicBlock *elsebb = nullptr;

  ASTNode *firstcond = static_cast<ASTNode *>(vec_at(p->ifn.conds, 0));
  inference_expr_type(firstcond);
  walk_stack(firstcond);
  auto pair = pop_right_value("cond");
  Value *cond = pair.first;
  if (pair.second->type != BOOL) {
    // when grammar also support other type compare, here should convert the other
    // type into bool type, like following, but need generate compare with right
    // type not hardcoded `int` type
    //cond = ir1.builder().CreateICmpNE(cond, ir1.gen_int(0), "if_cond_cmp");
    caerror(&(p->begloc), &(p->endloc), "condition only accept `bool` type, but find `%s`",
	    get_type_string(pair.second->type));
    return;
  }

  //tokenid_t tt1 = 0, tt2 = 0;
  CADataType *tt1 = nullptr;
  CADataType *tt2 = nullptr;

  ASTNode *firstbody = static_cast<ASTNode *>(vec_at(p->ifn.bodies, 0));
  if (p->ifn.remain) { /* if else */
    elsebb = ir1.gen_bb("elsebb");
    ir1.builder().CreateCondBr(cond, thenbb, elsebb);
    curr_fn->getBasicBlockList().push_back(thenbb);
    ir1.builder().SetInsertPoint(thenbb);
    walk_stack(firstbody);
    if (isexpr) {
      auto tmpv1 = pop_right_value("tmpv");
      ir1.store_var(tmpc, tmpv1.first);
      tt1 = tmpv1.second;
    }

    ir1.builder().CreateBr(outbb);

    curr_fn->getBasicBlockList().push_back(elsebb);
    ir1.builder().SetInsertPoint(elsebb);
    walk_stack(p->ifn.remain);
    if (isexpr) {
      auto tmpv2 = pop_right_value("tmpv");
      ir1.store_var(tmpc, tmpv2.first);
      tt2 = tmpv2.second;
    }
  } else { /* if */
    ir1.builder().CreateCondBr(cond, thenbb, outbb);
    curr_fn->getBasicBlockList().push_back(thenbb);
    ir1.builder().SetInsertPoint(thenbb);
    walk_stack(firstbody);
  }

  ir1.builder().CreateBr(outbb);
  curr_fn->getBasicBlockList().push_back(outbb);
  ir1.builder().SetInsertPoint(outbb);
  
  if (isexpr) {
    // TODO: check if use typeid instead of tokenid
    if (tt1->type != tt2->type) {
      yyerror("expression type not equal in if ... else ... expression");
      return;
    }

#if 0 // the phi is not debugger friendly, so using alloc for temporary variable
    PHINode *phiv = ir1.gen_phi(ir1.int_type<int>(), thenbb, tmpv1, elsebb, tmpv2);
    auto pnv = std::make_unique<CalcOperand>(OT_PHINode, phiv, tt1);
#else
    auto pnv = std::make_unique<CalcOperand>(OT_Alloc, tmpc, tt1);
#endif

    oprand_stack.push_back(std::move(pnv));
  }
}

static void walk_if_common2(ASTNode *p) {
  if (walk_pass == 1) {
    int condsize = vec_size(p->ifn.conds);

    // body part
    for (int i = 0; i < condsize; ++i) {
      ASTNode *bodyn = static_cast<ASTNode *>(vec_at(p->ifn.bodies, i));
      walk_stack(bodyn);
    }

    if (p->ifn.remain) {
      walk_stack(p->ifn.remain);
    }

    return;
  }

  if (!curr_fn)
    return;

  if (enable_debug_info())
    diinfo->emit_location(p->begloc.row, p->begloc.col, curr_lexical_scope->discope);

  // the clang always alloca in the header of the function, it may error
  // occurs when alloca in other blocks. Answer: not exactly

  // initialize BasicBlock
  int condsize = vec_size(p->ifn.conds);
  std::vector<BasicBlock *> condbbs;
  std::vector<BasicBlock *> bodybbs;
  BasicBlock *outbb = ir1.gen_bb("outbb");
  for (int i = 0; i < condsize; ++i) {
    char bbname[16];
    sprintf(bbname, "cond%d", i+1);
    condbbs.push_back(ir1.gen_bb(bbname));
    sprintf(bbname, "then%d", i);
    bodybbs.push_back(ir1.gen_bb(bbname));
  }

  if (p->ifn.remain) {
    char bbname[16];
    sprintf(bbname, "then%d", condsize);
    bodybbs.push_back(ir1.gen_bb(bbname));
  }

  // condition part
  for (int i = 0; i < condsize; ++i) {
    ASTNode *condn = static_cast<ASTNode *>(vec_at(p->ifn.conds, i));
    inference_expr_type(condn);
    walk_stack(condn);
    auto pair = pop_right_value("cond");
    Value *condv = pair.first;
    if (pair.second->type != BOOL) {
      // condition must be bool type
      caerror(&(p->begloc), &(p->endloc), "condition only accept `bool` type, but find `%s`",
	      get_type_string(pair.second->type));
      return;
    }

    ir1.builder().CreateCondBr(condv, bodybbs[i], condbbs[i]);
    curr_fn->getBasicBlockList().push_back(condbbs[i]);
    ir1.builder().SetInsertPoint(condbbs[i]);
  }

  ir1.builder().CreateBr(p->ifn.remain ? bodybbs[condsize] : outbb);

  // body part
  for (int i = 0; i < condsize; ++i) {
    curr_fn->getBasicBlockList().push_back(bodybbs[i]);
    ir1.builder().SetInsertPoint(bodybbs[i]);
    ASTNode *bodyn = static_cast<ASTNode *>(vec_at(p->ifn.bodies, i));
    walk_stack(bodyn);
    ir1.builder().CreateBr(outbb);
  }

  if (p->ifn.remain) {
    curr_fn->getBasicBlockList().push_back(bodybbs[condsize]);
    ir1.builder().SetInsertPoint(bodybbs[condsize]);
    walk_stack(p->ifn.remain);
    ir1.builder().CreateBr(outbb);   
  }

  curr_fn->getBasicBlockList().push_back(outbb);
  ir1.builder().SetInsertPoint(outbb);
}

static void walk_if(ASTNode *p) {
  if (p->ifn.isexpr)
    walk_if_common(p);
  else
    walk_if_common2(p);
}

static void walk_expr_ife(ASTNode *p) {
  walk_if_common(p);
}

static void dbgprint_value(Function *fn, CADataType *catype, Value *v);
static void dbgprint_value_range(Function *fn, CADataType *catype, Value *v) {
  GeneralRangeType range_type = catype->range_layout->type;
  switch (range_type) {
  case FullRange:
    // should not come here
    llvmcode_printf(fn, "..", nullptr);
    break;
  case InclusiveRange:
  case RightExclusiveRange: {
    Value *v1 = ir1.builder().CreateExtractValue(v, 0);
    assert(catype->range_layout->range->type == STRUCT);
    assert(catype->range_layout->range->struct_layout->type == Struct_GeneralTuple);
    dbgprint_value(fn, catype->range_layout->range->struct_layout->fields[0].type, v1);
    llvmcode_printf(fn, range_type == InclusiveRange ? "..=" : "..", nullptr);
    Value *v2 = ir1.builder().CreateExtractValue(v, 1);
    dbgprint_value(fn, catype->range_layout->range->struct_layout->fields[1].type, v2);
    break;
  }
  case InclusiveRangeTo:
  case RightExclusiveRangeTo:
    llvmcode_printf(fn, range_type == InclusiveRangeTo ? "..=" : "..", nullptr);
  case RangeFrom:
    dbgprint_value(fn, catype->range_layout->range, v);
    if (catype->range_layout->type == RangeFrom)
      llvmcode_printf(fn, "..", nullptr);

    break;
  default:
    yyerror("bad range type: %d", catype->range_layout->type);
    break;
  }
}

static void dbgprint_value(Function *fn, CADataType *catype, Value *v) {
  int len = 0;
  switch(catype->type) {
  case ARRAY:
    assert(catype->array_layout->dimension == 1);
    llvmcode_printf(fn, "[", nullptr);
    len = catype->array_layout->dimarray[0];
    for (int i = 0; i < len; ++i) {
      //ConstantArray *arrayv = static_cast<ConstantArray *>(v);
      //Constant *subv = arrayv->getAggregateElement(i);

      //Value *idx = ir1.gen_int(i);
      //Value *subv = ir1.builder().CreateGEP(v, idx, "subv");

      //Type* array_t =  llvm::PointerType::getUnqual(v->getType());
      Value *subv = ir1.builder().CreateExtractValue(v, i);

      dbgprint_value(fn, catype->array_layout->type, subv);
      if (i < len - 1)
	llvmcode_printf(fn, ", ", nullptr);
    }

    llvmcode_printf(fn, "]", nullptr);
    break;
  case POINTER:
    llvmcode_printf_primitive(fn, catype, v);
    break;
  case SLICE:
  case STRUCT: {
    const char *name = symname_get(catype->struct_layout->name);
    Constant *sname = ir1.get_global_string(name);
    CAStructField *fields = catype->struct_layout->fields;
    len = catype->struct_layout->fieldnum;
    CAStructType struct_type = catype->struct_layout->type;
    const char *fmt = NULL;
    switch(struct_type) {
    case Struct_GeneralTuple:
      fmt = "%s( ";
      break;
    case Struct_NamedTuple:
      fmt = "%s ( ";
      break;
    case Struct_Slice:
      fmt = "%s < ";
      break;
    case Struct_Union:
    case Struct_Enum:
    case Struct_NamedStruct:
    default:
      fmt = "%s { ";
      break;
    }

    llvmcode_printf(fn, fmt, sname, nullptr);
    for (int i = 0; i < len; ++i) {
      //ConstantArray *arrayv = static_cast<ConstantArray *>(v);
      //Constant *subv = arrayv->getAggregateElement(i);

      //Value *idx = ir1.gen_int(i);
      //Value *subv = ir1.builder().CreateGEP(v, idx, "subv");

      //Type* array_t =  llvm::PointerType::getUnqual(v->getType());
      if (struct_type == Struct_NamedStruct) {
	name = symname_get(fields[i].name); // field name
	sname = ir1.get_global_string(name);
	llvmcode_printf(fn, "%s: ", sname, nullptr);
      }

      Value *subv = ir1.builder().CreateExtractValue(v, i);
      dbgprint_value(fn, fields[i].type, subv);

      if (i < len - 1)
	llvmcode_printf(fn, ", ", nullptr);
    }

    switch(struct_type) {
    case Struct_NamedStruct:
    case Struct_Union:
    case Struct_Enum:
      fmt = " }";
      break;
    case Struct_Slice:
      fmt = " >";
      break;
    default:
      fmt = " )";
      break;
    }

    llvmcode_printf(fn, fmt, nullptr);
    //yyerror("dbgprint for struct type not implmeneted yet");
    break;
  }
  case RANGE: {
    dbgprint_value_range(fn, catype, v);
    break;
  }
  default:
    // output each of primitive type
    llvmcode_printf_primitive(fn, catype, v);
    break;
  }
}

#ifdef TEST_RUNTIME
static void test_rt_add() {
  Function *rt_add_fn = ir1.module().getFunction("rt_add");
  if (!rt_add_fn) {
    std::vector<Type *> types(2, ir1.int_type<int>());
    auto param_names = std::vector<const char *>(2, "a");
    param_names[1] = "b";
    rt_add_fn = ir1.gen_extern_fn(ir1.int_type<int>(), "rt_add", types, &param_names, false);
    rt_add_fn->setCallingConv(CallingConv::C);

    //AttrListPtr func_printf_PAL;
    //printf_fn->setAttributes(func_printf_PAL);
  }

  rt_add_fn = ir1.module().getFunction("rt_add");

  std::vector<Value *> params;
  params.push_back(ir1.gen_int(100));
  params.push_back(ir1.gen_int(234));
  ir1.builder().CreateCall(rt_add_fn, params, "add_v");

  std::vector<Type *> types2(2, ir1.int_type<int>());
  auto param_names2 = std::vector<const char *>(2, "a");
  param_names2[1] = "b";
  Function *rt_sub_fn = ir1.gen_extern_fn(ir1.int_type<int>(), "rt_sub", types2, &param_names2, false);
  rt_sub_fn->setCallingConv(CallingConv::C);

  std::vector<Value *> params2;
  params2.push_back(ir1.gen_int(100));
  params2.push_back(ir1.gen_int(234));
  ir1.builder().CreateCall(rt_sub_fn, params, "sub_v");
}
#endif

static void walk_dbgprint(ASTNode *p) {
  if (walk_pass == 1) {
    walk_stack(p->printn.expr);
    return;
  }

  // handle expression value transfer
  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

  inference_expr_type(p->printn.expr);
  walk_stack(p->printn.expr);
  auto pair = pop_right_value();
  Value *v = pair.first;
  Function *printf_fn = ir1.module().getFunction("printf");
  if (!printf_fn)
    yyerror("cannot find declared extern printf function");

  if (p->printn.expr->litn.litv.littypetok == CSTRING) {
    llvmcode_printf(printf_fn, "%s", v, nullptr);
    return;
  }

  dbgprint_value(printf_fn, pair.second, v);

#ifdef TEST_RUNTIME
  test_rt_add();
#endif
}

static void walk_dbgprinttype(ASTNode *p) {
  if (walk_pass == 1)
    return;

  CADataType *dt = catype_get_by_name(p->symtable, p->printtypen.type);
  int typesize = dt->size;

  // print datatype information when compiling
  // debug_catype_datatype(dt);

#if 1 // when need print in run uncomment it
  Function *printf_fn = ir1.module().getFunction("printf");
  if (!printf_fn)
    yyerror("cannot find declared extern printf function");

  // handle expression value transfer
  const char *format = "size = %lu, type: %s\n";
  Constant *format_str = ir1.get_global_string(format);

  // TODO: type string
  Constant *type_str = ir1.get_global_string(symname_get(dt->signature));

  Value *value = ir1.gen_int((uint64_t)typesize);
  std::vector<Value *> printf_args(1, format_str);
  printf_args.push_back(value);
  printf_args.push_back(type_str);

  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

  ir1.builder().CreateCall(printf_fn, printf_args, "n");
#endif
}

// How to inference the type of variable and determine the literal type in right
// expression?
// 1. when variable type already determined, the variable type will used to
// guide and check the right side type. it requires the right expression have
// the same type as the variable.
//   when the right side expression is a final (left) expression (id or literal)
//   1) When the right side type already determined then check if it match the
//   variable type, when not match then report an error.
//   2) When the right side type not determined, then try to determinate the
//   right side type as the same type of the variable. when cannot make the same
//   type then error occurs.
//   when the right side is a complex expression, then recursive the procedure
//   of following
//   1) the complex expression finally should have the same type as the variable
//   2) each children expression of the complex expression should have the same
//   type as the variable, then recursive each children expression for the type
//   until to leaf and when arrive to leaf it is the procedure of above.
//
// 2. when variable type not determined, the variable's type need inferenced
// according to the right expression, it will use the right side expression's
// type as it's type.
//   when the right side is a final expression (id or literal) then use the id
//   or the literal's type as the variable's type.
//   2) when the literal type is determined (with a postfix type), then check if
//   the literal value matches the determined type.
//
//   when the right side is a complex expression, then all the children node of
//   it should have or inferenced into the same type, when they have different
//   type then report error. when all have the same type then the variable type
//   uses it as it's type.
//   1) for each children of the right side expression, do determinate their's
//   type recursively (because the children may also have it's children, so it's
//   recursive), the recursive finally case is the above condition (final
//   expression).
//   TBD: how to spread one determined type in the deep tree structure
//   e.g. let a = (3243 + (432432 + (3432 * 43243 + 43i64 * (433 + 232)))) +
//   333;
//               +
//         +        333
//   3243        +
//        432432     +
//                *             *
//           3432   43243 43i64    +
//                             433   232
//
//   the 43i64 will make all the other part have the same type with it, the
//   32i64 is deeper and in different layers, how to determined it?
//
// Generally, the type of left and right side have following regular:
// 1) when both side have not determined a type then, the right side will use
// the common literal regular to determine the expression's type, and the left
// variable use the right side's type
// 2) when both side have a determined type then they should have the same type,
// or report an error.
// 3) when left side determined a type, right side not determined a type, then
// the left side will guide the right side's type and will check if the literal
// matches the left side type
// 4) when left side not determined a type and the right side determined a type
// then the left side will use the right side's type
// 5) when the right side is complexed expression, then all the part of the
// expression should have the same type. when some part not have a determined
// type then it will use the type in the same expression. 
//

// consider following 4 condition, for an assigment:
// 1) left have a type, right have a type, then check if they are the same
// 2) left have a type, right no type, then invoke `determine_literal_type`
// function uses left's type as the expression's type
// 3) left no type, right have a type, then use the right side type as the left
// (variable)'s type
// 4) both have no type, then inference the right side expression type with
// default one and apply it into the left side variable
static void inference_assign_type(ASTNode *idn, ASTNode *exprn, int assignop = -1) {
  typeid_t expr_types[2];
  ASTNode *group[2] = {idn, exprn};
  expr_types[0] = get_expr_type_from_tree(idn);
  expr_types[1] = get_expr_type_from_tree(exprn);

  if (expr_types[0] != typeid_novalue) {
    CADataType *idncatype = catype_get_by_name(idn->symtable, expr_types[0]);
    CHECK_GET_TYPE_VALUE(idn, idncatype, expr_types[0]);
    expr_types[0] = idncatype->signature;
  }

  if (expr_types[1] != typeid_novalue) {
    CADataType *idncatype = catype_get_by_name(exprn->symtable, expr_types[1]);
    CHECK_GET_TYPE_VALUE(idn, idncatype, expr_types[1]);
    expr_types[1] = idncatype->signature;
  }

  if (expr_types[0] == typeid_novalue && expr_types[1] == typeid_novalue) {
    expr_types[1] = inference_expr_type(exprn);
    if (expr_types[1] == typeid_novalue) {
      caerror(&(exprn->begloc), &(exprn->endloc), "inference expression type failed");
      return;
    }

    // if (expr_types[1] != typeid_novalue) {
    //   CADataType *idncatype = catype_get_by_name(idn->symtable, expr_types[1]);
    //   CHECK_GET_TYPE_VALUE(idn, idncatype, expr_types[1]);
    //   expr_types[1] = idncatype->signature;
    // }
  }
  
  reduce_node_and_type_group(group, expr_types, 2, assignop);
}

static void walk_assign(ASTNode *p) {
  if (walk_pass == 1)
    return;

  // idn can be type of TTE_Id or TTE_DerefLeft or TTE_ArrayItemLeft
  ASTNode *idn = p->assignn.id;
  ASTNode *exprn = p->assignn.expr;
  int assignop = p->assignn.op;

  // TODO: TTE_VarDefZeroValue seems not support assign, only support binding (let)
  if (exprn->type != TTE_VarDefZeroValue)
    inference_assign_type(idn, exprn, assignop);

  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

  CADataType *dt = nullptr;
  switch (idn->type) {
  case TTE_Id:
  case TTE_DerefLeft:
  case TTE_ArrayItemLeft:
  case TTE_StructFieldOpLeft: {
    typeid_t id = get_expr_type_from_tree(idn);
    dt = catype_get_by_name(idn->symtable, id);
    CHECK_GET_TYPE_VALUE(p, dt, id);
    break;
  }
  default:
    caerror(&(p->begloc), &(p->endloc), "wrong type `%d` of left value assignment", idn->type);
    break;
  }

  bool iscomplextype = catype_is_complex_type(dt);

  // when zero_initialize is true, it means to initialize the new allocated
  // variable of specified type with all zero value
  bool zero_initialize = false;
  Value *v = nullptr;
  if (exprn->type != TTE_VarDefZeroValue) {
    walk_stack(exprn);
    if (exprn->type == TTE_Expr && exprn->exprn.op == ADDRESS)
      iscomplextype = true;

    auto pair = pop_right_value("tmpexpr", !iscomplextype);
    v = pair.first;
    if (assignop == -1 && !catype_check_identical(dt, pair.second)) {
      caerror(&(p->begloc), &(p->endloc), "expected a type `%s`, but found `%s`",
	      catype_get_type_name(dt->signature), catype_get_type_name(pair.second->signature));
      return;
    }
  } else { // zero initial value
    // TODO: TTE_VarDefZeroValue seems not support assign, only support binding (let)
    // handle left value type of TTE_Id for zero initialized value
    assert(idn->type == TTE_Id);
    typeid_t id = get_expr_type_from_tree(idn);
    if (id == typeid_novalue) {
      caerror(&(idn->begloc), &(idn->endloc), "type of variable '%s' must be determined for zero initialized value",
	      symname_get(idn->idn.i));
      return;
    }

    zero_initialize = true;
  }

  Value *vp = nullptr;
  if (is_var_declare(idn))
    vp = walk_id_defv_declare(idn, dt, zero_initialize, v);
  else
    vp = walk_id_defv(idn, dt, assignop, zero_initialize, v);
  
  // in fact the pushed value should not used, because value assignment syntax is
  // not an expresssion ande have no a value
  auto u = std::make_unique<CalcOperand>(OT_Alloc, vp, dt);
  oprand_stack.push_back(std::move(u));
}

static CADataType *capattern_check_get_type(CAPattern *cap, ASTNode *exprn) {
  // the pattern type, cap->datatype and exprn type should all be identical
  CADataType *catype = catype_from_capattern(cap, exprn->symtable);
  if (cap->datatype != typeid_novalue) {
    CADataType *postcatype = catype_get_by_name(exprn->symtable, cap->datatype);
    CHECK_GET_TYPE_VALUE(exprn, postcatype, cap->datatype);

    if (!catype) {
      catype = postcatype;
    } else if (postcatype->signature != catype->signature) {
      caerror(&(cap->loc), NULL, "specified type `%s` not equal pattern type `%s`",
	      catype_get_type_name(postcatype->signature), catype_get_type_name(catype->signature));
      return nullptr;
    }
  }

  return catype;
}

static void register_variable_catype(int var, typeid_t type, SymTable *symtable) {
  STEntry *entry = sym_getsym(symtable, var, 0);
  generic_type_record_replace(&entry->u.varshielding.current->datatype, entry->u.varshielding.current->datatype, type);
  entry->u.varshielding.current->datatype = type;
}

static void bind_register_variable_catype(void *morebind, typeid_t type, SymTable *symtable) {
  size_t size = vec_size(morebind);
  for (size_t i = 0; i < size; ++i) {
    int name = (int)(long)vec_at(morebind, i);
    register_variable_catype(name, type, symtable);
  }
}

static void capattern_register_variable_catype(CAPattern *cap, typeid_t type, SymTable *symtable) {
  register_variable_catype(cap->name, type, symtable);
  bind_register_variable_catype(cap->morebind, type, symtable);
}

static int capattern_ignorerange_pos(CAPattern *cap) {
  for (int i = 0; i < cap->items->size; ++i) {
    if (cap->items->patterns[i]->type == PT_IgnoreRange)
      return i;
  }

  return -1;
}

static void inference_letbind_type_both_side(CAPattern *rotate_top_cap, CAPattern *cap, ASTNode *exprn);

// determine variable types in pattern and do format checking for let binding operation
static void inference_letbind_pattern_range(CAPattern *rotate_top_cap, CAPattern *cap, ASTNode *tuplenode, int from, int to, int typeoffset) {
  for (int i = from; i < to; ++i) {
    inference_letbind_type_both_side(rotate_top_cap, cap->items->patterns[i], tuplenode->arglistn.exprs[i + typeoffset]);
  }
}

static void determine_letbind_type(CAPattern *cap, CADataType *catype, SymTable *symtable);
static void determine_letbind_type_range(CAPattern *cap, CADataType *catype, int from, int to, SymTable *symtable) {
  for (int i = from; i < to; ++i) {
    determine_letbind_type(cap->items->patterns[i], catype, symtable);
  }	
}

static void varshielding_rotate_capattern(CAPattern *cap, SymTable *symtable, bool is_back = false);

// parameter `rotate_top_cap` used for handle the case of `let (f1, f2) = (1, 2); let (f2, f1) = (f1, f2)`
static void inference_letbind_type_both_side(CAPattern *rotate_top_cap, CAPattern *cap, ASTNode *exprn) {
  switch (cap->type) {
  case PT_IgnoreOne:
  case PT_IgnoreRange:
  case PT_Var: {
    // for variable shielding, resolving `let a = a;` statement
    varshielding_rotate_capattern(rotate_top_cap, exprn->symtable, true);
    typeid_t type = inference_expr_type(exprn);
    varshielding_rotate_capattern(rotate_top_cap, exprn->symtable, false);
    if (cap->type == PT_Var)
      capattern_register_variable_catype(cap, type, exprn->symtable);

    break;
  }
  case PT_GenTuple: {
    if (exprn->type == TTE_Id) {
      // handle the condition when exprn->type s TTE_Id, the type is just come from the left side
      varshielding_rotate_capattern(rotate_top_cap, exprn->symtable, true);
      typeid_t type = inference_expr_type(exprn);
      varshielding_rotate_capattern(rotate_top_cap, exprn->symtable, false);

      CADataType *catype = catype_get_by_name(exprn->symtable, type);
      CHECK_GET_TYPE_VALUE(exprn, catype, type);
      determine_letbind_type(cap, catype, exprn->symtable);

      if (cap->morebind)
	bind_register_variable_catype(cap->morebind, catype->signature, exprn->symtable);

      break;
    }

    if (exprn->type != TTE_Expr || exprn->exprn.op != TUPLE || exprn->exprn.noperand != 1) {
      caerror(&(cap->loc), NULL, "the right side expression is not a general tuple type: %d", cap->type);
      return;
    }

    ASTNode *tuplenode = exprn->exprn.operands[0];
    assert(tuplenode->type == TTE_ArgList);

    int ignorerangepos = capattern_ignorerange_pos(cap);
    if (ignorerangepos == -1 && cap->items->size != tuplenode->arglistn.argc) {
      caerror(&(cap->loc), NULL, "pattern have different fields `%d` than `%d` of left expression",
	      cap->items->size, tuplenode->arglistn.argc);
      return;
    }

    if (ignorerangepos == -1) {
      // with no ignore range ..
      inference_letbind_pattern_range(rotate_top_cap, cap, tuplenode, 0, cap->items->size, 0);
    } else {
      // with ignore range .., x1, x2, .., xm, xn, example: (v1, v2, ..(2), vm, vn) = (t1, t2, t3, ..., tx, tm, tn)
      // handle starting and ending matches
      inference_letbind_pattern_range(rotate_top_cap, cap, tuplenode, 0, ignorerangepos, 0);
      inference_letbind_pattern_range(rotate_top_cap, cap, tuplenode, ignorerangepos + 1, cap->items->size, tuplenode->arglistn.argc - cap->items->size);
    }

    CADataType *catype = catype_from_capattern(cap, exprn->symtable);
    if (!catype)
      caerror(&(cap->loc), NULL, "get general tuple type `%s` failed from determined pattern");

    if (cap->morebind)
      bind_register_variable_catype(cap->morebind, catype->signature, exprn->symtable);

    varshielding_rotate_capattern(rotate_top_cap, exprn->symtable, true);
    determine_expr_type(exprn, catype->signature);
    varshielding_rotate_capattern(rotate_top_cap, exprn->symtable, false);
    break;
  }
  case PT_Array: {
    int ignorerangepos = capattern_ignorerange_pos(cap);

    if (exprn->type != TTE_Id) {
      if (exprn->type != TTE_Expr || exprn->exprn.op != ARRAY || exprn->exprn.noperand != 1) {
	caerror(&(cap->loc), NULL, "the right side expression is not an array type: %d", cap->type);
	return;
      }

      ASTNode *arraynode = exprn->exprn.operands[0];
      assert(arraynode->type == TTE_ArrayDef);
    }

    // inference right side type directly and determine right side type
    varshielding_rotate_capattern(rotate_top_cap, exprn->symtable, true);
    typeid_t type = inference_expr_type(exprn);
    varshielding_rotate_capattern(rotate_top_cap, exprn->symtable, false);

    CADataType *catype = catype_get_by_name(exprn->symtable, type);
    CHECK_GET_TYPE_VALUE(exprn, catype, type);
    if (catype->type != ARRAY) {
      caerror(&(cap->loc), NULL, "expected array type but find `%s` for right side variable",
	      catype_get_type_name(type));
    }

    if (ignorerangepos == -1 && cap->items->size != catype->array_layout->dimarray[0]) {
      caerror(&(cap->loc), NULL, "pattern have different fields `%d` than `%d` of left expression",
	      cap->items->size, catype->array_layout->dimarray[0]);
      return;
    }

    CADataType *subcatype = catype->array_layout->type;

    if (ignorerangepos == -1) {
      // with no ignore range .. condition: `let [a, b, c] = [1, 2, 3]`, then inference type only from right side
      // for this condition inference type from left side: `let [a, b, c]: [u8; 3] = [1, 2, 3]`, it should already
      // be done in function `capattern_check_get_type -> catype_from_capattern`, so here need not coping with it
      determine_letbind_type(cap, catype, exprn->symtable);
    } else {
      // with ignore range .., x1, x2, .., xm, xn condition: `(v1, v2, ..(2), vm, vn) = (t1, t2, t3, ..., tx, tm, tn)`
      // handle starting and ending matches
      determine_letbind_type_range(cap, subcatype, 0, ignorerangepos, exprn->symtable);
      determine_letbind_type_range(cap, subcatype, ignorerangepos + 1, cap->items->size, exprn->symtable);
    }

    if (cap->morebind)
      bind_register_variable_catype(cap->morebind, catype->signature, exprn->symtable);

    break;
  }
  case PT_Tuple:
  case PT_Struct:
    // following 2 case should cannot come here, because it's caller `inference_letbind_type`
    // already handlered and returned it when in catype != NULL case, because PT_Tuple and
    // PT_Struct all have a name and in function `capattern_check_get_type` which is invoked
    // by function `inference_letbind_type`, it can get the datatype according to the name,
    // so need needed here
    caerror(&exprn->begloc,  &exprn->endloc, NULL,
	    "(internal) Pattern type `%s` should not come here for inferencing",
	    cap->type == PT_Tuple ? "PT_Tuple" : "PT_Struct"
	    );
    break;
  default:
    caerror(&(cap->loc), NULL, "Unknown pattern type `%d` when inferencing type", cap->type);
    break;
  }
}

static void inference_letbind_type(CAPattern *cap, ASTNode *exprn) {
  CADataType *catype = capattern_check_get_type(cap, exprn);
  if (catype) {
    determine_letbind_type(cap, catype, exprn->symtable);

    varshielding_rotate_capattern(cap, exprn->symtable, true);
    determine_expr_type(exprn, catype->signature);
    varshielding_rotate_capattern(cap, exprn->symtable, false);
    return;
  }

  // when cannot directly get datatype from pattern then inference type for / from both side (right expression)
  inference_letbind_type_both_side(cap, cap, exprn);
}

// determine variable types in pattern and do format checking for let binding operation
static void determine_letbind_pattern_range(CAPattern *cap, CADataType *catype, SymTable *symtable, int from, int to, int typeoffset) {
  for (int i = from; i < to; ++i) {
    determine_letbind_type(cap->items->patterns[i], catype->struct_layout->fields[i + typeoffset].type, symtable);
  }
}

static int struct_field_position_from_fieldname(CADataType *catype, int fieldname) {
  for (int i = 0; i < catype->struct_layout->fieldnum; ++i) {
    if (catype->struct_layout->fields[i].name == fieldname)
      return i;
  }

  return -1;
}

static CADataType *struct_subcatype_from_fieldname(CADataType *catype, int fieldname) {
  int pos = struct_field_position_from_fieldname(catype, fieldname);
  if (pos == -1)
    return nullptr;

  return catype->struct_layout->fields[pos].type;
}

static void determine_letbind_type_for_struct(CAPattern *cap, CADataType *catype, SymTable *symtable) {
  // come here struct or named tuple must already defined datatype
  if (catype->type != STRUCT) {
    caerror(&(cap->loc), NULL, "required a struct type, but found `%s` type", catype_get_type_name(catype->signature));
    return;
  }

  if (cap->name) {
    CADataType *dt = catype_from_capattern(cap, symtable);
    if (dt->signature != catype->signature) {
      caerror(&(cap->loc), NULL, "`%s` type required, but find `%s` pattern type",
	      catype_get_type_name(catype->signature), catype_get_type_name(dt->signature));
      return;
    }
  }

  if ((cap->items->size > catype->struct_layout->fieldnum)) {
    caerror(&(cap->loc), NULL, "pattern have more field `%d` than `%d` of datatype `%s`",
	    cap->items->size, catype->struct_layout->fieldnum,
	    catype_get_type_name(catype->signature));
    return;
  }

  if (cap->morebind)
    bind_register_variable_catype(cap->morebind, catype->signature, symtable);

  // determine variable position in tuple and related catype and recursive invoke this function
  int ignorerangepos = capattern_ignorerange_pos(cap);
  if (ignorerangepos == -1 && cap->items->size != catype->struct_layout->fieldnum) {
    caerror(&(cap->loc), NULL, "pattern have less field `%d` than `%d` of datatype `%s`",
	    cap->items->size, catype->struct_layout->fieldnum,
	    catype_get_type_name(catype->signature));
    return;
  }

  // implement variable position in struct
  if (cap->type == PT_Struct) {
    int endpos = cap->items->size;
    if (ignorerangepos != -1) {
      assert(ignorerangepos == cap->items->size - 1);
      endpos = ignorerangepos;
    }

    assert(catype->struct_layout->type < Struct_GeneralTuple);
    for (int i = 0; i < endpos; ++i) {
      int fieldname = cap->items->patterns[i]->fieldname;
      assert(fieldname != -1);
      CADataType *dt = nullptr;
      if (catype->struct_layout->type == Struct_NamedTuple) {
	// when it is a named tuple, the fieldname is the tuple item position
	if (fieldname >= catype->struct_layout->fieldnum) {
	  caerror(&(cap->loc), NULL, "tuple numbered field `%d` out of range `(0 ~ %d]` of datatype `%s`",
		  fieldname, catype->struct_layout->fieldnum, catype_get_type_name(catype->signature));
	  return;
	}

        dt = catype->struct_layout->fields[fieldname].type;
      } else {
	dt = struct_subcatype_from_fieldname(catype, fieldname);
	if (!dt) {
	  caerror(&(cap->loc), NULL, "cannot find field `%s` from datatype `%s`",
		  symname_get(fieldname), catype_get_type_name(catype->signature));
	  return;
	}
      }
      determine_letbind_type(cap->items->patterns[i], dt, symtable);
    }
    return;
  }

  if (ignorerangepos == -1) {
    // with no ignore range ..
    determine_letbind_pattern_range(cap, catype, symtable, 0, cap->items->size, 0);
  } else {
    // with ignore range .., x1, x2, .., xm, xn, example: (v1, v2, ..(2), vm, vn) = (t1, t2, t3, ..., tx, tm, tn)
    // handle starting and ending matches
    determine_letbind_pattern_range(cap, catype, symtable, 0, ignorerangepos, 0);
    determine_letbind_pattern_range(cap, catype, symtable, ignorerangepos + 1, cap->items->size, catype->struct_layout->fieldnum-cap->items->size);
  }
}

static void determine_letbind_type(CAPattern *cap, CADataType *catype, SymTable *symtable) {
  switch (cap->type) {
  case PT_Var:
    capattern_register_variable_catype(cap, catype->signature, symtable);
    break;
  case PT_Tuple:
  case PT_GenTuple:
  case PT_Struct:
    // when `catype->struct_layout->type` is not `Struct_NamedStruct` it means the left pattern used
    // struct form matching for the right object (tuple)
    determine_letbind_type_for_struct(cap, catype, symtable);
    break;
  case PT_Array:
    determine_letbind_type_range(cap, catype->array_layout->type, 0, cap->items->size, symtable);
    break;
  case PT_IgnoreOne:
    break;
  case PT_IgnoreRange:
    yyerror("inner error: should not come here, upper logic already processed");
    break;
  default:
    yyerror("inner error: unknown pattern type: `%d` when determining type", cap->type);
    break;
  }
}

static Value *bind_variable_value(SymTable *symtable, int name, Value *value, bool inplace_value, VarInitType init_type) {
  STEntry *entry = sym_getsym(symtable, name, 0);
  assert(entry);

  CADataType *catype = catype_get_by_name(symtable, entry->u.varshielding.current->datatype);

  const char *varname = symname_get(entry->u.varshielding.current->name);
  Type *type = llvmtype_from_catype(catype);
  if (entry->sym_type != Sym_Variable) {
    caerror(&(entry->sloc), NULL, "line: %d, col: %d: '%s' Not a variable", varname);
    return nullptr;
  }

  Value *var = nullptr;

  if (is_create_global_var(entry)) { // or using condition: symtable == &g_root_symtable
    var = ir1.gen_global_var(type, varname, value, false, value == nullptr);

    if (enable_debug_info())
      emit_global_var_dbginfo(varname, catype, entry->u.varshielding.current->loc.row);
  } else {
    Type::TypeID id = type->getTypeID();
    if (value && inplace_value && (id == Type::ArrayTyID || id == Type::StructTyID)) {
      var = value;
    } else {
      var = ir1.gen_entry_block_var(curr_fn, type, varname, nullptr);
      if (!value) {
        if (init_type == VarInit_Zero)
          aux_set_zero_to_store(type, var);
      } else {
	aux_copy_llvmvalue_to_store(type, var, value, varname);
      }
    }  

    if (enable_debug_info())
      emit_local_var_dbginfo(curr_fn, varname, catype, var, entry->u.varshielding.current->loc.row /* p->endloc.row */);
  }

  entry->u.varshielding.current->llvm_value = static_cast<void *>(var);
  return var;
}

static void atmore_bind_variable_value(SymTable *symtable, void *morebind, Value *value, VarInitType init_type) {
  size_t size = vec_size(morebind);
  for (size_t i = 0; i < size; ++i) {
    int name = (int)(long)vec_at(morebind, i);
    bind_variable_value(symtable, name, value, false, init_type);
  }
}

static void capattern_bind_variable_value(SymTable *symtable, CAPattern *cap,
                                          Value *value, bool inplace_value,
                                          VarInitType init_type)
{
  bind_variable_value(symtable, cap->name, value, inplace_value, init_type);
  atmore_bind_variable_value(symtable, cap->morebind, value, init_type);
}

static void capattern_bind_value(SymTable *symtable, CAPattern *cap,
                                 Value *value, bool inplace_value,
                                 CADataType *catype, VarInitType init_type);

// determine variable types in pattern and do format checking for let binding operation
static void capattern_bind_tuple_pattern_range(SymTable *symtable,
                                               CAPattern *cap, Value *value,
                                               CADataType *catype, int from,
                                               int to, int typeoffset,
                                               VarInitType init_type)
{
  Value *idxv0 = ir1.gen_int((int)0);
  std::vector<Value *> idxv(2, idxv0);

  for (int i = from; i < to; ++i) {
    int opos = i + typeoffset;
    CADataType *btype = catype->struct_layout->fields[opos].type;
    Value *subvalue = nullptr;
    if (value) {
      Value *idxvi = ir1.gen_int(opos);
      idxv[1] = idxvi;
      subvalue = ir1.builder().CreateGEP(value, idxv);

      if (!catype_is_complex_type(btype))
	subvalue = ir1.builder().CreateLoad(subvalue, "pat");
    }

    capattern_bind_value(symtable, cap->items->patterns[i], subvalue, false, btype, init_type);
  }
}

static void capattern_bind_struct_value(SymTable *symtable, CAPattern *cap,
                                        Value *value, CADataType *catype,
                                        VarInitType init_type)
{
  // when in this function the value should already come with the type of capattern
  if (cap->morebind)
    atmore_bind_variable_value(symtable, cap->morebind, value, init_type);

  // get the ignore variant position in struct / tuple, the check should already checked previously
  int ignorerangepos = capattern_ignorerange_pos(cap);

  // implement variable position in struct
  if (cap->type == PT_Struct) {
    int endpos = cap->items->size;
    if (ignorerangepos != -1) {
      endpos = ignorerangepos;
    }

    Value *idxv0 = ir1.gen_int((int)0);
    std::vector<Value *> idxv(2, idxv0);

    for (int i = 0; i < endpos; ++i) {
      int pos = cap->items->patterns[i]->fieldname;
      if (catype->struct_layout->type == Struct_NamedStruct)
	pos = struct_field_position_from_fieldname(catype, pos);

      CADataType *btype = catype->struct_layout->fields[pos].type;
      Value *subvalue = nullptr;
      if (value) {
	// get elements address of structure
	Value *idxvi = ir1.gen_int(pos);
	idxv[1] = idxvi;
	subvalue = ir1.builder().CreateGEP(value, idxv);
	if (!catype_is_complex_type(btype))
	  subvalue = ir1.builder().CreateLoad(subvalue, "pat");
      }

      capattern_bind_value(symtable, cap->items->patterns[i], subvalue, false, btype, init_type);
    }
    return;
  }

  // handle tuple value like upper
  if (ignorerangepos == -1) {
    // with no ignore range ..
    capattern_bind_tuple_pattern_range(symtable, cap, value, catype, 0,
                                       cap->items->size, 0, init_type);
  } else {
    // with ignore range .., x1, x2, .., xm, xn, example: (v1, v2, ..(2), vm,
    // vn) = (t1, t2, t3, ..., tx, tm, tn) handle starting and ending matches
    capattern_bind_tuple_pattern_range(symtable, cap, value, catype, 0,
                                       ignorerangepos, 0, init_type);
    capattern_bind_tuple_pattern_range(
        symtable, cap, value, catype, ignorerangepos + 1, cap->items->size,
        catype->struct_layout->fieldnum - cap->items->size, init_type);
  }
}

static void capattern_bind_array_pattern_range(SymTable *symtable,
                                               CAPattern *cap, Value *value,
                                               CADataType *catype, int from,
                                               int to, int typeoffset,
					       VarInitType init_type)
{
  Value *idxv0 = ir1.gen_int((int)0);
  std::vector<Value *> idxv(2, idxv0);

  CADataType *btype = catype->array_layout->type;

  for (int i = from; i < to; ++i) {
    int opos = i + typeoffset;
    Value *subvalue = nullptr;
    if (value) {
      Value *idxvi = ir1.gen_int(opos);
      idxv[1] = idxvi;
      subvalue = ir1.builder().CreateGEP(value, idxv);

      if (!catype_is_complex_type(btype))
	subvalue = ir1.builder().CreateLoad(subvalue, "pat");
    }

    capattern_bind_value(symtable, cap->items->patterns[i], subvalue, false, btype, init_type);
  }
}

static void capattern_bind_array_value(SymTable *symtable, CAPattern *cap,
                                       Value *value, CADataType *catype,
                                       VarInitType init_type)
{
  // when in this function the value should already come with the type of capattern
  if (cap->morebind)
    atmore_bind_variable_value(symtable, cap->morebind, value, init_type);

  // get the ignore variant position in struct / tuple, the check should already checked previously
  int ignorerangepos = capattern_ignorerange_pos(cap);

  if (ignorerangepos == -1) {
    // with no ignore range ..
    capattern_bind_array_pattern_range(symtable, cap, value, catype, 0,
                                       cap->items->size, 0, init_type);
  } else {
    // with ignore range .., x1, x2, .., xm, xn, example: (v1, v2, ..(2), vm,
    // vn) = (t1, t2, t3, ..., tx, tm, tn) handle starting and ending matches
    capattern_bind_array_pattern_range(symtable, cap, value, catype, 0,
                                       ignorerangepos, 0, init_type);
    capattern_bind_array_pattern_range(
        symtable, cap, value, catype, ignorerangepos + 1, cap->items->size,
        catype->array_layout->dimarray[0] - cap->items->size, init_type);
  }
}

// bind value for the variable variant in the pattern
static void capattern_bind_value(SymTable *symtable, CAPattern *cap,
                                 Value *value, bool inplace_value,
                                 CADataType *catype, VarInitType init_type) {
  switch (cap->type) {
  case PT_Var:
    capattern_bind_variable_value(symtable, cap, value, inplace_value, init_type);
    break;
  case PT_Tuple:
  case PT_GenTuple:
  case PT_Struct:
    // when `catype->struct_layout->type` is not `Struct_NamedStruct` it means the left pattern used
    // struct form matching for the right object (tuple)
    capattern_bind_struct_value(symtable, cap, value, catype, init_type);
    break;
  case PT_Array:
    capattern_bind_array_value(symtable, cap, value, catype, init_type);
    break;
  case PT_IgnoreOne:
    break;
  case PT_IgnoreRange:
    yyerror("inner error: should not come here, upper logic already processed");
    break;
  default:
    yyerror("inner error: unknown pattern type: `%d` when binding value", cap->type);
    break;
  }
}

static void varshielding_rotate_variable(CAVariableShielding *shielding, bool is_back) {
  if (vec_size(shielding->varlist)) {
    if (is_back) {
      vec_pushfront(shielding->varlist, shielding->current);
      shielding->current = (CAVariable *)vec_popback(shielding->varlist);
    } else {
      vec_append(shielding->varlist, shielding->current);
      shielding->current = (CAVariable *)vec_popfront(shielding->varlist);
    }
  }
}

static void varshielding_rotate_capattern_variable_single(int name, SymTable *symtable, bool is_back = false) {
  STEntry *entry = sym_getsym(symtable, name, 0);
  varshielding_rotate_variable(&entry->u.varshielding, is_back);
}

static void varshielding_rotate_capattern_variable_bind(void *morebind, SymTable *symtable, bool is_back = false) {
  size_t size = vec_size(morebind);
  for (size_t i = 0; i < size; ++i) {
    int name = (int)(long)vec_at(morebind, i);
    STEntry *entry = sym_getsym(symtable, name, 0);
    varshielding_rotate_variable(&entry->u.varshielding, is_back);
  }
}

static void varshielding_rotate_capattern_variable(CAPattern *cap, SymTable *symtable, bool is_back = false) {
  varshielding_rotate_capattern_variable_single(cap->name, symtable, is_back);
  varshielding_rotate_capattern_variable_bind(cap->morebind, symtable, is_back);
}

static void varshielding_rotate_capattern_struct(CAPattern *cap, SymTable *symtable, bool is_back = false) {
  varshielding_rotate_capattern_variable_bind(cap->morebind, symtable, is_back);

  PatternGroup *pg = cap->items;
  for (int i = 0; i < pg->size; ++i) {
    varshielding_rotate_capattern(pg->patterns[i], symtable, is_back);
  }
}

static void varshielding_rotate_capattern(CAPattern *cap, SymTable *symtable, bool is_back) {
  switch (cap->type) {
  case PT_Var:
    varshielding_rotate_capattern_variable(cap, symtable, is_back);
    break;
  case PT_Array:
  case PT_Tuple:
  case PT_GenTuple:
  case PT_Struct:
    varshielding_rotate_capattern_struct(cap, symtable, is_back);
    break;
  case PT_IgnoreOne:
  case PT_IgnoreRange:
    break;
  default:
    yyerror("inner error: unknown pattern type: `%d` when rotate var shielding", cap->type);
    break;
  }
}

// TODO: for local and global variable binding, refactor walk_assign function
static void walk_letbind(ASTNode *p) {
  if (walk_pass == 1)
    return;

  CAPattern *cap = p->letbindn.cap;
  ASTNode *exprn = p->letbindn.expr;
  Value *v = nullptr;
  CADataType *catype = nullptr;
  OperandType ot = OT_Alloc;
  VarInitType init_type = VarInit_Zero;
    
  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

  // The variable rebind functionality have rebind order, the rebind list is created in the order of
  // the rebind declaratin in the first pass (building AST tree), and the current variable in rebind
  // list is the last rebind variable after the AST tree is just build over, and so in the walk pass,
  // it will need firstly rotate forward and make current point to the first binded variable, so here
  // need rotate forward operation
  varshielding_rotate_capattern(cap, p->symtable);

  if (exprn->type != TTE_VarDefZeroValue) {
    // 1. inference type for both side of binding, to determine the types of both side
    // when is not zeroinitial
    // Should rotating as whole, the reason see segment `Rotation As a Whole` of document `variable-shielding.md`
    inference_letbind_type(cap, exprn);

    // 2. walk right side node and get Value
    // Here is a convention about rotation: the current variable bind is already reset to the last rebind variable
    // in the rebind list, so that the inner rotate operation is also in the order. So here will rotate
    // back to the last rebind variable, and in walk_stack will also using this convention
    varshielding_rotate_capattern(cap, exprn->symtable, true);
    walk_stack(exprn);

    // the inner walk operation for variabe bind will already reset to the state of ...
    varshielding_rotate_capattern(cap, exprn->symtable, false);

    //auto pair = pop_right_value("tmpexpr", false);
    //pair.first;
    auto datao = pop_right_operand("tmpexpr", false);
    v = datao->operand;
    catype = datao->catype;
    ot = datao->type;
    if (v == nullptr) {
      caerror(&(p->begloc), &(p->endloc), "create value failed");
      return;
    }
    // if (!catype_check_identical(dt, pair.second)) {
    // 	caerror(&(p->begloc), &(p->endloc), "expected a type `%s`, but found `%s`",
    // 		catype_get_type_name(dt->signature), catype_get_type_name(pair.second->signature));
    // 	return;
    // }

  } else {
    // 1. inference type for both side of binding, to determine the types of both side
    // when the pattern specified a type, example: let AA {f1, f2, ...}: datatype = __zero_initial__
    catype = capattern_check_get_type(cap, exprn);
    CHECK_GET_TYPE_VALUE(exprn, catype, cap->datatype);

    // catype can be null and the datatype must can obtain from pattern
    determine_letbind_type(cap, catype, exprn->symtable);

    init_type = exprn->varinitn.type;
  }

  if (v && ot == OT_Alloc && !catype_is_complex_type(catype))
    v = ir1.builder().CreateLoad(v, "tmpexpr");

  // 3. walk left side again and copy data from right side
  // here only when the value v is come from literal directly the inplace_value
  // can be set to true, or when it is a variable, if using insplace method, it
  // will use the same value for 2 variables. TODO: determine the direct literal
  // example and set inplace_value = true
  capattern_bind_value(exprn->symtable, cap, v, false, catype, init_type);
}

static void walk_expr_tuple_common(ASTNode *p, CADataType *catype, std::vector<Value *> &values);
static void walk_range(ASTNode *p) {
  if (walk_pass == 1)
    return;

  GeneralRange *range = &p->rangen.range;
  Value *start_value = nullptr;
  Value *end_value = nullptr;
  CADataType *start_type = nullptr;
  CADataType *end_type = nullptr;
  OperandType start_optype = OT_Const;
  OperandType end_optype = OT_Const;
  OperandType optype = OT_Const;

  bool iscomplextype = false;
  if (range->start) {
    walk_stack(range->start);
    iscomplextype = catype_is_complex_type(oprand_stack.back()->catype);
    auto co = pop_right_operand("start", !iscomplextype);
    start_value = co->operand;
    start_type = co->catype;
    start_optype = co->type;
  }

  if (range->end) {
    walk_stack(range->end);
    iscomplextype = catype_is_complex_type(oprand_stack.back()->catype);
    auto co = pop_right_operand("end", !iscomplextype);
    end_value = co->operand;
    end_type = co->catype;
    end_optype = co->type;
  }

  CADataType *catype = catype_from_range(p, (GeneralRangeType)range->type, range->inclusive, start_type, end_type);
  CHECK_GET_TYPE_VALUE(p, catype, 0);
  Value *value = nullptr;
  if (start_value && end_value) {
    std::vector<Value *> values;
    values.push_back(start_value);
    values.push_back(end_value);

    walk_expr_tuple_common(p, catype->range_layout->range, values);
    auto co = pop_right_operand("range", false);
    value = co->operand;
    optype = co->type;
  } else if (start_value) {
    value = start_value;
    optype = start_optype;
  } else if (end_value) {
    value = end_value;
    optype = end_optype;
  } else {
    // when full range, there is no llvm value counterparter, so just use the integer value
    value = ir1.gen_int(0);
    optype = OT_Const;
  }
  
  oprand_stack.push_back(std::make_unique<CalcOperand>(optype, value, catype));
}

static void walk_expr_minus(ASTNode *p) {
  typeid_t type = get_expr_type_from_tree(p->exprn.operands[0]);
  CADataType *dt = catype_get_by_name(p->symtable, type);
  CHECK_GET_TYPE_VALUE(p, dt, type);
  if (catype_is_unsigned(dt->type)) {
    yyerror("unsigned type `%s` cannot apply `-` operator", symname_get(type));
    return;
  }

  walk_stack(p->exprn.operands[0]);

  auto pair = pop_right_value();
  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

  Value *z = ir1.gen_int((uint8_t)0);
  z = ir1.builder().CreateSExt(z, pair.first->getType());
  Value *v = ir1.gen_sub(z, pair.first);
  oprand_stack.push_back(std::make_unique<CalcOperand>(OT_Calc, v, dt));
}

static bool is_bnot_type(tokenid_t type) {
  switch (type) {
  case I8:
  case I16:
  case I32:
  case I64:
  case U8:
  case U16:
  case U32:
  case U64:
  case BOOL:
    return true;
  default:
    return false;
  }
}

static void walk_unary_expr(ASTNode *p) {
  typeid_t type = get_expr_type_from_tree(p->exprn.operands[0]);
  CADataType *dt = catype_get_by_name(p->symtable, type);
  CHECK_GET_TYPE_VALUE(p, dt, type);

  switch (p->exprn.op) {
  case UMINUS:
    if (catype_is_unsigned(dt->type)) {
      yyerror("unsigned type `%s` cannot apply `-` operator", symname_get(type));
      return;
    }
    break;
  case BNOT:
    if (!is_bnot_type(dt->type)) {
      caerror(&(p->begloc), &(p->endloc), "expected integer type for bitwise & logical not, but find `%s`",
	      symname_get(dt->signature));
      return;
    }
    break;
  default:
    caerror(&(p->begloc), &(p->endloc), "unknown unary operator `%d`", p->exprn.op);
    return;
  }

  walk_stack(p->exprn.operands[0]);

  auto pair = pop_right_value();
  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

  Value *v = nullptr;
  if (p->exprn.op == UMINUS) {
    if (catype_is_float(pair.second->type))
      v = ir1.builder().CreateFNeg(pair.first, "fneg");
    else
      v = ir1.builder().CreateNeg(pair.first, "neg");
  } else if (dt->type == BOOL) {
    // bool logic or bitwise not (they are equal)
    v = ir1.builder().CreateNot(pair.first, "not");
  } else {
    // bitwise not
    v = ir1.builder().CreateNot(pair.first, "bitnot");
  }

  oprand_stack.push_back(std::make_unique<CalcOperand>(OT_Calc, v, dt));
}

static typeid_t domain_get_function_name(ASTNode *name) {
  typeid_t fnname = typeid_novalue;
  switch (name->domainfn.type) {
  case DFT_Domain:
    fnname = (int)(long)vec_at(name->domainfn.u.domain->parts, name->domainfn.u.domain->count - 1);
    break;
  case DFT_DomainAs:
    fnname = name->domainfn.u.domain_as->fnname;
    break;
  default:
    caerror(&name->begloc, &name->endloc, "unknown domain type: %d\n", name->domainfn.type);
    break;;
  }

  return fnname;
}

static void check_and_determine_param_type(ASTNode *name, ASTNode *param, int tuple, STEntry *param_entry, STEntry *cls_entry,
					   typeid_t method_struct_signature, int is_direct_call) {
  typeid_t fnname = typeid_novalue;
  switch (name->type) {
  case TTE_Id:
    fnname = name->idn.i;
    break;
  case TTE_Expr:
    fnname = name->exprn.operands[0]->sfopn.fieldname;
    break;
  case TTE_Domain:
    fnname = domain_get_function_name(name);
    break;
  default:
    yyerror("bad function call type: %d", name->type);
  }

  if (name->type == TTE_Id && name->idn.idtype != IdType::TTEId_FnName) {
      caerror(&(param->begloc), &(param->endloc), "the id: `%s` is not `%s` name",
	      symname_get(fnname), tuple ? "tuple" : "function");
      return;
  }

  int is_method = name->type == TTE_Expr;
  check_fn_define(fnname, param, tuple, param_entry, is_method);

  ST_ArgList *formalparam = nullptr;
  if (tuple)
    formalparam = param_entry->u.datatype.members;
  else
    formalparam = param_entry->u.f.arglists;

  if (cls_entry) {
    //if (!runable_is_method_in_struct(cls_entry, fnname)) {
    if (IS_GENERIC_FUNCTION(param_entry->u.f.ca_func_type) || param_entry->u.f.ca_func_type == CAFT_MethodInTrait) {
      SymTableAssoc *assoc = runable_find_entry_assoc(cls_entry, fnname, -1);
      formalparam->symtable->assoc = assoc;
    }
  }

  if (is_method) {
    // check first formal parameter with object type
    STEntry *paramentry = sym_getsym(formalparam->symtable, formalparam->argnames[0], 0);
    typeid_t datatype = paramentry->u.varshielding.current->datatype;
    CADataType *dt = catype_get_by_name(formalparam->symtable, datatype);
    CHECK_GET_TYPE_VALUE(param, dt, datatype);
    typeid_t formaltype = dt->signature;

    typeid_t realtype = method_struct_signature;
    if (is_direct_call) {
      realtype = sym_form_pointer_id(realtype);
    }

    // check the formal parameter and actual parameter type
    if (!catype_check_identical_in_symtable(name->symtable, realtype, param->symtable, formaltype)) {
      caerror(&(param->begloc), &(param->endloc), "the self parameter type '%s' not match the parameter declared type '%s'",
	      catype_get_type_name(realtype), catype_get_type_name(formaltype));
      return;
    }
  }

  // check and determine parameter type
  for (int i = 0; i < param->arglistn.argc; ++i) {
    typeid_t formaltype = typeid_novalue;
    int j = is_method ? i + 1 : i;
    if (j >= formalparam->argc) {
      // it is a variable parameter ...
      formaltype = typeid_novalue;
    } else {
      typeid_t datatype = typeid_novalue;
      if (tuple) {
	datatype = formalparam->types[j];
      } else {
	STEntry *paramentry = sym_getsym(formalparam->symtable, formalparam->argnames[j], 0);

        varshielding_rotate_variable(&paramentry->u.varshielding);
	datatype = paramentry->u.varshielding.current->datatype;
	varshielding_rotate_variable(&paramentry->u.varshielding, true);
      }

      CADataType *dt = catype_get_by_name(formalparam->symtable, datatype);
      CHECK_GET_TYPE_VALUE(param, dt, datatype);
      formaltype = dt->signature;
    }

    typeid_t realtype = formaltype;
    ASTNode *expr = param->arglistn.exprs[i]; // get one parameter

    if (formaltype == typeid_novalue)
      inference_expr_type(expr);
    else
      determine_expr_type(expr, formaltype);

    realtype = get_expr_type_from_tree(expr);
    if (formaltype == typeid_novalue && realtype != typeid_novalue) {
      CADataType *catype = catype_get_by_name(expr->symtable, realtype);
      formaltype = catype->signature;
    }

    // check the formal parameter and actual parameter type
    if (!catype_check_identical_in_symtable(name->symtable, realtype, param->symtable, formaltype)) {
      caerror(&(param->begloc), &(param->endloc), "the %d parameter type '%s' not match the parameter declared type '%s'",
	      i, catype_get_type_name(realtype), catype_get_type_name(formaltype));
      formalparam->symtable->assoc = nullptr;
      return;
    }
  }

  formalparam->symtable->assoc = nullptr;
}

static void walk_expr_tuple_common(ASTNode *p, CADataType *catype, std::vector<Value *> &values) {
  if (catype->type != STRUCT) {
    caerror(&(p->begloc), &(p->endloc), "type `%s` is not a struct type", catype_get_type_name(catype->signature));
    return;
  }

  if (catype->struct_layout->fieldnum != values.size()) {
    caerror(&(p->begloc), &(p->endloc), "struct type `%s` expression field size: `%d` not equal to the struct field size: `%d`",
	    catype_get_type_name(catype->signature), catype->struct_layout->fieldnum, values.size());
    return;
  }

  // allocate new array and copy related elements to the array
  StructType *structype = static_cast<StructType *>(llvmtype_from_catype(catype));
  AllocaInst *structure = ir1.gen_entry_block_var(curr_fn, structype);
  Value *idxv0 = ir1.gen_int((int)0);
  std::vector<Value *> idxv(2, idxv0);

  for (size_t i = 0; i < values.size(); ++i) {
    // get elements address of structure
    Value *idxvi = ir1.gen_int((int)i);
    idxv[1] = idxvi;
    Value *dest = ir1.builder().CreateGEP(// structype, 
					  structure, idxv);
    Type *lefttype = structype->getStructElementType(i);
    aux_copy_llvmvalue_to_store(lefttype, dest, values[i], "field");
  }
  
  auto u = std::make_unique<CalcOperand>(OT_Alloc, structure, catype);
  oprand_stack.push_back(std::move(u));
}

static void walk_expr_tuple(ASTNode *p, STEntry *entry, std::vector<Value *> &values) {
  assert(entry->u.datatype.tuple == 1);

  typeid_t structid = entry->u.datatype.id;
  CADataType *structcatype = catype_get_by_name(entry->u.datatype.idtable, structid);
  CHECK_GET_TYPE_VALUE(p, structcatype, structid);

  p->exprn.expr_type = structcatype->signature;

  walk_expr_tuple_common(p, structcatype, values);
}

static void llvmvalue_from_exprs(ASTNode **exprs, int len, std::vector<Value *> &argv, bool isvalue) {
  for (int i = 0; i < len; ++i) {
    // how to get the name for an expr? not possible / neccessary to get it
    walk_stack(exprs[i]);
    bool iscomplextype = false;
    if (!isvalue)
      iscomplextype = catype_is_complex_type(oprand_stack.back()->catype);

    auto pair = pop_right_value("exprarg", !iscomplextype);
    argv.push_back(pair.first);
  }
}

static void walk_expr_gentuple(ASTNode *p) {
  // the general tuple expresssion
  ASTNode *anode = p->exprn.operands[0];

  std::vector<Value *> values;
  llvmvalue_from_exprs(anode->arglistn.exprs, anode->arglistn.argc, values, false);

  typeid_t type = inference_expr_type(p);
  CADataType *catype = catype_get_by_name(p->symtable, type);
  walk_expr_tuple_common(p, catype, values);
}

static void walk_expr(ASTNode *p);

extern STEntry *sym_get_function_entry_for_method(ASTNode *name, query_type_fn_t query_fn, void **self_value, CADataType **struct_catype, STEntry **cls_entry);
static CADataType *query_type_with_value(TStructFieldOp *sfopn, void **self_value) {
  walk_stack(sfopn->expr);
  auto pair = pop_right_value("struct", !sfopn->direct);

  if (self_value)
    *self_value = pair.first;

  return pair.second;
}

static STEntry *sym_get_function_entry_for_method_value(ASTNode *name, Value **self_value, CADataType **struct_catype, STEntry **cls_entry) {
  return sym_get_function_entry_for_method(name, query_type_with_value, (void **)self_value, struct_catype, cls_entry);
}

// the expression call may be a function call or tuple literal definition,
// because the tuple literal form is the same as function, so handle it here
static void walk_expr_call(ASTNode *p) {
  // NEXT TODO: walk generic function and cache it and call it
  //Function *walk_fn_define_full_withsym_generic(ASTNode *p, TypeImplInfo *impl_info, SymTable *symtable, bool generic);
  // 

  ASTNode *name = p->exprn.operands[0];
  ASTNode *args = p->exprn.operands[1];

  const char *fnname = nullptr;
  typeid_t fnname_id = typeid_novalue;
  STEntry *entry = nullptr;
  STEntry *cls_entry = nullptr;
  int istuple = 0;
  Value *self_value = nullptr;
  switch (name->type) {
  case TTE_Id: {
    istuple = extract_function_or_tuple(p->symtable, name->idn.i, &entry, &fnname);
    if (istuple == -1) {
      caerror(&(p->begloc), &(p->endloc), "cannot find declared function: '%s'", fnname);
      return;
    }

    if (IS_GENERIC_FUNCTION(entry->u.f.ca_func_type)) {
      // handle generic function concretion
      // 1. get generic function signature, with generic type list
      //   1.1 get parameter list of function and pickup the generic type in parameter list
      //   1.2 when is hidden expand (`add(a, b)` without generic type) check if the generic parameter list can cover the generic type provided
      //     a. when cannot cover all then report error, because cannot inference all generic types
      //     b. when can cover all then inference the generic parameter type from true argument list
      //   1.3 when is `add<i32>(a, b)` with generic type check, it should include all the generic type
      //   1.4 form generic function signature from function name and the binded generic type with order
      //   1.5 in the upper process, it will also check and determine the parameter type
      // 2. check if the function already created before
      //   2.1 when exist, create call with prepared argument list
      //   2.2 when not exist, walk the generic function, and form the llvm function, then call it
      // 3. when generic function exists recursive call it self
      //   3.1 not really recursive call: when recursive call form a different concrete function,
      //       then just like the processing of this notice to concrete it
      //     a. function already concreted, then call it directly
      //     b. function not concreted, concreted it and then call it
      //   3.2 really recursive call: when the function form a same concrete function,
      //       then it's no need to do function concretion for the called same function, so just generate call to it
      // 4. about the caches of concreted generic functions, there is a cache to store the concreted function
      //    for each generic function definition in a scope with it's symbol table, so it should be good to maintain
      //    the data structure in the one entry of generic function
    }

    check_and_determine_param_type(name, args, istuple, entry, nullptr, typeid_novalue, 0);
    break;
  }
  case TTE_Expr: {
    CADataType *catype = nullptr;
    // get struct entry from domain subparts
    entry = sym_get_function_entry_for_method_value(name, &self_value, &catype, &cls_entry);
    check_and_determine_param_type(name, args, istuple, entry, cls_entry,
                                   catype->signature, name->exprn.operands[0]->sfopn.direct);
    fnname_id = name->exprn.operands[0]->sfopn.fieldname;
    fnname = symname_get(fnname_id);
    break;
  }
  case TTE_Domain: {
    // get struct entry from domain subparts
    entry = sym_get_function_entry_for_domainfn(name, args, &cls_entry, NULL /* &fnname_id */);
    check_and_determine_param_type(name, args, istuple, entry, cls_entry, typeid_novalue, 0);
    fnname_id = domain_get_function_name(name);
    fnname = symname_get(fnname_id);
    break;
  }
  default:
    yyerror("bad function call type: %d", name->type);
    return;
  }

  Function *fn = nullptr;
  if (!istuple) { // is function
    const char *fnname_full = symname_get(entry->u.f.mangled_id);
    fn = ir1.module().getFunction(fnname_full);
    if (!fn) {
      caerror(&(p->begloc), &(p->endloc), "cannot find declared function: '%s'", fnname);
      return;
    }
  }

  if (args->type != TTE_ArgList)
    caerror(&(p->begloc), &(p->endloc), "not a argument list: '%s'", fnname);

  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

  std::vector<Value *> argv;
  
  int is_method = name->type == TTE_Expr;
  if (is_method)
    argv.push_back(self_value);

  llvmvalue_from_exprs(args->arglistn.exprs, args->arglistn.argc, argv, !istuple);

  if (istuple)
    return walk_expr_tuple(p, entry, argv);

  Type *rettype = fn->getReturnType();
  bool isvoidty = rettype->isVoidTy();

  const char *fnname_full = symname_get(entry->u.f.mangled_id);
  auto itr = function_map.find(fnname_full);
  if (itr == function_map.end()) {
    caerror(&(p->begloc), &(p->endloc), "cannot find function '%s' node", fnname);
    return;
  }
 
  CallInst *callret = ir1.builder().CreateCall(fn, argv, isvoidty ? "" : fnname);
  if ((cls_entry && (IS_GENERIC_FUNCTION(entry->u.f.ca_func_type) || entry->u.f.ca_func_type == CAFT_MethodInTrait))) {
    SymTableAssoc *assoc = runable_find_entry_assoc(cls_entry, fnname_id, -1);
    itr->second->symtable->assoc = assoc;
  }

  CADataType *retdt = catype_get_by_name(itr->second->symtable, itr->second->fndecln.ret);
  CHECK_GET_TYPE_VALUE(itr->second, retdt, itr->second->fndecln.ret);
  itr->second->symtable->assoc = nullptr;

  OperandType optype = OT_CallInst;
  Value *newv = callret;
  if (!isvoidty) {
    optype = OT_Alloc;
    newv = ir1.gen_entry_block_var(curr_fn, rettype, "calltmp", callret);
  }

  auto operands = std::make_unique<CalcOperand>(optype, newv, retdt);
  oprand_stack.push_back(std::move(operands));
}

static void walk_ret(ASTNode *p) {
  if (walk_pass == 1)
    return;

  Type *rettype = curr_fn->getReturnType();
  BasicBlock *retbb = (BasicBlock *)curr_fn_node->fndefn.retbb;

  if (p->retn.expr) {
    ASTNode *retn = p->retn.expr;
    typeid_t retid = curr_fn_node->fndefn.fn_decl->fndecln.ret;
    CADataType *retcatype = catype_get_by_name(p->symtable, retid);
    CHECK_GET_TYPE_VALUE(p, retcatype, retid);
    determine_expr_type(retn, retcatype->signature);
    if (retn->type == TTE_Literal && !retn->litn.litv.fixed_type) {
      auto itr = function_map.find(curr_fn->getName().str());
      CADataType *retdt = catype_get_by_name(p->symtable, itr->second->fndecln.ret);
      CHECK_GET_TYPE_VALUE(p, retdt, itr->second->fndecln.ret);
    }

    walk_stack(retn);
    auto pair = pop_right_value();
    Value *v = pair.first;
    if (enable_debug_info())
      diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

    // match the function return value and the literal return value
    if (rettype != v->getType()) {
      typeid_t retty = curr_fn_node->fndefn.fn_decl->fndecln.ret;
      CADataType *retdt = catype_get_by_name(p->symtable, retty);
      CHECK_GET_TYPE_VALUE(retn, retdt, retty);
      typeid_t exprtype = get_expr_type_from_tree(retn);
      caerror(&(retn->begloc), &(retn->endloc), "return value `%s` type '%s' not match function type '%s'",
	      get_node_name_or_value(retn), symname_get(exprtype), symname_get(retty));
      return;
    }

    AllocaInst *retslot = (AllocaInst *)curr_fn_node->fndefn.retslot;
    ir1.builder().CreateStore(v, retslot);
  } else {
    if (enable_debug_info())
      diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

    if (rettype != ir1.void_type()) {
      caerror(&(p->begloc), &(p->endloc), "void type function, cannot return a valued");
      return;
    }
  }

  ir1.builder().CreateBr(retbb);

  BasicBlock *bb = ir1.gen_bb("afterret", curr_fn);
  ir1.builder().SetInsertPoint(bb);

  g_with_ret_value = true;
}

static void handle_pointer_op(ASTNode *p, CADataType *dt, Value *v1, CADataType *dt2, Value *v2) {
  Value *v3 = nullptr;
  Value *z = nullptr;
  Type *type = nullptr;

  switch (p->exprn.op) {
  case '-':
    z = ir1.gen_int((int64_t)0);
    v2 = ir1.builder().CreateSExt(v2, ir1.int_type<int64_t>());
    v2 = ir1.gen_sub(z, v2, "m");
  case '+':
    type = llvmtype_from_catype(dt->pointer_layout->type);
    v3 = ir1.builder().CreateGEP(type, v1, v2, "pop");
    break;
  default:
    caerror(&(p->begloc), &(p->endloc), "pointer operation only support `+` and `-`, but find `%c`", p->exprn.op);
    return;
  }

  oprand_stack.push_back(std::make_unique<CalcOperand>(OT_Calc, v3, dt));
  if (p->exprn.expr_type == typeid_novalue)
    p->exprn.expr_type = dt->signature;

  assert(p->exprn.expr_type == dt->signature);
  return;
}

static void walk_expr_landor(CADataType *dt1, Value *v1, ASTNode *p) {
  BasicBlock *thenbb = ir1.gen_bb("thenbb");
  BasicBlock *outbb = ir1.gen_bb("outbb");
  bool isand = (p->exprn.op == LAND);

  BasicBlock *v1bb = ir1.builder().GetInsertBlock();

  ir1.builder().CreateCondBr(v1, isand ? thenbb : outbb, isand ? outbb : thenbb);

  // then block, to calculate the value of second bool expression
  curr_fn->getBasicBlockList().push_back(thenbb);
  ir1.builder().SetInsertPoint(thenbb);
  walk_stack(p->exprn.operands[1]);
  auto pair2 = pop_right_value("v2");
  Value *v2 = pair2.first;

  // because the upper walk may appended new BB, so here cannot use thenbb directly, so just get it
  BasicBlock *v2bb = ir1.builder().GetInsertBlock();

  ir1.builder().CreateBr(outbb);

  // out block, when first bool expression already meet requirement
  curr_fn->getBasicBlockList().push_back(outbb);
  ir1.builder().SetInsertPoint(outbb);

  PHINode *phiv = ir1.gen_phi(ir1.bool_type(), v2bb, v2, v1bb, v1);
  auto pnv = std::make_unique<CalcOperand>(OT_PHINode, phiv, dt1);
  oprand_stack.push_back(std::move(pnv));
}

static void walk_expr_op2(ASTNode *p) {
  walk_stack(p->exprn.operands[0]);
  auto pair1 = pop_right_value("v1");

  switch (p->exprn.op) {
  case LAND:
  case LOR:
    return walk_expr_landor(pair1.second, pair1.first, p);
  }

  walk_stack(p->exprn.operands[1]);
  auto pair2 = pop_right_value("v2");
  Value *v1 = pair1.first;
  Value *v2 = pair2.first;
  Value *v3 = nullptr;

  typeid_t typeid1 = pair1.second->signature;
  typeid_t typeid2 = pair2.second->signature;

  CADataType *dt = catype_get_by_name(p->symtable, typeid1);
  CHECK_GET_TYPE_VALUE(p, dt, typeid1);

  CADataType *dt2 = catype_get_by_name(p->symtable, typeid2);
  CHECK_GET_TYPE_VALUE(p, dt2, typeid2);

  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

  if (dt->type == POINTER && catype_is_integer(dt2->type)) {
    handle_pointer_op(p, dt, v1, dt2, v2);
    return;
  }

  if ((p->exprn.op != SHIFTL &&  p->exprn.op != SHIFTR) && !catype_check_identical_in_symtable(p->symtable, typeid1, p->symtable, typeid2)) {
    caerror(&(p->begloc), &(p->endloc), "operation have 2 different types: '%s', '%s'",
	    symname_get(typeid1), symname_get(typeid2));
    return;
  }

  switch (p->exprn.op) {
  case '+':
    v3 = ir1.gen_add(v1, v2);
    break;
  case '-':
    v3 = ir1.gen_sub(v1, v2);
    break;
  case '*':
    v3 = ir1.gen_mul(v1, v2);
    break;
  case '/':
    v3 = ir1.gen_div(v1, v2);
    break;
  case '%':
    v3 = ir1.gen_mod(v1, v2);
    break;
  case '<':
  case '>':
  case GE:
  case LE:
  case NE:
  case EQ:
    v3 = generate_cmp_op(dt->type, v1, v2, p->exprn.op);
    // BOOL for 1 bit of bool type
    dt = catype_get_primitive_by_token(BOOL);
    break;
  case BAND:
    v3 = ir1.builder().CreateAnd(v1, v2, "band");
    break;
  case BOR:
    v3 = ir1.builder().CreateOr(v1, v2, "bor");
    break;
  case BXOR:
    v3 = ir1.builder().CreateXor(v1, v2, "bxor");
    break;
  case SHIFTL:
    v2 = ir1.builder().CreateZExtOrTrunc(v2, v1->getType());
    v3 = ir1.builder().CreateShl(v1, v2, "shl");
    break;
  case SHIFTR:
    v2 = ir1.builder().CreateZExtOrTrunc(v2, v1->getType());
    if (catype_is_signed(dt->type))
      v3 = ir1.builder().CreateAShr(v1, v2, "ashr");
    else
      v3 = ir1.builder().CreateLShr(v1, v2, "lshr");
    break;
  default:
    yyerror("unknown expression operands: %d", p->exprn.op);
    break;
  }

  oprand_stack.push_back(std::make_unique<CalcOperand>(OT_Calc, v3, dt));
  if (p->exprn.expr_type == typeid_novalue)
    p->exprn.expr_type = dt->signature;

  assert(p->exprn.expr_type == dt->signature);
}

static void walk_expr_as(ASTNode *node) {
  if (walk_pass == 1)
    return;

  //CADataType *type = node->exprasn.type;
  CADataType *astype = catype_get_by_name(node->symtable, node->exprasn.type);
  CHECK_GET_TYPE_VALUE(node, astype, node->exprasn.type);

  ASTNode *exprn = node->exprasn.expr;

  if (enable_debug_info())
    diinfo->emit_location(node->endloc.row, node->endloc.col, curr_lexical_scope->discope);

  inference_expr_type(exprn);
  walk_stack(exprn);

  typeid_t stype = get_expr_type_from_tree(exprn);
  CADataType *exprcatype = catype_get_by_name(node->symtable, stype);
  CHECK_GET_TYPE_VALUE(node, exprcatype, stype);
  tokenid_t stypetok = exprcatype->type;
  
  Instruction::CastOps castopt = gen_cast_ops(exprcatype, astype);
  if (castopt == (ICO)-1) {
    caerror(&(node->begloc), &(node->endloc), "cannot convert `%s` into `%s`",
	    catype_get_type_name(exprcatype->signature),
	    catype_get_type_name(astype->type));
    return;
  }

  bool array2ptr = (exprcatype->type == ARRAY && astype->type == POINTER);
  auto calco = pop_right_operand("tmpexpr", !array2ptr);
  Value *v = calco->operand;

  if (array2ptr) {
    Type *stype = llvmtype_from_catype(astype);
    v = ir1.gen_cast_value(ICO::BitCast, v, stype, "ptrcast");
    v = ir1.gen_entry_block_var(curr_fn, stype, "tmpptr", v);
  } else {
    if (castopt != (ICO)0) {
      Type *stype = llvmtype_from_catype(astype);
      v = ir1.gen_cast_value(castopt, v, stype);
    }
  }

  OperandType optype = array2ptr ? OT_Alloc : calco->type;

  auto u = std::make_unique<CalcOperand>(optype, v, astype);
  oprand_stack.push_back(std::move(u));
}

static void walk_expr_sizeof(ASTNode *id) {
  CADataType *catype = catype_get_by_name(id->symtable, id->idn.i);
  Value *value = ir1.gen_int((uint64_t)catype->size);
  CADataType *sizetype = catype_get_primitive_by_token(U64);
  auto u = std::make_unique<CalcOperand>(OT_Const, value, sizetype);
  oprand_stack.push_back(std::move(u));
}

static void walk_expr_array(ASTNode *p) {
  inference_expr_type(p);
  ASTNode *anode = p->exprn.operands[0];
  CADataType *arraycatype = catype_get_by_name(anode->symtable, p->exprn.expr_type);
  CHECK_GET_TYPE_VALUE(anode, arraycatype, anode->exprasn.type);

  std::vector<ASTNode *> *vnodes = arrayexpr_deref(anode->anoden.aexpr);
  std::vector<Value *> values;
  Type *lefttype = nullptr;
  CalcOperand leftco;
  bool iscomplextype = catype_is_complex_type(arraycatype->array_layout->type);
  typeid_t leftsubtypeid = arraycatype->array_layout->type->signature;
  for (size_t i = 0; i < vnodes->size(); ++i) {
    ASTNode *subnode = (*vnodes)[i];
    walk_stack(subnode);
    typeid_t rightsubtypeid = get_expr_type_from_tree(subnode);
    if (leftsubtypeid != rightsubtypeid) {
      yyerror("array element type not identical with variable type: `%s` != `%s`",
	      catype_get_type_name(rightsubtypeid), catype_get_type_name(leftsubtypeid));
      return;
    }

    std::unique_ptr<CalcOperand> co = pop_right_operand("arritem", !iscomplextype);
    if (i == 0)
      leftco = *co;

    lefttype = leftco.operand->getType();
    Type *type = co->operand->getType();
    if (lefttype != type) {
      caerror(&(anode->begloc), &(anode->endloc), "array type have different element type: idx: %d, `%s` != `%s`",
	      i, catype_get_type_name(leftco.catype->signature), catype_get_type_name(co->catype->signature));
      return;
    }

    values.push_back(co->operand);
    leftco = *co;
  }

  // allocate new array and copy related elements to the array
  Type *arraytype = llvmtype_from_catype(arraycatype);
  AllocaInst *arr = ir1.gen_entry_block_var(curr_fn, arraytype);
  Value *idxv0 = ir1.gen_int(0);
  std::vector<Value *> idxv(2, idxv0);

  if (vnodes->size() != 0 && lefttype->getTypeID() == Type::PointerTyID)
    lefttype = static_cast<PointerType *>(lefttype)->getElementType();

  if (anode->anoden.aexpr.repeat_count &&
      arraycatype->array_layout->type->size == 1) {
    // condition of: let a = [x; 10000];
    if (!values.empty()) {
      // using memset when type is 1 byte length to do the job
      // Type *i8type = ir1.intptr_type<int8_t>();
      // Value *i8var = ir1.builder().CreatePointerCast(values[0], i8type);
      // TypeSize size = ir1.module().getDataLayout().getTypeAllocSize(type);
      Align align = (static_cast<AllocaInst *>(values[0]))->getAlign();

      CallInst *ci = ir1.builder().CreateMemSet(
          arr, values[0], anode->anoden.aexpr.repeat_count, align);
    }
  } else {
#if 0 // TODO: here should handle the multi-byte constant value
    if (anode->anoden.aexpr.repeat_count && !values.empty()) {
      // using memcpy the constant when type is multi-byte type
      ArrayType *constant_array = static_cast<ArrayType *>(arraytype);
      std::vector<Constant *> array_elements(anode->anoden.aexpr.repeat_count,
                                             constant_array);
      Constant *constant = ConstantArray::get(constant_array, array_elements);
    } else
#else
      if (anode->anoden.aexpr.repeat_count > 0) {
	for (size_t i = 0; i < anode->anoden.aexpr.repeat_count; ++i) {
          // get elements address of arr
          Value *idxvi = ir1.gen_int(i);
          idxv[1] = idxvi;
          Value *dest = ir1.builder().CreateGEP(arr, idxv);
          aux_copy_llvmvalue_to_store(lefttype, dest, values[0], "tmpsuba");
	}
      } else {
        for (size_t i = 0; i < values.size(); ++i) {
          // get elements address of arr
          Value *idxvi = ir1.gen_int(i);
          idxv[1] = idxvi;
          Value *dest = ir1.builder().CreateGEP(arr, idxv);
          aux_copy_llvmvalue_to_store(lefttype, dest, values[i], "tmpsuba");
        }
      }
    }
#endif

  auto u = std::make_unique<CalcOperand>(OT_Alloc, arr, arraycatype);
  oprand_stack.push_back(std::move(u));
}

static int structexpr_get_field_order(int name, CAStructField *fields, size_t size) {
  for (int i = 0; i < size; ++i) {
    if (fields[i].name == name)
      return i;
  }

  return -1;
}

static void walk_expr_struct(ASTNode *p) {
  typeid_t structid = inference_expr_type(p);
  ASTNode *snode = p->exprn.operands[0];
  CADataType *structcatype = catype_get_by_name(snode->symtable, structid);
  CHECK_GET_TYPE_VALUE(snode, structcatype, snode->exprasn.type);
  if (structcatype->type != STRUCT) {
    caerror(&(snode->begloc), &(snode->endloc), "type `%s` is not a struct type",
	    catype_get_type_name(structcatype->signature));
    return;
  }

  std::vector<void *> *vnodes = structexpr_deref(snode->snoden);
  if (structcatype->struct_layout->fieldnum != vnodes->size()) {
    caerror(&(snode->begloc), &(snode->endloc), "struct type `%s` expression field size: `%d` not equal to the struct field size: `%d`",
	    catype_get_type_name(structcatype->signature), structcatype->struct_layout->fieldnum, vnodes->size());
    return;
  }

  CAStructField *fields = structcatype->struct_layout->fields;

  // when is named field then store the field order in struct definition
  std::vector<int> fieldorder;
  std::vector<Value *> values;
  for (size_t i = 0; i < vnodes->size(); ++i) {
    ASTNode *fieldnode = nullptr;
    int exprfieldname = -1;
    void *inode = (*vnodes)[i];
    int order = i;
    if (snode->snoden.named) {
      CAStructNamed *namedexpr = static_cast<CAStructNamed *>(inode);
      order = structexpr_get_field_order(namedexpr->name, fields, vnodes->size());
      if (order == -1) {
	caerror(&(snode->begloc), &(snode->endloc), "cannot find the field name in struct `%s` for the `%d` field `%s` in expression",
		catype_get_type_name(structcatype->struct_layout->name), i, symname_get(namedexpr->name));
	return;
      }

      fieldorder.push_back(order);
      fieldnode = namedexpr->expr;
      // TODO: here may need free `namedexpr` when it never used again
      // free(namedexpr)
    } else {
      fieldnode = static_cast<ASTNode *>(inode);
    }

    walk_stack(fieldnode);
    bool iscomplextype = catype_is_complex_type(fields[order].type);
    auto pair = pop_right_value("field", !iscomplextype);
    if (pair.second->signature != fields[order].type->signature) {
      caerror(&(snode->begloc), &(snode->endloc), "the field `%d`'s type `%s` of struct expression is different from the struct definition: `%s`",
	      i, catype_get_type_name(pair.second->signature),
	      catype_get_type_name(fields[order].type->signature));
      return;
    }

    values.push_back(pair.first);
  }

  if (snode->snoden.named) {
    // rearrange values order according to the named order
    std::vector<int> copy = fieldorder;
    std::sort(copy.begin(), copy.end());
    for (int i = 1; i < copy.size(); ++i) {
      if (copy[i-1] == copy[i]) {
	caerror(&(snode->begloc), &(snode->endloc), "multiple expression specified for field `%s` in struct `%s`",
		symname_get(fields[i].type->signature), symname_get(structcatype->struct_layout->name));
	return;
      }
    }

    for (int i = 0; i < fieldorder.size(); ++i) {
      if (i == fieldorder[i])
	continue;

      Value *tmpv = values[i];
      int tmpi = fieldorder[i];

      // go home for tmpi and tmpv
      while(tmpi != i) {
	int tmp = fieldorder[tmpi];
	fieldorder[tmpi] = tmpi;

        Value *tmpvi = values[tmpi];
	values[tmpi] = tmpv;
	tmpv = tmpvi;
	tmpi = tmp;
      }
      fieldorder[tmpi] = tmpi;
      values[tmpi] = tmpv;
    }
  }

  // allocate new array and copy related elements to the array
  StructType *structype = static_cast<StructType *>(llvmtype_from_catype(structcatype));
  AllocaInst *structure = ir1.gen_entry_block_var(curr_fn, structype);
  Value *idxv0 = ir1.gen_int((int)0);
  std::vector<Value *> idxv(2, idxv0);

  for (size_t i = 0; i < values.size(); ++i) {
    // get elements address of structure
    Value *idxvi = ir1.gen_int((int)i);
    idxv[1] = idxvi;
    Value *dest = ir1.builder().CreateGEP(// structype, 
					  structure, idxv);
    Type *lefttype = structype->getStructElementType(i);
    aux_copy_llvmvalue_to_store(lefttype, dest, values[i], "field");
  }
  
  auto u = std::make_unique<CalcOperand>(OT_Alloc, structure, structcatype);
  oprand_stack.push_back(std::move(u));
}

static void walk_expr_arrayitem(ASTNode *p) {
  assert(p->exprn.noperand == 1);
  ASTNode *anode = p->exprn.operands[0];
  assert(anode->type == TTE_ArrayItemRight);

  inference_expr_type(p);
  CADataType *arrayitemcatype = catype_get_by_name(anode->symtable, p->exprn.expr_type);
  CHECK_GET_TYPE_VALUE(anode, arrayitemcatype, p->exprn.expr_type);

  Value *arrayitemvalue = extract_value_from_array(anode);
  oprand_stack.push_back(std::make_unique<CalcOperand>(OT_Alloc, arrayitemvalue, arrayitemcatype));
}

static void walk_expr_structitemop(ASTNode *p) {
  assert(p->exprn.noperand == 1);
  ASTNode *anode = p->exprn.operands[0];
  assert(anode->type == TTE_StructFieldOpRight);

  inference_expr_type(p);
  CADataType *arrayitemcatype = catype_get_by_name(anode->symtable, p->exprn.expr_type);
  CHECK_GET_TYPE_VALUE(anode, arrayitemcatype, p->exprn.expr_type);

  Value *arrayitemvalue = extract_value_from_struct(anode);
  oprand_stack.push_back(std::make_unique<CalcOperand>(OT_Alloc, arrayitemvalue, arrayitemcatype));
}

static void walk_expr_deref(ASTNode *rexpr) {
  ASTNode *expr = rexpr->exprn.operands[0];
  walk_stack(expr);
  auto pair = pop_right_value("deref", false);
  if (pair.second->type != POINTER) {
    caerror(&(rexpr->begloc), &(rexpr->endloc), " cannot deref type `%s`",
	    catype_get_type_name(pair.second->signature));
    return;
  }

  assert(pair.second->pointer_layout->dimension == 1);
  rexpr->exprn.expr_type = pair.second->pointer_layout->type->signature;

  Value *v = ir1.builder().CreateLoad(pair.first, "deref");
  OperandType ot = OT_Alloc;
  if (!v->getType()->isPointerTy())
      ot = OT_Calc;

  auto u = std::make_unique<CalcOperand>(ot, v, pair.second->pointer_layout->type);
  oprand_stack.push_back(std::move(u));
}

static void walk_expr_address(ASTNode *aexpr) {
  ASTNode *expr = aexpr->exprn.operands[0];
  walk_stack(expr);
  auto pair = pop_right_operand("addr", false);

  pair->catype = catype_make_pointer_type(pair->catype);
  pair->type = OT_Calc; // make it not loadable value
  oprand_stack.push_back(std::move(pair));
}

static void walk_expr_box(ASTNode *expr) {
  ASTNode *boxexpr = expr->exprn.operands[0];
  walk_stack(boxexpr);
  auto pair = pop_right_operand("box", false);
  expr->exprn.expr_type = pair->catype->signature;
  oprand_stack.push_back(std::move(pair));
}

static void walk_expr_range(ASTNode *expr) {
  ASTNode *range_expr = expr->exprn.operands[0];
  walk_stack(range_expr);
}

static void walk_expr(ASTNode *p) {
  if (walk_pass == 1)
    return;

  // not allow global assign value, global variable definition is not assign
  if (!curr_fn)
    return;

  switch (p->exprn.op) {
  case ARRAY:
    walk_expr_array(p);
    break;
  case STRUCT:
    walk_expr_struct(p);
    break;
  case TUPLE:
    walk_expr_gentuple(p);
    break;
  case ARRAYITEM:
    walk_expr_arrayitem(p);
    break;
  case STRUCTITEM:
    walk_expr_structitemop(p);
    break;
  case AS:
    walk_stack(p->exprn.operands[0]);
    break;
  case SIZEOF:
    walk_expr_sizeof(p->exprn.operands[0]);
    break;
  case UMINUS:
  case BNOT:
    walk_unary_expr(p);
    break;
  case FN_CALL:
    walk_expr_call(p);
    break;
  case STMT_EXPR:
    // walk the statement part
    walk_stack(p->exprn.operands[0]);

    // walk the expression part
    walk_stack(p->exprn.operands[1]);
    {
      std::unique_ptr<CalcOperand> &o = oprand_stack.back();

      // TODO: here and 
      // p->expr_type = o.type;
    }
    break;
  case IF_EXPR:
    // TODO: how to assign value of p->expr_type
    walk_expr_ife(p->exprn.operands[0]);
    break;
  case DEREF:
    walk_expr_deref(p);
    break;
  case ADDRESS:
    walk_expr_address(p);
    break;
  case BOX:
    walk_expr_box(p);
    break;
  case RANGE:
    walk_expr_range(p);
    break;
  default:
    walk_expr_op2(p);
    break;
  }
}

static int post_check_fn_proto(STEntry *prev, typeid_t fnname, ST_ArgList *currargs, typeid_t rettype) {
  ST_ArgList *prevargs = prev->u.f.arglists;

  for (int i = 0; i < prevargs->argc; ++i) {
    STEntry *preventry = sym_getsym(prevargs->symtable, prevargs->argnames[i], 0);
    STEntry *currentry = sym_getsym(currargs->symtable, currargs->argnames[i], 0);
    if (!preventry || !currentry || preventry->sym_type != Sym_Variable || currentry->sym_type != Sym_Variable) {
      caerror(&prev->sloc, NULL, "function '%s' internal error: symbol table entry error",
	      catype_get_function_name(fnname));
      return -1;
    }

    // NEXT TODO: consider how to handle the trait default implementation ASTNode tree into struct implementation
    // 1. operate the only one ASTNode entry and walk for it, the node is shared by every struct implemenation that
    // implement it.
    // 2. clone the trait fn ASTNode tree into struct and walk the struct
    varshielding_rotate_variable(&preventry->u.varshielding);
    CADataType *prevcatype = catype_get_by_name(prevargs->symtable, preventry->u.varshielding.current->datatype);
    varshielding_rotate_variable(&preventry->u.varshielding, true);

    varshielding_rotate_variable(&currentry->u.varshielding);
    CADataType *currcatype = catype_get_by_name(currargs->symtable, currentry->u.varshielding.current->datatype);
    varshielding_rotate_variable(&currentry->u.varshielding, true);

    if (prevcatype->signature != currcatype->signature) {
      caerror(&prev->sloc, NULL, "function '%s' parameter type not identical, `%s` != `%s` see: line %d, col %d.",
	      catype_get_function_name(fnname),
	      catype_get_type_name(prevcatype->signature),
	      catype_get_type_name(currcatype->signature),
	      prev->sloc.row, prev->sloc.col);
      return -1;
    }
  }

  CADataType *prevret = catype_get_by_name(prevargs->symtable, prev->u.f.rettype);
  CADataType *currret = catype_get_by_name(currargs->symtable, rettype);

  // check if function return type is the same as declared
  if (prevret->signature != currret->signature) {
    caerror(&prev->sloc, NULL, "function '%s' return type not identical, see: line %d, col %d.",
	    catype_get_function_name(fnname), prev->sloc.row, prev->sloc.col);
    return -1;
  }

  return 0;
}

template <typename Fn>
static const char *mangling_function_name_nottype(int lexical_stack_size, Fn func) {
  // handle inner function name
  std::stringstream name;
  name << MANGLED_NAME_PREFIX;

  // first pass fill prefix, second pass fill name
  bool first_pass = true;
  int i = 0;
  while (i++ < 2) {
    for (int itr = 1; itr < lexical_stack_size; ++itr) {
      auto scope = lexical_scope_stack[itr].get();
      switch (scope->lexical_type) {
      case LT_Block:
        if (first_pass) {
          name << 'L' << scope->lexical_id;
        }
        break;
      case LT_Function:
        if (first_pass) {
          name << 'F';
        } else {
          const char *local_name =
              catype_get_function_name(scope->u.function_name);
          size_t len = strlen(local_name);
          name << len << local_name;
        }
        break;
      case LT_Module:
      case LT_Struct:
        yyerror("not implemented yet: %d", scope->lexical_type);
        break;
      case LT_Global:
      default:
        yyerror("bad lexical type: %d", scope->lexical_type);
        break;
      }
    }

    // append this function
    func(name, first_pass);

    first_pass = false;
  }

  int symname = symname_check_insert(name.str().c_str());
  return symname_get(symname);
}

static int lexical_find_symtable_pos(SymTable *symtable) {
  int i = lexical_scope_stack.size();
  while (--i >= 0) {
    if (lexical_scope_stack[i]->symtable == symtable)
      return i;
  }

  return -1;
}

static const char *mangling_function_name(SymTable *symtable, typeid_t fnname, TypeImplInfo *impl_info,
					  STEntry **cls_entry, SymTable *st_type) {
  if (!impl_info) {
    if (lexical_scope_stack.size() == 1)
      return catype_get_function_name(fnname);

    auto trait_name_fn = [fnname](std::stringstream &name, bool first_pass) {
      if (first_pass) {
        name << 'F';
      } else {
        const char *local_name = catype_get_function_name(fnname);
        size_t len = strlen(local_name);
        name << len << local_name;
      }
    };
    return mangling_function_name_nottype(lexical_scope_stack.size(), trait_name_fn);
  }

  if (impl_info->class_id == -1) {
    yyerror("implemented struct not specified, for method `%s`", catype_get_function_name(fnname));
    return nullptr;
  }

  symtable = st_type ? st_type : sym_parent_or_global(symtable);

  // the symbol table which storing the struct entry
  SymTable *entry_st = nullptr;
  STEntry *entry = sym_getsym_with_symtable(symtable, impl_info->class_id, 1, &entry_st);
  if (!entry || entry->sym_type != Sym_DataType) {
    caerror(entry ? &entry->sloc : NULL, NULL, "cannot find symbol entry for type '%s'",
	    catype_get_type_name(impl_info->class_id));
    return nullptr;
  }

  if (cls_entry)
    *cls_entry = entry;

  // get the structure path in the form of `lexical_scope_stack`, means using the struct definition
  // position as the implementation path, but not the implementation path itself
  int pos = lexical_find_symtable_pos(entry_st);
  if (pos == -1) {
    caerror(entry ? &entry->sloc : NULL, NULL, "cannot find struct '%s' lexical position",
	    catype_get_type_name(impl_info->class_id));
    return nullptr;
  }

  auto trait_name_fn = [fnname, impl_info](std::stringstream &name, bool first_pass) {
    if (first_pass) {
      // for implementing struct mangling name with `struct + function`
      // for implementing trait for struct mangling name with `trait + struct + function`
      if (impl_info->trait_id != -1) {
	name << 'T';
      }
      name << "SF";
    } else {
      // NEXT TODO: implement trait mangling considering traits path, trait has it's own path
      if (impl_info->trait_id != -1) {
        const char *trait_name = catype_get_type_name(impl_info->trait_id);
        size_t len = strlen(trait_name);
        name << len << trait_name;
      }

      const char *struct_name = catype_get_type_name(impl_info->class_id);
      size_t len = strlen(struct_name);
      name << len << struct_name;

      // the local_name here is in the struct impl form: AAA::func or Struct1::<Trait1>::func1
      const char *local_name = catype_struct_impl_id_to_function_name_str(fnname);
      len = strlen(local_name);
      name << len << local_name;
    }
  };

  return mangling_function_name_nottype(pos + 1, trait_name_fn);
}

static Function *walk_fn_declare_full_withsym(ASTNode *p, TypeImplInfo *impl_info, SymTable *st_type) {
  STEntry *cls_entry = nullptr;
  STEntry **cls_entry_out = impl_info ? &cls_entry : nullptr;

  // here using symbol table of class implementation
  const char *fnname_full = mangling_function_name(p->symtable, p->fndecln.name, impl_info, cls_entry_out, st_type);

  // if impl_info is not null then cls_entry must cannot be null
  assert(impl_info == nullptr || cls_entry != nullptr);
  int fnname_full_id = symname_check_insert(fnname_full);

  SymTable *symtable = sym_parent_or_global(p->symtable);
  STEntry *preventry = sym_getsym(symtable, p->fndecln.name, 0);

  auto fitr = g_function_post_check_map.find(fnname_full_id);
  if (fitr != g_function_post_check_map.end()) {
    // check global redeclared function parameter, some function declared
    // multiple times in source code, check if they have same argument here
    post_check_fn_proto(preventry, p->fndecln.name, &p->fndecln.args,
                        p->fndecln.ret);
  } else {
    g_function_post_check_map[fnname_full_id] = static_cast<void *>(p);
  }

  Function *fn = ir1.module().getFunction(fnname_full);
  auto itr = function_map.find(fnname_full);
  if (itr != function_map.end()) {
    if (!fn) {
      // when consider name, the function map set belongs to ir1 module set
      yyerror("very strange, function must exists in the module");
      return nullptr;
    }

    return fn;
  }

  std::vector<const char *> param_names;
  std::vector<Type *> params;
  for (int i = 0; i < p->fndecln.args.argc; ++i) {
    int argname = p->fndecln.args.argnames[i];
    param_names.push_back(symname_get(argname));

    STEntry *entry = sym_getsym(p->symtable, argname, 0);
    if (!entry) {
      yyerror("cannot get parameter for %s\n", symname_get(argname));
      return NULL;
    }

    if (entry->sym_type != SymType::Sym_Variable) {
      yyerror("parameter is not variable type: %d", entry->sym_type);
      return nullptr;
    }

#if 0
    SymTable *st = p->symtable;
    typeid_t typeid_st = entry->u.varshielding.current->datatype;
    if (i == 0 && impl_info) {
      // for the first parameter
      typeid_t trait_self_ptr_id = catype_trait_self_ptr_id();
      typeid_t trait_type = entry->u.varshielding.current->datatype;
      if (trait_type == trait_self_ptr_id) {
	// it is a *Self skeleton parameter, here should replace it with the real type
	typeid_st = sym_form_pointer_id(impl_info->class_id);
	entry->u.varshielding.current->datatype = typeid_st;
	if (st_type)
	  st = st_type;
      }
    }

    CADataType *dt = catype_get_by_name(st, typeid_st);
    CHECK_GET_TYPE_VALUE(p, dt, entry->u.varshielding.current->datatype);
#endif

    varshielding_rotate_variable(&entry->u.varshielding);
    CADataType *dt = catype_get_by_name(p->symtable, entry->u.varshielding.current->datatype);
    CHECK_GET_TYPE_VALUE(p, dt, entry->u.varshielding.current->datatype);
    varshielding_rotate_variable(&entry->u.varshielding, true);

    Type *type = llvmtype_from_catype(dt);
    params.push_back(type);
  }

  CADataType *retdt = catype_get_by_name(p->symtable, p->fndecln.ret);
  CHECK_GET_TYPE_VALUE(p, retdt, p->fndecln.ret);
  Type *rettype = llvmtype_from_catype(retdt);
  fn = ir1.gen_extern_fn(rettype, fnname_full, params, &param_names, !!p->fndecln.args.contain_varg);
  function_map.insert(std::make_pair(fnname_full, p));
  fn->setCallingConv(CallingConv::C);

  if (walk_pass == 1) {
    if (cls_entry) {
      typeid_t name = catype_struct_impl_id_to_function_name(p->fndecln.name);
      if (st_type) {
	// for trait copied default method of generic method, then copy an entry
	typeid_t fnname = sym_form_method_id(name, impl_info->class_id, impl_info->trait_id);

	STEntry *preventry_copyed = sym_check_insert(st_type, fnname, Sym_FnDef);
        preventry_copyed->u.f.arglists = (ST_ArgList *)malloc(sizeof(ST_ArgList));
        *preventry_copyed->u.f.arglists = *preventry->u.f.arglists;
	preventry_copyed->u.f.ca_func_type = preventry->u.f.ca_func_type;
	preventry_copyed->u.f.generic_types = preventry->u.f.generic_types;
	preventry_copyed->u.f.rettype = preventry->u.f.rettype;
	preventry_copyed->u.f.mangled_id = fnname_full_id;
	preventry = preventry_copyed;
      }

      runable_add_entry(impl_info, cls_entry, name, fnname_full_id, preventry);
    }

    preventry->u.f.mangled_id = fnname_full_id;
  }

  //AttrListPtr func_printf_PAL;
  //fn->setAttributes(func_printf_PAL);

  return fn;
}

static Function *walk_fn_declare_full(ASTNode *p, TypeImplInfo *impl_info) {
  return walk_fn_declare_full_withsym(p, impl_info, nullptr);
}

static void walk_fn_declare(ASTNode *p) {
  if (walk_pass == 1)
    walk_fn_declare_full(p, nullptr);
}

static void generate_final_return(ASTNode *p) {
  // should check if the function returned a value instead of append a return value always
  CADataType *retdt = catype_get_by_name(p->symtable, p->fndefn.fn_decl->fndecln.ret);
  CHECK_GET_TYPE_VALUE(p, retdt, p->fndefn.fn_decl->fndecln.ret);

  if (retdt->type == VOID) {
    ir1.builder().CreateRetVoid();
    return;
  }

  if (g_with_ret_value) {
    Value *v = ir1.builder().CreateLoad((Value *)p->fndefn.retslot, "retret");
    ir1.builder().CreateRet(v);
    return;
  }

  if (!enable_emit_main()) {
    caerror(&p->begloc, &p->endloc, "There is no return value in function `%s`",
	    catype_get_function_name(p->fndefn.fn_decl->fndecln.name));
    return;
  }

  Value *retv = create_default_integer_value(retdt->type);
  ir1.builder().CreateRet(retv);
}

static Function *walk_fn_define_full_withsym_generic(ASTNode *p, TypeImplInfo *impl_info, SymTable *symtable, bool generic) {
  if (p->fndefn.fn_decl->fndecln.generic_types && !generic)
    return nullptr;

  g_with_ret_value = false;
  Function *fn = walk_fn_declare_full_withsym(p->fndefn.fn_decl, impl_info, symtable);
  if (walk_pass == 1) {
    walk_stack(p->fndefn.stmts);
    return fn;
  }

  if (symtable)
    generic_type_stack.push_back(std::make_pair(p->symtable->assoc, generic_type_var_set_t()));
  else
    generic_type_stack.push_back(std::make_pair(nullptr, generic_type_var_set_t()));

  if (p->fndefn.fn_decl->fndecln.args.argc != fn->arg_size())
    yyerror("argument number not identical with definition (%d != %d)",
	    p->fndefn.fn_decl->fndecln.args.argc, fn->arg_size());

  curr_fn_stack.push_back(CurrFnInfo(curr_fn, curr_fn_node));
  curr_fn_node = p;
  curr_fn = fn;

  // because support inner function, so here save the old BB may be in previous function definition
  // and restore it after this function is processed over
  BasicBlock *saved_insert_bb = ir1.builder().GetInsertBlock();

  BasicBlock *retbb = ir1.gen_bb("ret");
  p->fndefn.retbb = (void *)retbb;

  BasicBlock *bb = ir1.gen_bb("entry", fn);
  ir1.builder().SetInsertPoint(bb);

  // insert here debugging information
  init_fn_param_info(fn, p->fndefn.fn_decl->fndecln.args, p->symtable, p->begloc.row);

  CADataType *retdt = catype_get_by_name(p->symtable, p->fndefn.fn_decl->fndecln.ret);
  CHECK_GET_TYPE_VALUE(p, retdt, p->fndefn.fn_decl->fndecln.ret);
  if (retdt->type != VOID) {
    p->fndefn.retslot = (void *)ir1.gen_entry_block_var(curr_fn, fn->getReturnType(), "retslot");
  } else {
    p->fndefn.retslot = nullptr;
  }

  if (enable_debug_info())
    diinfo->emit_location(p->begloc.row, p->begloc.col, curr_lexical_scope->discope);

  walk_stack(p->fndefn.stmts);

  // the return statement is not in source code, but be added by the compiler
  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col);

  ir1.builder().CreateBr(retbb);
  fn->getBasicBlockList().push_back(retbb);
  ir1.builder().SetInsertPoint(retbb);
  generate_final_return(p);

  if (enable_debug_info()) {
    // finalize the debug info for only the function
    diinfo->dibuilder->finalizeSubprogram((DISubprogram *)diinfo->lexical_blocks.back());
    diinfo->lexical_blocks.pop_back();

    //diinfo->dibuilder->finalize();
  }

  std::string verify_message;
  llvm::raw_string_ostream rso(verify_message);
  if (llvm::verifyFunction(*fn, &rso)) {
    fprintf(stderr, "\nfrom line %d, to line %d: function verify failed: %s\n",
	    p->begloc.row, p->endloc.row, verify_message.c_str());
  }

  if (enable_emit_main()) {
    ir1.builder().SetInsertPoint(main_bb);
    curr_fn_node = main_fn_node;
    curr_fn = main_fn;
  } else {
    ir1.builder().SetInsertPoint(saved_insert_bb);
    curr_fn_node = curr_fn_stack.back().fn_node;
    curr_fn = curr_fn_stack.back().llvm_fn;
    curr_fn_stack.pop_back();
  }

  if (symtable) {
    generic_type_handle_replace(generic_type_stack.back());
  }

  generic_type_stack.pop_back();

  return fn;
}

static Function *walk_fn_define_full_withsym(ASTNode *p, TypeImplInfo *impl_info, SymTable *symtable) {
  return walk_fn_define_full_withsym_generic(p, impl_info, symtable, false);
}

static Function *walk_fn_define_full(ASTNode *p, TypeImplInfo *impl_info) {
  return walk_fn_define_full_withsym(p, impl_info, nullptr);
}

static void walk_fn_define(ASTNode * p) {
  walk_fn_define_full(p, nullptr);
}

static void set_difference(const std::map<typeid_t, struct ASTNode *> &s1,
                           const std::set<typeid_t> &s2,
                           std::set<typeid_t> &diff) {
  diff.clear();
  auto iter1 = s1.begin();
  auto iter2 = s2.begin();
  while (iter1 != s1.end() && iter2 != s2.end()) {
    if (iter1->first < *iter2) {
      diff.insert(iter1->first);
      ++iter1;
    } else if (iter1->first == *iter2) {
      ++iter1;
      ++iter2;
    } else {
      ++iter2;
    }
  }

  while (iter1 != s1.end()) {
    diff.insert(iter1->first);
    ++iter1;
  }
}

static bool compare_self_type(CADataType *cls_catype, ASTNode *traitfnn, ASTNode *implfnn, bool &self_param_checked) {
  ST_ArgList &trait_args = traitfnn->fndecln.args;
  ST_ArgList &impl_args = implfnn->fndecln.args;

  self_param_checked = false;
  if (trait_args.argc > 0) {
    if (!strcmp(symname_get(trait_args.argnames[0]), OSELF) ||
	!strcmp(symname_get(impl_args.argnames[0]), OSELF)) {
      // check self name
      if (trait_args.argnames[0] != impl_args.argnames[0]) {
        caerror_noexit(&implfnn->begloc, &implfnn->endloc,
                       "method `%s` has an incompatible type for trait\n"
                       "first parameter name is `%s`\n\n"
                       "note: type in trait",
                       catype_get_type_name(traitfnn->fndecln.name),
                       symname_get(impl_args.argnames[0]));
        caerror_noexit(&traitfnn->begloc, &traitfnn->endloc,
                       "first parameter name is `%s`",
                       symname_get(trait_args.argnames[0]));
        return false;
      }

      // check trait self type
      typeid_t trait_self_ptr_id = catype_trait_self_ptr_id();
      STEntry *trait_entry = sym_getsym(trait_args.symtable, trait_args.argnames[0], 0);
      typeid_t trait_type = trait_entry->u.varshielding.current->datatype;
      if (trait_type != trait_self_ptr_id) {
        caerror_noexit(&traitfnn->begloc, &traitfnn->endloc,
		       "invalid `self` parameter type: %s\n"
                       "note: type of `self` must be `Self`\n"
		       "help: consider changing to `self`",
		       catype_get_type_name(trait_type));
	return false;
      }

      // check impl self type, the self type must be the struct pointer type
      STEntry *impl_entry = sym_getsym(impl_args.symtable, impl_args.argnames[0], 0);
      typeid_t impl_type = impl_entry->u.varshielding.current->datatype;
      CADataType *impl_catype = catype_get_by_name(impl_args.symtable, impl_type);

      if (impl_catype->type != POINTER ||
          impl_catype->pointer_layout->type->signature != cls_catype->signature) {
        caerror_noexit(&implfnn->begloc, &implfnn->endloc,
                       "invalid `self` parameter type: %s\n"
                       "note: type of `self` must be `Self`\n"
                       "help: consider changing to `self`",
                       catype_get_type_name(impl_catype->signature));
        return false;
      }

      self_param_checked = true;
    }
  }

  return true;
}

static bool compare_trait_and_impl_args(CADataType *cls_catype, ASTNode *traitfnn, ASTNode *implfnn) {
  ST_ArgList &trait_args = traitfnn->fndecln.args;
  ST_ArgList &impl_args = implfnn->fndecln.args;

  // check parameter numbers
  if (trait_args.argc != impl_args.argc) {
    caerror_noexit(&implfnn->begloc, &implfnn->endloc,
		   "method `%s` has an incompatible argument number `%d` with trait `%d`",
		   catype_get_type_name(implfnn->fndecln.name), impl_args.argc, trait_args.argc);
    caerror_noexit(&traitfnn->begloc, &traitfnn->endloc, "");
    return false;
  }

  // check self parameter
  bool self_param_checked = false;
  if (!compare_self_type(cls_catype, traitfnn, implfnn, self_param_checked)) {
    return false;
  }

  // check parameters
  for (int i = self_param_checked ? 1 : 0; i < trait_args.argc; ++i) {
    STEntry *trait_entry = sym_getsym(trait_args.symtable, trait_args.argnames[i], 0);
    typeid_t trait_type = trait_entry->u.varshielding.current->datatype;
    CADataType *trait_catype = catype_get_by_name(trait_args.symtable, trait_type);

    STEntry *impl_entry = sym_getsym(impl_args.symtable, impl_args.argnames[i], 0);
    typeid_t impl_type = impl_entry->u.varshielding.current->datatype;
    CADataType *impl_catype = catype_get_by_name(impl_args.symtable, impl_type);
    if (trait_catype->signature != impl_catype->signature) {
      caerror_noexit(&implfnn->begloc, &implfnn->endloc,
                     "method `%s` has an incompatible type for trait\n"
		     "`%s` is of type `%s`\n\n"
                     "note: type in trait",
                     catype_get_type_name(traitfnn->fndecln.name),
		     symname_get(impl_args.argnames[i]),
                     catype_get_type_name(impl_catype->signature)
		     );
      caerror_noexit(&traitfnn->begloc, &traitfnn->endloc, "`%s` is of type `%s`",
		     symname_get(trait_args.argnames[i]),
                     catype_get_type_name(trait_catype->signature)
		     );
      return false;
    }
  }

  return true;
}

static bool compare_trait_and_impl_signature(CADataType *cls_catype, ASTNode *traitfnn, ASTNode *implfnn, SymTableAssoc *assoc) {
  assert(traitfnn->type == TTE_FnDecl && implfnn->type == TTE_FnDecl);

  // check return code
  traitfnn->symtable->assoc = assoc;
  CADataType *trait_ret_catype = catype_get_by_name(traitfnn->symtable, traitfnn->fndecln.ret);
  CADataType *impl_ret_catype = catype_get_by_name(implfnn->symtable, implfnn->fndecln.ret);
  if (trait_ret_catype->signature != impl_ret_catype->signature) {
    caerror_noexit(&implfnn->begloc, &implfnn->endloc, "method `%s` has an incompatible type for trait",
		   "\n\nnote: type in trait",
		   catype_get_type_name(traitfnn->fndecln.name));
    caerror_noexit(&traitfnn->begloc, &traitfnn->endloc, "");
    return false;
  }

  typeid_t purename_trait = catype_struct_impl_id_to_function_name(traitfnn->fndecln.name);
  typeid_t purename_impl = catype_struct_impl_id_to_function_name(implfnn->fndecln.name);
  if (purename_trait != purename_impl) {
    caerror_noexit(&implfnn->begloc, &implfnn->endloc, "strange: impl name `%s` not equal to trait name `%s`",
		   symname_get(traitfnn->fndecln.name), symname_get(implfnn->fndecln.name));
    return false;
  }

  bool ok = compare_trait_and_impl_args(cls_catype, traitfnn, implfnn);
  traitfnn->symtable->assoc = nullptr;
  return ok;
}

static void check_trait_impl_match(ASTNode *node, std::vector<std::pair<typeid_t, ASTNode *>> &use_default_impls, SymTableAssoc *assoc) {
  int trait_id = node->fndefn_impl.impl_info.trait_id;
  STEntry *entry = sym_getsym(node->symtable, trait_id, 1);
  if (!entry) {
    caerror(&node->begloc, &node->endloc, "cannot find trait `%s` definition",
	    catype_get_type_name(trait_id));
    return;
  }

  // TODO: remove 3 lines, seems useless
  ASTNode *trait_defn = entry->u.trait_def.node;
  assert(trait_defn->traitfnlistn.trait_id == trait_id);
  assert(trait_defn->type == TTE_TraitFn);

  // each node in trait defs is a function declaration or default definition
  TraitNodeInfo *trait_info = (TraitNodeInfo *)entry->u.trait_def.trait_entry;

  CADataType *cls_catype = catype_get_by_name(node->symtable, node->fndefn_impl.impl_info.class_id);

  // the trait function definition calcaulted firstly in sym_create_trait_defs_entry
  std::set<typeid_t> fnids_impls;
  for (int i = 0; i < node->fndefn_impl.count; ++i) {
    // each impl node is a function definition
    ASTNode *impl_fn_node = (ASTNode *)vec_at(node->fndefn_impl.data, i);
    ASTNode *decln = impl_fn_node->fndefn.fn_decl;
    typeid_t purename = catype_struct_impl_id_to_function_name(decln->fndecln.name);
    auto iter = trait_info->fnnodes.find(purename);
    if (iter == trait_info->fnnodes.end()) {
      caerror(&impl_fn_node->begloc, &impl_fn_node->endloc, "method `%s` is not a member of trait `%s`",
	      catype_get_type_name(decln->fndecln.name), catype_get_type_name(impl_fn_node->fndefn.fn_decl->fndecln.name));
      return;
    }

    if (!compare_trait_and_impl_signature(cls_catype, iter->second->fndefn.fn_decl, decln, assoc)) {
      caerror(&impl_fn_node->begloc, &impl_fn_node->endloc, "method `%s` signature not match trait `%s`",
	      catype_get_type_name(decln->fndecln.name), catype_get_type_name(impl_fn_node->fndefn.fn_decl->fndecln.name));
      return;
    }

    fnids_impls.insert(purename);
  }

  // check not implemented method for trait
  std::set<typeid_t> fnids_not_impls;
  set_difference(trait_info->fnnodes, fnids_impls, fnids_not_impls);

  std::vector<typeid_t> missing_impls;
  if (!fnids_not_impls.empty()) {
    std::stringstream ss;
    for (auto iter = fnids_not_impls.begin(); iter != fnids_not_impls.end(); ++iter) {
      auto iter_find = trait_info->ids_no_def.find(*iter);
      if (iter_find != trait_info->ids_no_def.end()) {
	missing_impls.push_back(*iter_find);
	const char *local_name = symname_get(*iter_find);
	ss << '`' << local_name << '`' << ", ";
      }
    }

    if (!missing_impls.empty()) {
      std::string s = ss.str();
      s.pop_back();
      s.pop_back();
      caerror(&node->begloc, &node->endloc, "not all trait items implemented, missing: %s", s.c_str());
      return;
    }
  }

  for (auto iter = fnids_not_impls.begin(); iter != fnids_not_impls.end(); ++iter) {
    use_default_impls.push_back(std::make_pair(*iter, trait_info->fnnodes[*iter]));
  }
}

static void walk_fn_define_impl(ASTNode *node) {
  // pair.first: the method name
  // pair.second: the method implementation
  SymTableAssoc *assoc = nullptr;

  // in order to not copy the entire ASTNode tree for trait method definition, so here just
  // create temporary symbol table for storing `Self` type alias or generic type alias
  // information (used in generic function)
  int self_id = symname_check_insert(CSELF);
  SymTable *symtable = push_new_symtable_with_parent(node->symtable);
  STEntry *entry = make_type_def_entry(self_id, node->fndefn_impl.impl_info.class_id,
				       symtable, &node->begloc, &node->endloc);
  assoc = new_SymTableAssoc(STAT_Generic, symtable);
  sym_assoc_add_item(assoc, entry->sym_name); // sym_name: t:Self, for Self stub type

  // tell it to find struct entry from association table, because current template may have no such type information
  sym_assoc_add_item(assoc, node->fndefn_impl.impl_info.class_id);

  std::vector<std::pair<typeid_t, ASTNode *>> use_default_impls;
  if (node->fndefn_impl.impl_info.trait_id != -1) {
    check_trait_impl_match(node, use_default_impls, assoc);
  }

  void *handle = node->fndefn_impl.data;

  for (int i = 0; i < node->fndefn_impl.count; ++i) {
    ASTNode *node_impl = static_cast<ASTNode *>(vec_at(handle, i));
    walk_fn_define_full(node_impl, &node->fndefn_impl.impl_info);
  }

  // copy trait default method implementation and walk
  // pick up the method with *Self as the first parameter
  for (auto pair : use_default_impls) {
    pair.second->symtable->assoc = assoc;

    auto impl_info = &node->fndefn_impl.impl_info;
    STEntry *cls_entry = sym_getsym_with_symtable(symtable, impl_info->class_id, 1, nullptr);
    runable_add_entry_assoc(impl_info, cls_entry, pair.first, assoc);

    // NEXT TODO: need clone ASTNode here and pass symtable? or steal the sym
    walk_fn_define_full_withsym(pair.second, impl_info, symtable);
    pair.second->symtable->assoc = nullptr;
    //free_symtable(symtable);
  }
}

static void walk_trait_fnlist(ASTNode *node) {
  // TODO: remove the useless code
  for (int i = 0; i < node->traitfnlistn.count; ++i) {
    ASTNode *traitfn = (ASTNode *)vec_at(node->traitfnlistn.data, i);
  }
}

static void walk_struct(ASTNode *node) {
  if (walk_pass == 1)
    return;

  // only check struct definition, but not generate Type object

  STEntry *entry = node->entry;
  typeid_t id = entry->u.datatype.id;

  CADataType *dt = catype_get_by_name(node->symtable, id);
  CHECK_GET_TYPE_VALUE(node, dt, id);
}

static void walk_typedef(ASTNode *node) {
  if (walk_pass == 1)
    return;

  CADataType *dt = catype_get_by_name(node->symtable, node->typedefn.newtype);
  if (!dt) {
    caerror(&(node->begloc), &(node->endloc), "get type (or unwind type) `%s` failed",
	    symname_get(node->typedefn.newtype));
  }
}

static void walk_vardefvalue(ASTNode *node) {}
static void walk_arraydef(ASTNode *node) {}
static void walk_derefleft(ASTNode *node) {}
static void walk_arrayitemleft(ASTNode *node) {}
static void walk_arrayitemright(ASTNode *node) {}
static void walk_structfieldopleft(ASTNode *node) {}
static void walk_structfieldopright(ASTNode *node) {}
static void walk_structexpr(ASTNode *node) {}

static void walk_lexical_body(ASTNode *node) {
  auto lscope = std::make_unique<LexicalScope>();
  LexicalScope *parentscope = curr_lexical_scope; // also = lexical_scope_stack.back().get();
  if (walk_pass > 1 && enable_debug_info()) {
    if (node->lnoden.fnbuddy) {
      // when the scope have a buddy function scope then use the function scope as the scope but not lexical scope
      auto itr = fn_debug_map.find(curr_fn);
      assert(itr != fn_debug_map.end());

      lscope->discope = itr->second->disp;
    } else {
      // when the scope is not under a function scope then create lexical scope 
      lscope->discope = diinfo->dibuilder->createLexicalBlock(parentscope->discope, diunit, node->begloc.row, node->begloc.col);
    }
  }

  curr_lexical_scope = lscope.get();
  curr_lexical_scope->symtable = node->symtable;
  curr_lexical_scope->lexical_id = ++curr_lexical_count;
  if (node->lnoden.fnbuddy != nullptr) {
    curr_lexical_scope->lexical_type = LT_Function;
    curr_lexical_scope->u.function_name = node->lnoden.fnbuddy->fndefn.fn_decl->fndecln.name;
  } else {
    curr_lexical_scope->lexical_type = LT_Block;
  }

  lexical_scope_stack.push_back(std::move(lscope));

  walk_stack(node->lnoden.stmts);

  // emit the location here, to avoid upper inner function definition mess the debug info
  if (enable_debug_info())
    diinfo->emit_location(node->begloc.row, node->begloc.col, curr_lexical_scope->discope);

  lscope = std::move(lexical_scope_stack.back());
  lexical_scope_stack.pop_back();
  curr_lexical_scope = parentscope;

  // TODO: extend here: coping with auto variable release operation when the scope `lscope` end up
}

typedef void (*walk_fn_t)(ASTNode *p);
static walk_fn_t walk_fn_array[TTE_Num] = {
  (walk_fn_t)walk_empty,
  (walk_fn_t)walk_literal,
  (walk_fn_t)walk_id,
  (walk_fn_t)walk_label,
  (walk_fn_t)walk_label_goto,
  (walk_fn_t)walk_expr,
  (walk_fn_t)walk_fn_declare,
  (walk_fn_t)walk_fn_define,
  (walk_fn_t)walk_while,
  (walk_fn_t)walk_if,
  (walk_fn_t)walk_expr_as,
  (walk_fn_t)walk_struct,
  (walk_fn_t)walk_dbgprint,
  (walk_fn_t)walk_dbgprinttype,
  (walk_fn_t)walk_ret,
  (walk_fn_t)walk_assign,
  (walk_fn_t)walk_empty,
  (walk_fn_t)walk_stmtlist,
  (walk_fn_t)walk_typedef,
  (walk_fn_t)walk_vardefvalue,
  (walk_fn_t)walk_arraydef,
  (walk_fn_t)walk_derefleft,
  (walk_fn_t)walk_arrayitemleft,
  (walk_fn_t)walk_arrayitemright,
  (walk_fn_t)walk_structfieldopleft,
  (walk_fn_t)walk_structfieldopright,
  (walk_fn_t)walk_structexpr,
  (walk_fn_t)walk_lexical_body,
  (walk_fn_t)walk_loop,
  (walk_fn_t)walk_break,
  (walk_fn_t)walk_continue,
  (walk_fn_t)walk_for,
  (walk_fn_t)walk_box,
  (walk_fn_t)walk_drop,
  (walk_fn_t)walk_letbind,
  (walk_fn_t)walk_range,
  (walk_fn_t)walk_fn_define_impl,
  (walk_fn_t)walk_empty,
  (walk_fn_t)walk_trait_fnlist,
};

static int walk_stack(ASTNode *p) {
  if (!p)
    return 0;

  (*walk_fn_array[p->type])(p);
  return 0;
}

static void do_optimize_pass() {
  switch (genv.opt_level) {
  case OL_O1:
    {
      Function *fn = ir1.module().getFunction("main");
      ir1.fpm().run(*fn);
      break;
    }
  case OL_O2:
    {
      Function *fn = ir1.module().getFunction("main");
      ir1.fpm().run(*fn);
      ir1.pm().run(ir1.module());
      break;
    }
  case OL_O3:
    break;
  case OL_NONE:
  default:
    break;
  }
}

static int llvm_codegen_begin(RootTree *tree) {
  // mock ASTNode for generated main function
  curr_fn = nullptr;
  auto lscope = std::make_unique<LexicalScope>();
  root_lexical_scope = lscope.get();
  curr_lexical_scope = root_lexical_scope;
  curr_lexical_scope->symtable = &g_root_symtable;
  lexical_scope_stack.push_back(std::move(lscope));

  if (enable_debug_info()) {
    diunit = diinfo->create_difile();
    curr_lexical_scope->discope = diunit;
  }

  if (enable_emit_main()) {
    main_fn = ir1.gen_function(ir1.int_type<int>(), "main", std::vector<Type *>());
    main_bb = ir1.gen_bb("entry", main_fn);
    main_fn_node->fndefn.retbb = (void *)ir1.gen_bb("ret");

    curr_fn_node = main_fn_node;
    curr_fn = main_fn;
    ir1.builder().SetInsertPoint(main_bb);
    ST_ArgList arglist;
    arglist.argc = 0;

    init_fn_param_info(main_fn, arglist, tree->root_symtable, tree->begloc_main.row);
    main_fn_node->fndefn.retslot =
      (void *)ir1.gen_entry_block_var(curr_fn, main_fn->getReturnType(), "retslot", ir1.gen_int<int>(0));
  }

  initialize_inner_functions();

  return 0;
}

static int llvm_codegen_ll(const char *output = nullptr) {
  // run pass
  do_optimize_pass();

  if (output && output[0]) {
    std::error_code ec;

#if LLVM_VERSION > 12
    raw_fd_ostream os(output, ec, llvm::sys::fs::OF_None);
#else
    raw_fd_ostream os(output, ec, llvm::sys::fs::F_None);
#endif

    if (ec) {
      errs() << "could not open file: " << output
	     << ", error code: " << ec.message();
      return -1;
    }

    ir1.module().print(os, nullptr);
  } else {
    ir1.module().print(outs(), nullptr);
  }
  return 0;
}

static CodeGenOpt::Level to_llvm_codegenopt(Optimize_Level level) {
  return (CodeGenOpt::Level)level;
}

static int llvm_codegen_native(CodeGenFileType type, const char *output = nullptr) {
  // x86_64-pc-linux-gnu (clang --version)
  std::string target_triple = llvm::sys::getDefaultTargetTriple();
  std::string error;
  const Target *target = llvm::TargetRegistry::lookupTarget(target_triple, error);
  if (!target) {
    llvm::errs() << error;
    return -1;
  }

  auto cpu = "generic";
  auto features = "";
  llvm::TargetOptions opt;
  auto rm = llvm::Optional<Reloc::Model>();
  Optional<CodeModel::Model> cm = None;
  CodeGenOpt::Level ol = to_llvm_codegenopt(genv.opt_level);
  bool jit = false;
  TargetMachine *target_machine =
    target->createTargetMachine(target_triple, cpu, features, opt, rm, cm, ol, jit);
  ir1.module().setDataLayout(target_machine->createDataLayout());
  ir1.module().setTargetTriple(target_triple);

  // run pass
  do_optimize_pass();

  legacy::PassManager pass;
  auto filetype = type; // CGFT_ObjectFile; CGFT_AssemblyFile;  CGFT_Null;

  if (output && output[0]) {
    std::error_code ec;

#if LLVM_VERSION > 12
    raw_fd_ostream os(output, ec, llvm::sys::fs::OF_None);
#else
    raw_fd_ostream os(output, ec, llvm::sys::fs::F_None);
#endif

    if (ec) {
      errs() << "could not open file: " << output
	     << ", error code: " << ec.message();
      return -1;
    }

    target_machine->addPassesToEmitFile(pass, os, nullptr, filetype);
    pass.run(ir1.module());
    os.flush();
  } else {
    target_machine->addPassesToEmitFile(pass, outs(), nullptr, filetype);
    pass.run(ir1.module());
    outs().flush();
  }

  return 0;
}

static int llvm_codegen_jit(const char *output = nullptr) {
  do_optimize_pass();

  ir1.module().setDataLayout(jit1->get_datalayout());
  auto rt = jit1->get_main_jitdl().createResourceTracker();
  auto tsm = orc::ThreadSafeModule(ir1.move_module(), ir1.move_ctx());
  exit_on_error(jit1->add_module(std::move(tsm), rt));
  auto func_symbol = exit_on_error(jit1->find("main"));
  int (*func)() = (int (*)())(intptr_t)func_symbol.getAddress();

  int saved_stdout = STDOUT_FILENO;
  int saved_stderr = STDERR_FILENO;
  int tofile = output && output[0];

  if (tofile) {
    // one of the O_RDONLY O_WRONLY O_RDWR must be provided in the flag
    int fd = open(output, O_CREAT | O_TRUNC | O_WRONLY | O_APPEND, 0666);
    saved_stdout = dup(STDOUT_FILENO);
    saved_stderr = dup(STDERR_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    close(fd);
  }

  printf("\nreturn value: %d\n", func());
  fflush(stdout);

  if (tofile) {
    dup2(saved_stdout, STDOUT_FILENO);
    dup2(saved_stderr, STDERR_FILENO);
    close(saved_stdout);
    close(saved_stderr);
  }

  exit_on_error(rt->remove());
  return 0;
}

static const char *make_native_linker_command(const char *input, const char *output) {
  static char command[1024];

  // sprintf(command, "clang %s -o %s", input, output);

  //sprintf(command, "ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o build/CMakeFiles/ca.dir/ca_runtime.c.o %s -o %s -lc", input, output);
  const char *cruntime = std::getenv("CA_RUNTIME_LIBPATH");
  if (!cruntime)
    cruntime = "cruntime";

  sprintf(command, "ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 %s/*.o %s -o %s -lc -lgc", cruntime, input, output);

  return command;
}

static int llvm_codegen_end() {
  int ret = 0;

  // TODO: may need not handle variables release work in global lexical scope
  auto lscope = std::move(lexical_scope_stack.back());

  if (enable_emit_main()) {
    BasicBlock *retbb = (BasicBlock *)main_fn_node->fndefn.retbb;
    ir1.builder().CreateBr(retbb);
    main_fn->getBasicBlockList().push_back(retbb);
    ir1.builder().SetInsertPoint(retbb);
    Value *v = ir1.builder().CreateLoad((Value *)main_fn_node->fndefn.retslot, "retret");
    ir1.builder().CreateRet(v);

    // pop off the lexical block for the main function. When enhanced and define
    // other functions it will need encapsulate the related functions into function
    // or class
    if (enable_debug_info()) {
      diinfo->lexical_blocks.pop_back();

      // Validate the generated code, checking for consistency.
      verifyFunction(*main_fn);
    }
  }

#if 0
  // this code fragment should put into when generating a function failed
  // when generating each new function a lexical block should be pushed and when
  // the new function is generate over, then the should pop up the lexical block
  // when any error occurs when generating function body, then remove function.
  main_fn->eraseFromParent();

  // pop up the lexical block for the function since it is added unconditionally
  diinfo->lexical_blocks.pop_back();
#endif

  // finalize the debug info
  if (enable_debug_info())
    diinfo->dibuilder->finalize();

  std::string verify_message;
  llvm::raw_string_ostream rso(verify_message);
  bool verify_debug = true;
  if (verifyModule(ir1.module(), &rso, &verify_debug) ) {
    fprintf(stderr, "\nmodule verify failed: %s\n",
	    verify_message.c_str());
  }

  switch (genv.llvm_gen_type) {
  case LGT_LL:
    ret = llvm_codegen_ll(genv.outfile);
    break;
  case LGT_S:
    ret = llvm_codegen_native(CGFT_AssemblyFile, genv.outfile);
    break;
  case LGT_C:
    ret = llvm_codegen_native(CGFT_ObjectFile, genv.outfile);
    break;
  case LGT_JIT:
    ret = llvm_codegen_jit(genv.outfile);
    break;
  case LGT_NATIVE:
    {
    char objname[MAX_PATH + 1];
    sprintf(objname, "%s.o", genv.outfile);
    ret = llvm_codegen_native(CGFT_ObjectFile, objname);
    if (ret == -1)
      return ret;

    system(make_native_linker_command(objname, genv.outfile));
    }

    break;
  default:
    break;
  }

  return ret;
}

static void init_runtime_symbols() {
  std::vector<std::pair<const char *, void *>> name_addresses;
#ifdef TEST_RUNTIME
  name_addresses.push_back(std::make_pair("rt_add", (void *)&rt_add));
  name_addresses.push_back(std::make_pair("rt_sub", (void *)&rt_sub));
#endif
  jit1->register_imported_symbols(name_addresses);
}

BEGIN_EXTERN_C
void init_llvm_env() {
  ir1.init_module_and_passmanager(genv.src_path);
  jit1 = exit_on_error(jit_codegen::JIT1::create_instance());
  if (enable_debug_info())
    diinfo = std::make_unique<dwarf_debug::DWARFDebugInfo>(ir1.builder(), ir1.module(), genv.src_path);

  init_runtime_symbols();
}

int walk(RootTree *tree) {
  // first walk for iterating function prototype into llvm object
  // second walk for iterating all tree nodes
  int first_lexical_count = 0;

  llvm_codegen_begin(tree);
  while (walk_pass++ < 2) {
    NodeChain *p = tree->head;
    for (int i = 0; i < tree->count; ++i) {
      walk_stack(p->node);
      p = p->next;
    }

    if (walk_pass == 1) {
      first_lexical_count = curr_lexical_count;
      curr_lexical_count = 0;
    }
  }

  if (first_lexical_count != curr_lexical_count)
    yyerror("lexical count not identical in 2 pass: %d != %d\n",
	    first_lexical_count, curr_lexical_count);

  llvm_codegen_end();
  return 0;
}
END_EXTERN_C

