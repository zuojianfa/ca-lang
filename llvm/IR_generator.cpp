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
#include <cassert>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
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

#include "ca.h"
#include "ca_types.h"
#include "type_system.h"
#include "type_system_llvm.h"

BEGIN_EXTERN_C

#include "ca.tab.h"
#include "config.h"
#include "symtable.h"
#include "dotgraph.h"

int yyparse(void);
int yyparser_init();

CompileEnv genv;

extern int glineno;
extern int gcolno;
END_EXTERN_C

extern FILE *yyin;

// llvm section
#include "ir1.h"
#include "jit1.h"
#include "dwarf_debug.h"
#include "IR_generator.h"

using namespace llvm;
ir_codegen::IR1 ir1;
std::unique_ptr<dwarf_debug::DWARFDebugInfo> diinfo;
std::unique_ptr<jit_codegen::JIT1> jit1;

static ExitOnError exit_on_error;
static bool g_with_ret_value = false;
extern SymTable g_root_symtable;

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

#define MAX_OPS 10000

#include <stack>

struct FnDebugInfo {
  DISubprogram *disp;
};

static llvm::Function *g_box_fn = nullptr;
static llvm::Function *g_drop_fn = nullptr;
static llvm::Function *main_fn = nullptr;

// handle when processing current function, the top level function is main function
static llvm::Function *curr_fn = nullptr;
static ASTNode *curr_fn_node = nullptr;
static llvm::BasicBlock *main_bb = nullptr;
extern ASTNode *main_fn_node;
static llvm::DIFile *diunit = nullptr;
static std::vector<std::unique_ptr<CalcOperand>> oprand_stack;

// for storing defined BasicBlock, or pre-define BasicBlock in GOTO statement
static std::map<std::string, BasicBlock *> label_map;
static std::map<std::string, ASTNode *> function_map;

// TODO: should here using a current debug info instead of the map, it no need
// to use a map here, because it only used when function define, just like
// curr_fn
static std::map<Function *, std::unique_ptr<FnDebugInfo>> fn_debug_map;

static std::vector<std::unique_ptr<LexicalScope>> lexical_scope_stack;
static LexicalScope *curr_lexical_scope = nullptr;
static LexicalScope *root_lexical_scope = nullptr;

static int walk_stack(ASTNode *p);
extern RootTree *gtree;
extern std::unordered_map<typeid_t, void *> g_function_post_map;

// for handling function parameter checking
static std::unordered_map<typeid_t, void *> g_function_post_check_map;

static std::vector<std::unique_ptr<LoopControlInfo>> g_loop_controls;

std::vector<ASTNode *> *arrayexpr_deref(CAArrayExpr obj);
std::vector<void *> *structexpr_deref(CAStructExpr obj);

const static char *box_fn_name = "malloc";
const static char *drop_fn_name = "free";

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

static void llvmcode_printf(Function *fn, const char *format, ...) {
  Constant *llvmformat = ir1.builder().CreateGlobalStringPtr(format);
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

    // save the value into symbol table
    entry->u.varshielding.current->llvm_value = static_cast<void *>(slot);

    ++i;
  }
}

