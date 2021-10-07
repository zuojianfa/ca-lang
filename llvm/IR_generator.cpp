#include "llvm/BinaryFormat/Dwarf.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/CodeGen.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <utility>
#include <vector>

#ifdef __cplusplus
extern "C" {
#endif

#include "ca.h"
#include "ca.tab.h"
#include "config.h"
#include "symtable.h"

void yyerror(const char *s, ...);
int yyparse(void);
int yyparser_init();

CompileEnv genv;

extern int glineno;
extern int gcolno;

#ifdef __cplusplus
}
#endif

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

// llvm section

// NEXT TODO: add debug information for functions

#define MAX_OPS 10000

#include <stack>

struct FnDebugInfo {
  DISubprogram *disp;
};

static llvm::Function *main_fn = nullptr;
static llvm::Function *curr_fn = nullptr; // handle when processing current function, the top level function is main function
static llvm::BasicBlock *main_bb = nullptr;
static llvm::DIFile *diunit = nullptr;
static std::vector<std::unique_ptr<CalcOperand>> oprand_stack;
static std::map<std::string, BasicBlock *> label_map; // for storing defined BasicBlock, or pre-define BasicBlock in GOTO statement
static std::map<std::string, Function *> function_map;
static std::map<std::string, Value *> var_map;
static std::map<llvm::Function *, std::unique_ptr<FnDebugInfo>> fn_debug_map;

static int walk_stack(ASTNode *p);

static Value *pop_right_value(const char *name = "load") {
  std::unique_ptr<CalcOperand> o = std::move(oprand_stack.back());
  oprand_stack.pop_back();

  Value *v;
  if (o->type == OT_Alloc) {
    v = ir1.builder().CreateLoad(o->operand, name);
  } else {
    v = o->operand;
  }

  return v;
}

static int enable_debug_info() { return genv.emit_debug; }
static int enable_emit_main() { return genv.emit_main; }

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

    /* should equal to arg.getName().str().c_str() */
    const char *name = symname_get(argn);

    // get the argument from call parameter into this field
    AllocaInst *slot = ir1.gen_var(ir1.int_type<int>(), name, &arg);

    if (enable_debug_info()) {
      DILocalVariable *divar = diinfo->dibuilder->createParameterVariable(disp, arg.getName(), i, diunit, row, diinfo->int_type_di(), true);

      const DILocation *diloc = DILocation::get(disp->getContext(), row, 0, disp);
      diinfo->dibuilder->insertDeclare(slot, divar, diinfo->dibuilder->createExpression(),
				       diloc, ir1.builder().GetInsertBlock());
    }

    // save the value into symbol table
    STEntry *entry = sym_getsym(st, argn, 0);
    entry->u.llvm_value = static_cast<void *>(slot);

    ++i;
  }
}

static void emit_global_var_dbginfo(const char *varname, int row) {
  llvm::DIGlobalVariableExpression *digve =
    diinfo->dibuilder->createGlobalVariableExpression(diunit, varname, StringRef(), diunit, row,
						      diinfo->int_type_di(), false);
}

static void emit_local_var_dbginfo(llvm::Function *fn, const char *varname, llvm::Value *var, int row) {
  auto itr = fn_debug_map.find(fn);
  if (itr == fn_debug_map.end())
    yyerror("cannot find function '%s' in map", fn->getName().str().c_str());

  auto &dbginfo = *itr->second;
  DILocalVariable *divar =
    diinfo->dibuilder->createAutoVariable(dbginfo.disp, varname, diunit, row, diinfo->int_type_di(), true);

  const DILocation *diloc = DILocation::get(dbginfo.disp->getContext(), row, 0, dbginfo.disp);
  diinfo->dibuilder->insertDeclare(var, divar, diinfo->dibuilder->createExpression(),
				   diloc, ir1.builder().GetInsertBlock());
}

static void walk_empty(ASTNode *p) {}

static Value *gen_literal_value(CALiteral *value) {
  switch (value->datatype->type) {
  case I32:
    return ir1.gen_int(value->u.i32value);
  case I64:
    return ir1.gen_int(value->u.i64value);
  case U32:
    return ir1.gen_int(value->u.u32value);
  case U64:
    return ir1.gen_int(value->u.u64value);
  case F32:
    return ir1.gen_float(value->u.f32value);
  case F64:
    return ir1.gen_float(value->u.f64value);
  case BOOL:
    return ir1.gen_bool(value->u.boolvalue);
  case CHAR:
    return ir1.gen_int(value->u.charvalue);
  case UCHAR:
    return ir1.gen_int(value->u.ucharvalue);
  default:
    return nullptr;
  }
}

static Value *walk_literal(ASTNode *p) {
  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col);

  Value *v = gen_literal_value(&p->litn.litv);
  auto operands = std::make_unique<CalcOperand>(OT_Const, v);
  oprand_stack.push_back(std::move(operands));

  return v;
}

// generate variable, if in a function then it is a local variable, when not in
// a function but `-nomain` is specified then generate a global variable else
// also generate a global variable for other use
static Value *walk_id_defv(ASTNode *p, Value *defval = nullptr) {
  Value *var;
  const char *name = symname_get(p->idn.i);
  //STEntry *entry = sym_getsym(p->symtable, p->idn.i, 1);
  STEntry *entry = p->entry;
  if (entry->sym_type != Sym_Variable)
    yyerror("line: %d, col: %d: '%s' Not a variable", entry->sloc.col, entry->sloc.row, name);

  if (entry->u.llvm_value) {
    var = static_cast<Value *>(entry->u.llvm_value);
    if (defval)
      ir1.builder().CreateStore(defval, var, name);
  } else {
    // if nomain specified then curr_fn and main_fn are all nullptr, so they are also equal
    if (curr_fn == main_fn) {
      var = ir1.gen_global_var(ir1.int_type<int>(), name, defval);
      if (enable_debug_info())
	emit_global_var_dbginfo(name, p->endloc.row);
    } else {
      var = ir1.gen_var(ir1.int_type<int>(), name, defval);
      if (enable_debug_info())
	emit_local_var_dbginfo(curr_fn, name, var, p->endloc.row);
    }
    entry->u.llvm_value = static_cast<void *>(var);
  }

#if 0
  auto itr = var_map.find(name);
  if (itr != var_map.end())
    var = itr->second;
  else {
    if (enable_debug_info())
      diinfo->emit_location(p->endloc.row, p->endloc.col);
    var = ir1.gen_var(ir1.int_type<int>(), name);
    var_map.insert(std::make_pair(name, var));
  }
#endif

  return var;
}

static Value *walk_id(ASTNode *p) {
  Value *var = walk_id_defv(p);
  
  auto operands = std::make_unique<CalcOperand>(OT_Alloc, var);
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

  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col);

  ir1.builder().CreateBr(bb);
  curr_fn->getBasicBlockList().push_back(bb);
  ir1.builder().SetInsertPoint(bb);
  return bb;
}

static void walk_label_goto(ASTNode *p) {}

static void walk_stmt_semicolon(ASTNode *p) {
  walk_stack(p->exprn.operands[0]);
  walk_stack(p->exprn.operands[1]);
}

static void walk_while(ASTNode *p) {
  if (!curr_fn)
    return;

  BasicBlock *condbb = ir1.gen_bb("condbb");
  BasicBlock *whilebb = ir1.gen_bb("whilebb");
  BasicBlock *endwhilebb = ir1.gen_bb("endwhilebb");

  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col);

  ir1.builder().CreateBr(condbb);
  curr_fn->getBasicBlockList().push_back(condbb);
  ir1.builder().SetInsertPoint(condbb);
  walk_stack(p->whilen.cond);
  Value *cond = pop_right_value("cond");
  cond = ir1.builder().CreateICmpNE(cond, ir1.gen_int(0), "while_cond_cmp");
  ir1.builder().CreateCondBr(cond, whilebb, endwhilebb);

  curr_fn->getBasicBlockList().push_back(whilebb);
  ir1.builder().SetInsertPoint(whilebb);

  walk_stack(p->whilen.body);
  // TODO: how to remove the stack element that never used?
  ir1.builder().CreateBr(condbb);

  curr_fn->getBasicBlockList().push_back(endwhilebb);
  ir1.builder().SetInsertPoint(endwhilebb);
}