static DIType *ditype_get_or_create_from_catype(CADataType *catype, DIScope *scope);
static DIType *ditype_create_from_catype(CADataType *catype, DIScope *scope) {
  const char *name = nullptr;
  switch(catype->type) {
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
      const char *fieldname = catype->struct_layout->tuple ? nullptr : symname_get(field.name);

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
  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

  if (!p->litn.litv.fixed_type)
    inference_expr_type(p);

  CADataType *catype = catype_get_by_name(p->symtable, p->litn.litv.datatype);
  CHECK_GET_TYPE_VALUE(p, catype, p->litn.litv.datatype);

  Value *v = gen_literal_value(&p->litn.litv, catype, p->begloc);

  if (catype->type == ARRAY || catype->type == STRUCT) {
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
  walk_stack(node->aitemn.arraynode);
  auto pair = pop_right_value("aname", false);
  CADataType *arraycatype = pair.second;
  //CHECK_GET_TYPE_VALUE(node, arraycatype, entry->u.var->datatype);

  void *indices = node->aitemn.indices;
  size_t size = vec_size(indices);
  std::vector<Value *> vindices;
  CADataType *catype = arraycatype;
  vindices.push_back(ir1.gen_int(0));

  // Question: how to check the index is in scope of an array?
  // Answer: when the index is not constant, only can through runtime checking, e.g.
  // insert index scope checking code into generated code, (convert array bound into
  // llvm::Value object, and insert code to compare the index value and the bound value
  // print error or exit when out of bound
  for (int i = 0; i < size; ++i) {
    if (catype->type != ARRAY) {
      caerror(&(node->begloc), &(node->endloc), "type `%d` not an array on index `%d`",
	      catype->type, i);
      return nullptr;
    }

    ASTNode *expr = (ASTNode *)vec_at(indices, i);
    inference_expr_type(expr);
    walk_stack(expr);
    std::pair<Value *, CADataType *> pair = pop_right_value("item", true);
    if (!catype_is_integer(pair.second->type)) {
      caerror(&(node->begloc), &(node->endloc), "array index type must be integer, but find `%s` on `%d`",
	      catype_get_type_name(pair.second->signature), i);
      return nullptr;
    }

    vindices.push_back(pair.first);
    catype = catype->array_layout->type;
  }

  //Value *arrayvalue = static_cast<Value *>(entry->u.var->llvm_value);
  //Value *arrayvalue = pair.first;

  // arrayitemvalue: is an alloc memory address, so following can store value into it
  Value *arrayitemvalue = ir1.builder().CreateInBoundsGEP(pair.first, vindices);
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
  if (structcatype->struct_layout->tuple)
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

static Value *walk_id(ASTNode *p) {
  CADataType *catype = catype_get_by_name(p->symtable, p->entry->u.varshielding.current->datatype);
  CHECK_GET_TYPE_VALUE(p, catype, p->entry->u.varshielding.current->datatype);
  
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
  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

  ir1.builder().CreateBr(g_loop_controls.back()->outbb);

  BasicBlock *extrabb = ir1.gen_bb("extra", curr_fn);
  ir1.builder().SetInsertPoint(extrabb);
}

static void walk_continue(ASTNode *p) {
  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

  ir1.builder().CreateBr(g_loop_controls.back()->condbb);

  BasicBlock *extrabb = ir1.gen_bb("extra", curr_fn);
  ir1.builder().SetInsertPoint(extrabb);
}

static void walk_loop(ASTNode *p) {
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
  // NEXT TODO: handle the reference type variable
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

static void walk_box(ASTNode *p) {
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
  Value *stackv = ir1.gen_entry_block_var(curr_fn, type, "bindptr", heapv);
  
  // 4. copy Value willv into heap allocated space
  Type *pointeellvmty = llvmtype_from_catype(pointeety);
  aux_copy_llvmvalue_to_store(pointeellvmty, heapv, willv, "binddata");
  
  // 5. return the pointer memory address
  auto operands = std::make_unique<CalcOperand>(OT_Alloc, stackv, pointerty);
  oprand_stack.push_back(std::move(operands));
}

static void walk_drop(ASTNode *p) {
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
    assert(catype->range_layout->range->struct_layout->tuple == 2);
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
  case STRUCT: {
    const char *name = symname_get(catype->struct_layout->name);
    Constant *sname = ir1.builder().CreateGlobalStringPtr(name);
    CAStructField *fields = catype->struct_layout->fields;
    len = catype->struct_layout->fieldnum;
    int tuple = catype->struct_layout->tuple;
    const char *fmt = tuple == 2 ? "%s( " : tuple == 1 ? "%s ( " : "%s { ";
    llvmcode_printf(fn, fmt, sname, nullptr);
    for (int i = 0; i < len; ++i) {
      //ConstantArray *arrayv = static_cast<ConstantArray *>(v);
      //Constant *subv = arrayv->getAggregateElement(i);

      //Value *idx = ir1.gen_int(i);
      //Value *subv = ir1.builder().CreateGEP(v, idx, "subv");

      //Type* array_t =  llvm::PointerType::getUnqual(v->getType());
      if (!tuple) {
	name = symname_get(fields[i].name); // field name
	sname = ir1.builder().CreateGlobalStringPtr(name);
	llvmcode_printf(fn, "%s: ", sname, nullptr);
      }

      Value *subv = ir1.builder().CreateExtractValue(v, i);
      dbgprint_value(fn, fields[i].type, subv);

      if (i < len - 1)
	llvmcode_printf(fn, ", ", nullptr);
    }

    llvmcode_printf(fn, tuple ? " )" : " }", nullptr);
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

static void walk_dbgprint(ASTNode *p) {
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
}

static void walk_dbgprinttype(ASTNode *p) {
  CADataType *dt = catype_get_by_name(p->symtable, p->printtypen.type);
  int typesize = dt->size;

  // print datatype information when compiling
  debug_catype_datatype(dt);

#if 1 // when need print in run uncomment it
  Function *printf_fn = ir1.module().getFunction("printf");
  if (!printf_fn)
    yyerror("cannot find declared extern printf function");

  // handle expression value transfer
  const char *format = "size = %lu, type: %s\n";
  Constant *format_str = ir1.builder().CreateGlobalStringPtr(format);

  // TODO: type string
  Constant *type_str = ir1.builder().CreateGlobalStringPtr(symname_get(dt->signature));

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
  // idn can be type of TTE_Id or TTE_DerefLeft or TTE_ArrayItemLeft
  ASTNode *idn = p->assignn.id;
  ASTNode *exprn = p->assignn.expr;
  int assignop = p->assignn.op;

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

static void determine_letbind_type(CAPattern *cap, CADataType *catype,
                                   SymTable *symtable);

static void register_variable_catype(int var, typeid_t type, SymTable *symtable) {
  STEntry *entry = sym_getsym(symtable, var, 0);
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

static void inference_letbind_type_both_side(CAPattern *cap, ASTNode *exprn);

// determine variable types in pattern and do format checking for let binding operation
static void inference_letbind_pattern_range(CAPattern *cap, ASTNode *tuplenode, int from, int to, int typeoffset) {
  for (int i = from; i < to; ++i) {
    inference_letbind_type_both_side(cap->items->patterns[i], tuplenode->arglistn.exprs[i + typeoffset]);
  }
}

static void varshielding_rotate_capattern(CAPattern *cap, SymTable *symtable, bool is_back = false);
static void inference_letbind_type_both_side(CAPattern *cap, ASTNode *exprn) {
  switch (cap->type) {
  case PT_IgnoreOne:
  case PT_IgnoreRange:
  case PT_Var: {
    // for variable shielding, resolving `let a = a;` statement
    varshielding_rotate_capattern(cap, exprn->symtable, true);
    typeid_t type = inference_expr_type(exprn);
    varshielding_rotate_capattern(cap, exprn->symtable, false);
    if (cap->type == PT_Var)
      capattern_register_variable_catype(cap, type, exprn->symtable);

    break;
  }
  case PT_GenTuple: {
    if (exprn->type != TTE_Expr || exprn->exprn.op != TUPLE || exprn->exprn.noperand != 1) {
      caerror(&(cap->loc), NULL, "the right side expression is not a genral tuple type");
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
      inference_letbind_pattern_range(cap, tuplenode, 0, cap->items->size, 0);
    } else {
      // with ignore range .., x1, x2, .., xm, xn, example: (v1, v2, ..(2), vm, vn) = (t1, t2, t3, ..., tx, tm, tn)
      // handle starting and ending matches
      inference_letbind_pattern_range(cap, tuplenode, 0, ignorerangepos, 0);
      inference_letbind_pattern_range(cap, tuplenode, ignorerangepos + 1, cap->items->size, tuplenode->arglistn.argc - cap->items->size);
    }

    CADataType *catype = catype_from_capattern(cap, exprn->symtable);
    if (!catype)
      caerror(&(cap->loc), NULL, "get general tuple type `%s` failed from determined pattern");

    if (cap->morebind)
      bind_register_variable_catype(cap->morebind, catype->signature, exprn->symtable);

    varshielding_rotate_capattern(cap, exprn->symtable, true);
    determine_expr_type(exprn, catype->signature);
    varshielding_rotate_capattern(cap, exprn->symtable, false);
    break;
  }
  case PT_Tuple:
  case PT_Struct: {
    // named struct / tuple must can create a type according to it's name
    CADataType *catype = capattern_check_get_type(cap, exprn);
    CHECK_GET_TYPE_VALUE(exprn, catype, cap->name);

    varshielding_rotate_capattern(cap, exprn->symtable, true);
    determine_expr_type(exprn, catype->signature);
    varshielding_rotate_capattern(cap, exprn->symtable, false);

    determine_letbind_type(cap, catype, exprn->symtable);
    break;
    }
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
  inference_letbind_type_both_side(cap, exprn);
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

static void determine_letbind_type_for_struct(CAPattern *cap, CADataType *catype, SymTable *symtable, int tuple) {
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

  if (tuple != catype->struct_layout->tuple) {
    // it means the tuple using struct form of match
    // now allow
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

    assert(catype->struct_layout->tuple != 2);
    for (int i = 0; i < endpos; ++i) {
      int fieldname = cap->items->patterns[i]->fieldname;
      assert(fieldname != -1);
      CADataType *dt = nullptr;
      if (catype->struct_layout->tuple == 1) {
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
    determine_letbind_type_for_struct(cap, catype, symtable, cap->type == PT_Tuple ? 1 : 2);
    break;
  case PT_Struct:
    determine_letbind_type_for_struct(cap, catype, symtable, 0);
    break;
  case PT_IgnoreOne:
    break;
  case PT_IgnoreRange:
    yyerror("inner error: should not come here, upper logic already processed");
    break;
  default:
    yyerror("inner error: unknown pattern type: `%d`", cap->type);
    break;
  }
}

static Value *bind_variable_value(SymTable *symtable, int name, Value *value) {
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
#if 1
    var = ir1.gen_entry_block_var(curr_fn, type, varname, nullptr);
#else
    BasicBlock *currbb = ir1.builder().GetInsertBlock();
    BasicBlock *entrybb = &curr_fn->getEntryBlock();
    BasicBlock::iterator itr = entrybb->getFirstInsertionPt();
    ir1.builder().SetInsertPoint(entrybb, itr);
    var = ir1.gen_var(type, varname, nullptr);
    ir1.builder().SetInsertPoint(currbb);
#endif

    if (!value)
      aux_set_zero_to_store(type, var);
    else
      aux_copy_llvmvalue_to_store(type, var, value, varname);

    if (enable_debug_info())
      emit_local_var_dbginfo(curr_fn, varname, catype, var, entry->u.varshielding.current->loc.row /* p->endloc.row */);
  }

  entry->u.varshielding.current->llvm_value = static_cast<void *>(var);
  return var;
}

static void atmore_bind_variable_value(SymTable *symtable, void *morebind, Value *value) {
  size_t size = vec_size(morebind);
  for (size_t i = 0; i < size; ++i) {
    int name = (int)(long)vec_at(morebind, i);
    bind_variable_value(symtable, name, value);
  }
}

static void capattern_bind_variable_value(SymTable *symtable, CAPattern *cap, Value *value) {
  bind_variable_value(symtable, cap->name, value);
  atmore_bind_variable_value(symtable, cap->morebind, value);
}

static void capattern_bind_value(SymTable *symtable, CAPattern *cap,
                                 Value *value, CADataType *catype);

// determine variable types in pattern and do format checking for let binding operation
static void capattern_bind_pattern_range(SymTable *symtable, CAPattern *cap, Value *value, CADataType *catype,
					 int from, int to, int typeoffset) {
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

    capattern_bind_value(symtable, cap->items->patterns[i], subvalue, btype);
  }
}

static void capattern_bind_struct_value(SymTable *symtable, CAPattern *cap, Value *value, CADataType *catype, int tuple) {
  // when in this function the value should already come with the type of capattern
  if (cap->morebind)
    atmore_bind_variable_value(symtable, cap->morebind, value);

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
      if (catype->struct_layout->tuple == 0)
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

      capattern_bind_value(symtable, cap->items->patterns[i], subvalue, btype);
    }
    return;
  }

  // handle tuple value like upper
  if (ignorerangepos == -1) {
    // with no ignore range ..
    capattern_bind_pattern_range(symtable, cap, value, catype, 0, cap->items->size, 0);
  } else {
    // with ignore range .., x1, x2, .., xm, xn, example: (v1, v2, ..(2), vm, vn) = (t1, t2, t3, ..., tx, tm, tn)
    // handle starting and ending matches
    capattern_bind_pattern_range(symtable, cap, value, catype, 0, ignorerangepos, 0);
    capattern_bind_pattern_range(symtable, cap, value, catype, ignorerangepos + 1, cap->items->size, catype->struct_layout->fieldnum-cap->items->size);
  }  
}

static void capattern_bind_value(SymTable *symtable, CAPattern *cap, Value *value, CADataType *catype) {
  switch (cap->type) {
  case PT_Var:
    capattern_bind_variable_value(symtable, cap, value);
    break;
  case PT_Tuple:
  case PT_GenTuple:
    capattern_bind_struct_value(symtable, cap, value, catype, cap->type == PT_Tuple ? 1 : 2);
    break;
  case PT_Struct:
    capattern_bind_struct_value(symtable, cap, value, catype, 0);
    break;
  case PT_IgnoreOne:
    break;
  case PT_IgnoreRange:
    yyerror("inner error: should not come here, upper logic already processed");
    break;
  default:
    yyerror("inner error: unknown pattern type: `%d`", cap->type);
    break;
  }
}

static void varshielding_rotate_variable(CAVariableShielding *shielding, bool is_back = false) {
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
  case PT_Tuple:
  case PT_GenTuple:
  case PT_Struct:
    varshielding_rotate_capattern_struct(cap, symtable, is_back);
    break;
  case PT_IgnoreOne:
  case PT_IgnoreRange:
    break;
  default:
    yyerror("inner error: unknown pattern type: `%d`", cap->type);
    break;
  }
}

// NEXT TODO: for local and global variable binding, refactor walk_assign function
static void walk_letbind(ASTNode *p) {
  CAPattern *cap = p->letbindn.cap;
  ASTNode *exprn = p->letbindn.expr;
  Value *v = nullptr;
  CADataType *catype = nullptr;
  OperandType ot = OT_Alloc;
    
  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

  varshielding_rotate_capattern(cap, p->symtable);

  if (exprn->type != TTE_VarDefZeroValue) {
    // 1. inference type for both side of binding, to determine the types of both side
    // when is not zeroinitial
    inference_letbind_type(cap, exprn);

    // 2. walk right side node and get Value
    varshielding_rotate_capattern(cap, exprn->symtable, true);
    walk_stack(exprn);
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
  }

  if (v && ot == OT_Alloc && !catype_is_complex_type(catype))
    v = ir1.builder().CreateLoad(v, "tmpexpr");

  // 3. walk left side again and copy data from right side
  capattern_bind_value(exprn->symtable, cap, v, catype);
}

static void walk_expr_tuple_common(ASTNode *p, CADataType *catype, std::vector<Value *> &values);
static void walk_range(ASTNode *p) {
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

static void check_and_determine_param_type(ASTNode *name, ASTNode *param, int tuple, STEntry *entry) {
  typeid_t fnname = name->idn.i;
  if (name->idn.idtype != IdType::TTEId_FnName) {
      caerror(&(param->begloc), &(param->endloc), "the id: `%s` is not `%s` name",
	      symname_get(fnname), tuple ? "tuple" : "function");
      return;
  }

  check_fn_define(fnname, param, tuple, entry);

  ST_ArgList *formalparam = nullptr;
  if (tuple)
    formalparam = entry->u.datatype.members;
  else
    formalparam = entry->u.f.arglists;

  // check and determine parameter type
  for (int i = 0; i < param->arglistn.argc; ++i) {
    typeid_t formaltype = typeid_novalue;
    if (i >= formalparam->argc) {
      // it is a variable parameter ...
      formaltype = typeid_novalue;
    } else {
      typeid_t datatype = typeid_novalue;
      if (tuple) {
	datatype = formalparam->types[i];
      } else {
	STEntry *paramentry = sym_getsym(formalparam->symtable, formalparam->argnames[i], 0);
	datatype = paramentry->u.varshielding.current->datatype;
      }

      CADataType *dt = catype_get_by_name(name->symtable, datatype);
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
      return;
    }
  }
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

// the expression call may be a function call or tuple literal definition,
// because the tuple literal form is the same as function, so handle it here
static void walk_expr_call(ASTNode *p) {
  ASTNode *name = p->exprn.operands[0];
  ASTNode *args = p->exprn.operands[1];

  const char *fnname = nullptr;
  Function *fn = nullptr;
  STEntry *entry = nullptr;
  int istuple = extract_function_or_tuple(p->symtable, name->idn.i, &entry, &fnname, (void **)&fn);
  if (istuple == -1) {
      caerror(&(p->begloc), &(p->endloc), "cannot find declared function: '%s'", fnname);
      return;
  }

  check_and_determine_param_type(name, args, istuple, entry);

  if (args->type != TTE_ArgList)
    caerror(&(p->begloc), &(p->endloc), "not a argument list: '%s'", fnname);

  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col, curr_lexical_scope->discope);

  std::vector<Value *> argv;
  llvmvalue_from_exprs(args->arglistn.exprs, args->arglistn.argc, argv, !istuple);

  if (istuple)
    return walk_expr_tuple(p, entry, argv);

  Type *rettype = fn->getReturnType();
  bool isvoidty = rettype->isVoidTy();

  auto itr = function_map.find(fnname);
  if (itr == function_map.end()) {
    caerror(&(p->begloc), &(p->endloc), "cannot find function '%s' node", fnname);
    return;
  }
 
  CallInst *callret = ir1.builder().CreateCall(fn, argv, isvoidty ? "" : fnname);
  CADataType *retdt = catype_get_by_name(p->symtable, itr->second->fndecln.ret);
  CHECK_GET_TYPE_VALUE(p, retdt, itr->second->fndecln.ret);

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

  for (size_t i = 0; i < values.size(); ++i) {
    // get elements address of arr
    Value *idxvi = ir1.gen_int(i);
    idxv[1] = idxvi;
    Value *dest = ir1.builder().CreateGEP(arr, idxv);
    aux_copy_llvmvalue_to_store(lefttype, dest, values[i], "tmpsuba");
  }
  
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

  // NEXT TODO:
  //auto pair = pop_right_value();
}

static void walk_expr(ASTNode *p) {
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

    CADataType *prevcatype = catype_get_by_name(prevargs->symtable, preventry->u.varshielding.current->datatype);
    CADataType *currcatype = catype_get_by_name(currargs->symtable, currentry->u.varshielding.current->datatype);

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

static Function *walk_fn_declare(ASTNode *p) {
  auto fitr = g_function_post_check_map.find(p->fndecln.name);
  if (fitr != g_function_post_check_map.end()) {
    // check redeclared function parameter
    STEntry *preventry = sym_getsym(&g_root_symtable, p->fndecln.name, 0);
    post_check_fn_proto(preventry, p->fndecln.name, &p->fndecln.args, p->fndecln.ret);
  } else {
    g_function_post_check_map[p->fndecln.name] = static_cast<void *>(p);
  }

  const char *fnname = catype_get_function_name(p->fndecln.name);

  Function *fn = ir1.module().getFunction(fnname);
  auto itr = function_map.find(fnname);
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

    CADataType *dt = catype_get_by_name(p->symtable, entry->u.varshielding.current->datatype);
    CHECK_GET_TYPE_VALUE(p, dt, entry->u.varshielding.current->datatype);

    Type *type = llvmtype_from_catype(dt);
    params.push_back(type);
  }

  CADataType *retdt = catype_get_by_name(p->symtable, p->fndecln.ret);
  CHECK_GET_TYPE_VALUE(p, retdt, p->fndecln.ret);
  Type *rettype = llvmtype_from_catype(retdt);
  fn = ir1.gen_extern_fn(rettype, fnname, params, &param_names, !!p->fndecln.args.contain_varg);
  function_map.insert(std::make_pair(fnname, p));
  fn->setCallingConv(CallingConv::C);

  //AttrListPtr func_printf_PAL;
  //fn->setAttributes(func_printf_PAL);

  return fn;
}

static void generate_final_return(ASTNode *p) {
  // should check if the function returned a value instead of append a return value always
  CADataType *retdt = catype_get_by_name(p->symtable, p->fndefn.fn_decl->fndecln.ret);
  CHECK_GET_TYPE_VALUE(p, retdt, p->fndefn.fn_decl->fndecln.ret);

  if (g_with_ret_value) {
    if (retdt->type == VOID) {
      ir1.builder().CreateRetVoid();
    } else {
      Value *v = ir1.builder().CreateLoad((Value *)p->fndefn.retslot, "retret");
      ir1.builder().CreateRet(v);
    }
    return;
  }

  Value *retv = create_default_integer_value(retdt->type);
  if (retv)
    ir1.builder().CreateRet(retv);
  else
    ir1.builder().CreateRetVoid();
}

static Function *walk_fn_define(ASTNode *p) {
  g_with_ret_value = false;
  Function *fn = walk_fn_declare(p->fndefn.fn_decl);
  if (p->fndefn.fn_decl->fndecln.args.argc != fn->arg_size())
    yyerror("argument number not identical with definition (%d != %d)",
	    p->fndefn.fn_decl->fndecln.args.argc, fn->arg_size());

  curr_fn_node = p;
  curr_fn = fn;

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

  DIScope *save_discope = curr_lexical_scope->discope;

  walk_stack(p->fndefn.stmts);

  // the return statement is not in source code, but be added by the compiler
  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col);

  ir1.builder().CreateBr(retbb);
  fn->getBasicBlockList().push_back(retbb);
  ir1.builder().SetInsertPoint(retbb);
  generate_final_return(p);

  if (enable_debug_info()) {
    diinfo->lexical_blocks.pop_back();

    // finalize the debug info for verify function successfully, it may slow down
    // the performance invoke here, may move all function verification into later
    // TODO: move into all module is processed
    diinfo->dibuilder->finalize();
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
    curr_fn_node = nullptr;
    curr_fn = nullptr;
  }

  return fn;
}

static void walk_struct(ASTNode *node) {
  // only check struct definition, but not generate Type object

  STEntry *entry = node->entry;
  typeid_t id = entry->u.datatype.id;

  CADataType *dt = catype_get_by_name(node->symtable, id);
  CHECK_GET_TYPE_VALUE(node, dt, id);
}

static void walk_typedef(ASTNode *node) {
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
  if (enable_debug_info()) {
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
  lexical_scope_stack.push_back(std::move(lscope));

  walk_stack(node->lnoden.stmts);

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

  sprintf(command, "ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o %s -o %s -lc", input, output);

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

static void init_llvm_env() {
  ir1.init_module_and_passmanager(genv.src_path);
  jit1 = exit_on_error(jit_codegen::JIT1::create_instance());
  if (enable_debug_info())
    diinfo = std::make_unique<dwarf_debug::DWARFDebugInfo>(ir1.builder(), ir1.module(), genv.src_path);
}

static void handle_post_functions() {
  for (auto itr = g_function_post_map.begin(); itr != g_function_post_map.end(); ++itr) {
    CallParamAux *paramaux = (CallParamAux *)itr->second;
    if (!paramaux->checked) {
      yyerror("function or tuple expression `%s` is used but not defined", catype_get_type_name(itr->first));
      return;
    }

    ASTNode *node = paramaux->param;

    if (node->type == TTE_FnDecl)
      walk_fn_declare(node);
    else if (node->type == TTE_FnDef)
      walk_fn_declare(node->fndefn.fn_decl);
    else
      yyerror("(internal) not a function declare or definition");
  }
}

static int walk(RootTree *tree) {
  NodeChain *p = tree->head;
  llvm_codegen_begin(tree);
  for (int i = 0; i < tree->count; ++i) {
    walk_stack(p->node);
    p = p->next;
  }

  llvm_codegen_end();
  return 0;
}

static void usage() {
  // [-ll] | [-S] | [-native] | [-c] | [-jit] [-O] | [-g] | [-nomain] | [-dot <dotfile>]
  fprintf(stderr,
	  "Usage: ca [options] <input> [<output>]\n"
	  "Options:\n"
	  "         -ll:      compile into IR assembly file: .ll (llvm)\n"
	  "         -S:       compile into native (as) assembly file: .s\n"
	  "         -native:  compile into native execute file: ELF file on linux, PE file on windows (default value)\n"
	  "         -c:       compile into native object file: .o\n"
	  "         -jit:     interpret using jit (llvm)\n"
	  "         -O[123]:  do optimization of level 1 2 3, default is level 2\n"
	  "         -g:       do not do any optimization (default value)\n"
	  "         -main:    do generate the default main function\n"
	  "         -dot <dotfile>:  generate the do not generate the default main function\n"
	  );
  exit(-1);
}

static int init_config(int argc, char *argv[]) {
  int arg = 0;

  if (++arg >= argc)
    usage();

  genv.llvm_gen_type = LGT_JIT;
  genv.opt_level = OL_NONE;
  genv.emit_debug = 0;
  genv.emit_main = 0;
  genv.emit_dot = 0;
  genv.dot_sparsed = 1;

  int i = 0;
  while(i < 3) {
    if (argv[arg][0] == '-') {
      if (!strcmp(argv[arg], "-ll")) {
	genv.llvm_gen_type = LGT_LL;
      } else if (!strcmp(argv[arg], "-S")) {
	genv.llvm_gen_type = LGT_S;
      } else if (!strcmp(argv[arg], "-native")) {
	genv.llvm_gen_type = LGT_NATIVE;
      } else if (!strcmp(argv[arg], "-c")) {
	genv.llvm_gen_type = LGT_C;
      } else if (!strcmp(argv[arg], "-jit")) {
	genv.llvm_gen_type = LGT_JIT;
      } else if (!strcmp(argv[arg], "-O")) {
	genv.opt_level = OL_O2;
      } else if (!strcmp(argv[arg], "-O1")) {
	genv.opt_level = OL_O1;
      } else if (!strcmp(argv[arg], "-O2")) {
	genv.opt_level = OL_O2;
      } else if (!strcmp(argv[arg], "-O3")) {
	genv.opt_level = OL_O3;
      } else if (!strcmp(argv[arg], "-g")) {
	genv.emit_debug = 1;
      } else if (!strcmp(argv[arg], "-main")) {
	genv.emit_main = 1;
      } else if (!strcmp(argv[arg], "-dot")) {
	genv.emit_dot = 1;
	if (++arg >= argc || argv[arg][0] == '-') {
	  fprintf(stderr, "Should specify a dot file path\n\n");
	  usage();
	}
	strcpy(genv.dotpath, argv[arg]);
      } else {
	usage();
      }

      if (++arg >= argc)
	usage();

      ++i;
      continue;
    }

    break;
  }

  if (arg >= argc)
    usage();

  size_t len = strlen(argv[arg]);
  if (len > MAX_PATH) {
    fprintf(stderr, "too long of source file path: %s\n", argv[arg]);
    return -1;
  }

  strcpy(genv.src_path, argv[arg]);

  genv.ginput = fopen(argv[arg], "r");
  if (!genv.ginput) {
    fprintf(stderr, "Open input file failed: %s, errno=%d\n", argv[1], errno);
    return -1;
  }

  if (++arg >= argc) {
    genv.outfile[0] = '\0';
    genv.goutput = stdout;
  } else {
    strcpy(genv.outfile, argv[arg]);
#if 0
    genv.goutput = fopen(argv[arg], "w+");
    if (!genv.goutput) {
      fprintf(stderr, "Open output file failed: %s, errno=%d\n", argv[arg], errno);
      return -1;
    }
#endif
  }

  genv.code_buffer = (int *)calloc(MAX_OPS, sizeof(int));
  if (!genv.code_buffer) {
    fprintf(stderr, "Allocate buffer failed, errno=%d\n", errno);
    return -1;
  }

  genv.stackbased = 1;

  return 0;
}

int main(int argc, char *argv[]) {
  if (init_config(argc, argv)) {
    fprintf(stderr, "init config failed\n");
    exit(-1);
  }

  if (yyparser_init()) {
    fprintf(stderr, "init parser failed\n");
    exit(-1);
  }

  init_llvm_env();

  yyin = genv.ginput;

  if (genv.llvm_gen_type == LGT_JIT)
    fprintf(stderr, "program `%s` :\n", genv.src_path);

  yyparse();

  handle_post_functions();

  walk(gtree);

  dot_finalize();

  return 0;
}