static void walk_if(ASTNode *p) {
  if (!curr_fn)
    return;

  if (enable_debug_info())
    diinfo->emit_location(p->begloc.row, p->begloc.col);

  Value *tmpv1 = nullptr;
  Value *tmpv2 = nullptr;
  Value *tmpc = ir1.gen_var(ir1.int_type<int>(), "tmpc");

  BasicBlock *thenbb = ir1.gen_bb("thenbb");
  BasicBlock *outbb = ir1.gen_bb("outbb");
  BasicBlock *elsebb = nullptr;

  walk_stack(p->ifn.conds[0]);
  Value *cond = pop_right_value("cond");
  cond = ir1.builder().CreateICmpNE(cond, ir1.gen_int(0), "if_cond_cmp");
  if (p->ifn.remain) { /* if else */
    elsebb = ir1.gen_bb("elsebb");
    ir1.builder().CreateCondBr(cond, thenbb, elsebb);
    curr_fn->getBasicBlockList().push_back(thenbb);
    ir1.builder().SetInsertPoint(thenbb);
    walk_stack(p->ifn.bodies[0]);
    if (p->ifn.isexpr) {
      tmpv1 = pop_right_value("tmpv");
      ir1.store_var(tmpc, tmpv1);
    }

    ir1.builder().CreateBr(outbb);

    curr_fn->getBasicBlockList().push_back(elsebb);
    ir1.builder().SetInsertPoint(elsebb);
    walk_stack(p->ifn.remain);
    if (p->ifn.isexpr) {
      tmpv2 = pop_right_value("tmpv");
      ir1.store_var(tmpc, tmpv2);
    }
  } else { /* if */
    ir1.builder().CreateCondBr(cond, thenbb, outbb);
    curr_fn->getBasicBlockList().push_back(thenbb);
    ir1.builder().SetInsertPoint(thenbb);
    walk_stack(p->ifn.bodies[0]);
  }

  ir1.builder().CreateBr(outbb);
  curr_fn->getBasicBlockList().push_back(outbb);
  ir1.builder().SetInsertPoint(outbb);
  
  if (p->ifn.isexpr) {
#if 0 // the phi is not debugger friendly, so using alloc for temporary variable
    PHINode *phiv = ir1.gen_phi(ir1.int_type<int>(), thenbb, tmpv1, elsebb, tmpv2);
    auto pnv = std::make_unique<CalcOperand>(OT_PHINode, phiv);
#else
    auto pnv = std::make_unique<CalcOperand>(OT_Alloc, tmpc);
#endif

    oprand_stack.push_back(std::move(pnv));
  }
}

static void walk_stmt_print(ASTNode *p) {
  walk_stack(p->exprn.operands[0]);
  Value *v = pop_right_value();
  Function *printf_fn = ir1.module().getFunction("printf");
  if (!printf_fn)
    yyerror("cannot find declared extern printf function");

  Constant *format_str = ir1.builder().CreateGlobalStringPtr("%d\n");
  std::vector<Value *> printf_args(1, format_str);
  printf_args.push_back(v);

  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col);

  ir1.builder().CreateCall(printf_fn, printf_args, "n");
}

static void walk_stmt_assign(ASTNode *p) {
  walk_stack(p->exprn.operands[1]);
  Value *v = pop_right_value();
  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col);

  Value *vp = walk_id_defv(p->exprn.operands[0], v);

  oprand_stack.push_back(std::make_unique<CalcOperand>(OT_Store, vp));
}

static void walk_stmt_minus(ASTNode *p) {
  walk_stack(p->exprn.operands[0]);
  Value *v = pop_right_value();
  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col);
  v = ir1.gen_sub(ir1.gen_int(0), v);
  oprand_stack.push_back(std::make_unique<CalcOperand>(OT_Variable, v));
}

static void walk_stmt_goto(ASTNode *p) {
  ASTNode *label = p->exprn.operands[0];
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
    diinfo->emit_location(p->endloc.row, p->endloc.col);
  ir1.builder().CreateBr(bb);
}

static void walk_stmt_call(ASTNode *p) {
  ASTNode *name = p->exprn.operands[0];
  ASTNode *args = p->exprn.operands[1];
  const char *fnname = symname_get(name->idn.i);

  Function *fn = ir1.module().getFunction(fnname);
  if (!fn)
    yyerror("cannot find declared function: '%s'", fnname);

  if (args->exprn.op != ARG_LISTS_ACTUAL)
    yyerror("not a argument list: '%s'", fnname);

  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col);

  std::vector<Value *> argv;
  for (int i = 0; i < args->exprn.noperand; ++i) {
    Value *v;
    if (args->exprn.operands[i]->type == TTE_Literal) {
      v = gen_literal_value(&args->exprn.operands[i]->litn.litv);
    } else {
      const char *argname = symname_get(args->exprn.operands[i]->idn.i);
      walk_stack(args->exprn.operands[i]);
      v = pop_right_value(argname);
    }

    argv.push_back(v);
  }
 
  CallInst *callret = ir1.builder().CreateCall(fn, argv, fnname);
  auto operands = std::make_unique<CalcOperand>(OT_CallInst, callret);
  oprand_stack.push_back(std::move(operands));
}

static void walk_stmt_ret(ASTNode *p) {
  if (p->exprn.noperand) {
    walk_stack(p->exprn.operands[0]);
    Value *v = pop_right_value();
    if (enable_debug_info())
      diinfo->emit_location(p->endloc.row, p->endloc.col);

    ir1.builder().CreateRet(v);
  } else {
    if (enable_debug_info())
      diinfo->emit_location(p->endloc.row, p->endloc.col);

    ir1.builder().CreateRetVoid();
  }

  g_with_ret_value = true;
}

static void walk_stmt_expr(ASTNode *p) {
  walk_stack(p->exprn.operands[0]);
  Value *v1 = pop_right_value("v1");
  walk_stack(p->exprn.operands[1]);
  Value *v2 = pop_right_value("v2");
  Value *v3 = nullptr;

  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col);

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
  case '<':
    v3 = ir1.builder().CreateICmpSLT(v1, v2, "lt");
    v3 = ir1.builder().CreateSExt(v3, ir1.int_type<int>());
    break;
  case '>':
    v3 = ir1.builder().CreateICmpSGT(v1, v2, "gt");
    v3 = ir1.builder().CreateSExt(v3, ir1.int_type<int>());
    break;
  case GE:
    v3 = ir1.builder().CreateICmpSGE(v1, v2, "ge");
    v3 = ir1.builder().CreateSExt(v3, ir1.int_type<int>());
    break;
  case LE:
    v3 = ir1.builder().CreateICmpSGE(v1, v2, "le");
    v3 = ir1.builder().CreateSExt(v3, ir1.int_type<int>());
    break;
  case NE:
    v3 = ir1.builder().CreateICmpNE(v1, v2, "ne");
    v3 = ir1.builder().CreateSExt(v3, ir1.int_type<int>());
    break;
  case EQ:
    v3 = ir1.builder().CreateICmpEQ(v1, v2, "eq");
    v3 = ir1.builder().CreateSExt(v3, ir1.int_type<int>());
    break;
  default:
    yyerror("unknown expression operands: %d", p->exprn.op);
    break;
  }

  oprand_stack.push_back(std::make_unique<CalcOperand>(OT_Variable, v3));
}

static void walk_statement(ASTNode *p) {
  if (!curr_fn && p->exprn.op != '=' && p->exprn.op != ';')
    return;

  switch (p->exprn.op) {
  case ';':
    walk_stmt_semicolon(p);
    break;
  case PRINT:
    walk_stmt_print(p);
    break;
  case '=':
    walk_stmt_assign(p);
    break;
  case UMINUS:
    walk_stmt_minus(p);
    break;
  case GOTO:
    walk_stmt_goto(p);
    break;
  case FN_CALL:
    walk_stmt_call(p);
    break;
  case RET:
    walk_stmt_ret(p);
    break;
  default:
    walk_stmt_expr(p);
    break;
  }
}

static Function *walk_fn_declare(ASTNode *p) {
  const char *fnname = symname_get(p->fndecln.name);
  Function *fn = ir1.module().getFunction(fnname);
  if (!fn) {
    std::vector<const char *> param_names;
    std::vector<Type *> params;
    for (int i = 0; i < p->fndecln.args.argc; ++i) {
      param_names.push_back(symname_get(p->fndecln.args.argnames[i]));
      params.push_back(ir1.int_type<int>());
    }

    fn = ir1.gen_extern_fn(ir1.int_type<int>(), fnname, params, &param_names, !!p->fndecln.args.contain_varg);
    function_map.insert(std::make_pair(fnname, fn));
    fn->setCallingConv(CallingConv::C);

    //AttrListPtr func_printf_PAL;
    //fn->setAttributes(func_printf_PAL);
  }

  return fn;
}

static Function *walk_fn_define(ASTNode *p) {
  g_with_ret_value = false;
  Function *fn = walk_fn_declare(p->fndefn.fn_decl);
  if (p->fndefn.fn_decl->fndecln.args.argc != fn->arg_size())
    yyerror("argument number not identical with definition (%d != %d)",
	    p->fndefn.fn_decl->fndecln.args.argc, fn->arg_size());

  BasicBlock *bb = ir1.gen_bb("entry", fn);
  ir1.builder().SetInsertPoint(bb);
  curr_fn = fn;

  // insert here debugging information
  init_fn_param_info(fn, p->fndefn.fn_decl->fndecln.args, p->symtable, p->begloc.row);

  if (enable_debug_info())
    diinfo->emit_location(p->begloc.row, p->begloc.col);

  walk_stack(p->fndefn.stmts);

  if (enable_debug_info())
    diinfo->emit_location(p->endloc.row, p->endloc.col);

  if (!g_with_ret_value)
    ir1.builder().CreateRet(ir1.gen_int<int>(0));

  if (enable_debug_info())
    diinfo->lexical_blocks.pop_back();

  llvm::verifyFunction(*fn);

  if (enable_emit_main()) {
    ir1.builder().SetInsertPoint(main_bb);
    curr_fn = main_fn;
  } else {
    curr_fn = nullptr;
  }

  return fn;
}

typedef void (*walk_fn_t)(ASTNode *p);
static walk_fn_t walk_fn_array[TTE_Num] = {
  (walk_fn_t)walk_empty,
  (walk_fn_t)walk_literal,
  (walk_fn_t)walk_id,
  (walk_fn_t)walk_label,
  (walk_fn_t)walk_label_goto,
  (walk_fn_t)walk_statement,
  (walk_fn_t)walk_fn_declare,
  (walk_fn_t)walk_fn_define,
  (walk_fn_t)walk_while,
  (walk_fn_t)walk_if,
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
  curr_fn = nullptr;

  if (enable_debug_info())
    diunit = diinfo->create_difile();

  if (enable_emit_main()) {
    main_fn = ir1.gen_function(ir1.int_type<int>(), "main", std::vector<Type *>());
    main_bb = ir1.gen_bb("entry", main_fn);
    curr_fn = main_fn;
    ir1.builder().SetInsertPoint(main_bb);
    ST_ArgList arglist;
    arglist.argc = 0;

    init_fn_param_info(main_fn, arglist, tree->root_symtable, tree->begloc_main.row);
  }

  // TODO: add grammar for handling extern functions instead hardcoded here
  Function *printf_fn = ir1.module().getFunction("printf");
  if (!printf_fn) {
    auto param_names = std::vector<const char *>(1, "s");
    printf_fn = ir1.gen_extern_fn(ir1.int_type<int>(), "printf",
				  std::vector<Type *>(1, ir1.intptr_type<char>()),
				  &param_names,	true);
    function_map.insert(std::make_pair("printf", printf_fn));
    printf_fn->setCallingConv(CallingConv::C);

    //AttrListPtr func_printf_PAL;
    //printf_fn->setAttributes(func_printf_PAL);
  }

  return 0;
}

static int llvm_codegen_ll(const char *output = nullptr) {
  // run pass
  do_optimize_pass();

  if (output && output[0]) {
    std::error_code ec;
    raw_fd_ostream os(output, ec, llvm::sys::fs::F_None);
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
  CodeGenOpt::Level ol = CodeGenOpt::Default;
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
    raw_fd_ostream os(output, ec, llvm::sys::fs::F_None);
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

static int llvm_codegen_jit() {
  do_optimize_pass();

  ir1.module().setDataLayout(jit1->get_datalayout());
  auto rt = jit1->get_main_jitdl().createResourceTracker();
  auto tsm = orc::ThreadSafeModule(ir1.move_module(), ir1.move_ctx());
  exit_on_error(jit1->add_module(std::move(tsm), rt));
  auto func_symbol = exit_on_error(jit1->find("main"));
  int (*func)() = (int (*)())(intptr_t)func_symbol.getAddress();
  printf("func ret: %d\n", func());
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
  if (enable_emit_main()) {
    ir1.builder().CreateRet(ir1.gen_int(0));

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
  // TODO: this code fragment should put into when generating a function failed
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
    ret = llvm_codegen_jit();
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

#ifdef __cplusplus
extern "C"
#endif
int walk(RootTree *tree) {
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
  fprintf(stderr,
	  "Usage: ca [-ll] | [-S] | [-native] | [-c] | [-jit] [-O] | [-g] <input> [<output>]\n"
	  "Options:\n"
	  "         -ll:      compile into IR assembly file: .ll (llvm)\n"
	  "         -S:       compile into native (as) assembly file: .s\n"
	  "         -native:  compile into native execute file: ELF file on linux, PE file on windows (default value)\n"
	  "         -c:       compile into native object file: .o\n"
	  "         -jit:     interpret using jit (llvm)\n"
	  "         -O:       do optimization\n"
	  "         -g:       do not do any optimization (default value)\n"
	  );
  exit(-1);
}

static int init_config(int argc, char *argv[]) {
  int arg = 0;

  if (++arg >= argc)
    usage();

  genv.llvm_gen_type = LGT_NATIVE;
  genv.opt_level = OL_NONE;
  genv.emit_debug = 0;
  genv.emit_main = 1;

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
	genv.opt_level = OL_O1;
      } else if (!strcmp(argv[arg], "-g")) {
	genv.emit_debug = 1;
      } else if (!strcmp(argv[arg], "-nomain")) {
	genv.emit_main = 0;
      } else {
	usage();
      }
      ++arg;
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
    genv.goutput = fopen(argv[arg], "w+");
    if (!genv.goutput) {
      fprintf(stderr, "Open output file failed: %s, errno=%d\n", argv[arg], errno);
      return -1;
    }
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
  if (yyparser_init()) {
    fprintf(stderr, "init parser failed\n");
    exit(-1);
  }

  if (init_config(argc, argv)) {
    fprintf(stderr, "init config failed\n");
    exit(-1);
  }

  init_llvm_env();

  yyin = genv.ginput;
  yyparse();

  return 0;
}
