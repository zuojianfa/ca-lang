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

#include "ir1.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Support/TargetSelect.h"
#include <utility>

namespace ir_codegen {
IR1::IR1() {
  init_llvm_env();
}

IR1::~IR1() {
}

void IR1::init_llvm_env() {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();
  InitializeNativeTargetDisassembler();

#if 0
  // initialize all target if needed
  InitializeAllTargetInfos();
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmParsers();
  InitializeAllAsmPrinters();
  InitializeAllDisassemblers();
#endif
}

void IR1::init_module_and_passmanager(const char *modname) {
  // open new context and module
  _ctx = std::make_unique<LLVMContext>();

  _module = std::make_unique<Module>(modname, *_ctx);
  // _module->setDataLayout(llvm_jit->getDataLayout());

  // create new builder for the module
  _builder = std::make_unique<IRBuilder<>>(*_ctx);
  
  // create new function pass manager attached to it
  _fpm = std::make_unique<legacy::FunctionPassManager>(_module.get());

  // Do simple "peephole" optimizations and bit-twiddling optzns.
  _fpm->add(createInstructionCombiningPass());

  // Reassociate expressions.
  _fpm->add(createReassociatePass());

  // Eliminate Common SubExpressions.
  _fpm->add(createGVNPass());

  // Simplify the control flow graph (deleting unreachable blocks, etc).
  _fpm->add(createCFGSimplificationPass());
  _fpm->doInitialization();

  // create new pass manager attached to it
  _pm = std::make_unique<legacy::PassManager>();

  // Do simple "peephole" optimizations and bit-twiddling optzns.
  _pm->add(createInstructionCombiningPass());

  // Reassociate expressions.
  _pm->add(createReassociatePass());

  // Eliminate Common SubExpressions.
  _pm->add(createGVNPass());

  // Simplify the control flow graph (deleting unreachable blocks, etc).
  _pm->add(createCFGSimplificationPass());
}

Function *
IR1::gen_function(Type *retty, const char *name, std::vector<Type *> params,
                  std::vector<const char *> *param_names,
		  bool ispublic, bool isvararg)
{
  if (auto *f = _module->getFunction(name))
    return f;

  llvm::FunctionType *ft = llvm::FunctionType::get(retty, params, isvararg);
  auto linkage = ispublic ? llvm::Function::ExternalLinkage : llvm::Function::PrivateLinkage;
  llvm::Function *f = llvm::Function::Create(ft, linkage, name, _module.get());

  if (param_names) {
    int i = 0;
    for (auto &arg: f->args()) {
      if (i < param_names->size())
	arg.setName(param_names->at(i));
      else
	break;
      ++i;
    }
  }

  return f;
}

Function *IR1::gen_extern_fn(Type *retty, const char *name, std::vector<Type *> params,
			     std::vector<const char *> *param_names, bool isvararg) {
  return this->gen_function(retty, name, params, param_names, true, isvararg);
}


PHINode *IR1::gen_phi(Type *type, BasicBlock *thenbb, Value *v1, BasicBlock *elsebb, Value *v2) {
  PHINode *pn = builder().CreatePHI(type, 2, "iftmp");
  pn->addIncoming(v1, thenbb);
  pn->addIncoming(v2, elsebb);
  return pn;
}

AllocaInst *IR1::gen_entry_block_var(Function *fn, Type *type, const char *name, Value *defval) {
  IRBuilder<> tmpbuilder(&fn->getEntryBlock(), fn->getEntryBlock().begin());
  AllocaInst *slot = tmpbuilder.CreateAlloca(type, nullptr, name);
  if (defval)
    _builder->CreateStore(defval, slot, "slot");

  return slot;
}

AllocaInst *IR1::gen_var(Type *type, const char *name, Value *defval) {
  // the slot will be arraytype
  AllocaInst *slot = _builder->CreateAlloca(type, nullptr, name);
  if (defval)
    _builder->CreateStore(defval, slot, "slot");

  return slot;
}

GlobalVariable *IR1::gen_global_var(Type *type, const char *name, Value *defval, bool isconst, bool zeroinitial) {
#if 0
  if (!defval)
    defval = _module->getOrInsertGlobal(name, type);
  GlobalVariable *gvar = _module->getNamedGlobal(name);

  // dso_local global i32 @'name'
  // internal local is GlobalValue::InternalLinkage
  // dso_local global is GlobalValue::ExternalLinkage + DSOLocal
  gvar->setLinkage(GlobalValue::InternalLinkage);
  gvar->setDSOLocal(true);

#else
  if (zeroinitial) {
    ConstantAggregateZero* zerovalue = ConstantAggregateZero::get(type);
    defval = zerovalue;
  } else if (!defval)
    defval = Constant::getIntegerValue(int_type<int>(), APInt(32, 0));

  GlobalVariable* gvar = new GlobalVariable
    (*_module, type, isconst, GlobalValue::InternalLinkage, 0, name);
#endif

  //Align align = (static_cast<AllocaInst *>(defval))->getAlign();
  //gvar->setAlignment(align);
  gvar->setAlignment(MaybeAlign(4));

  // when not setInitializer the generated code will be:
  // `@count = internal global i32, align 4`
  // after invoke setInitializer the code will be:
  // `@count = internal global i32 @count, align 4`

  gvar->setInitializer((Constant *)defval);

  return gvar;
}

Constant *IR1::get_global_string(const std::string &s) {
  auto itr = _global_strs.find(s);
  if (itr == _global_strs.end()) {
    Constant *c = _builder->CreateGlobalStringPtr(s);
    _global_strs.insert(std::make_pair(s, c));
    return c;
  }

  return itr->second;
}

Value *IR1::gen_two_ops_value(Value *a, Value *b, const char *name,
			      two_fop_fn_t floatfn, two_op_fn_t intfn) {
  switch(a->getType()->getTypeID()) {
  case Type::FloatTyID:
  case Type::DoubleTyID:
    return (_builder.get()->*floatfn)(a, b, name, nullptr);
  case Type::IntegerTyID:
    return (_builder.get()->*intfn)(a, b, name, false, false);
  default:
    return nullptr;
  }  
}

Value *IR1::gen_add(Value *a, Value *b, const char *name) {
  return gen_two_ops_value(a, b, name, &IRBuilder<>::CreateFAdd, &IRBuilder<>::CreateAdd);
}

Value *IR1::gen_sub(Value *a, Value *b, const char *name) {
  return gen_two_ops_value(a, b, name, &IRBuilder<>::CreateFSub, &IRBuilder<>::CreateSub);
}

Value *IR1::gen_mul(Value *a, Value *b, const char *name) {
  return gen_two_ops_value(a, b, name, &IRBuilder<>::CreateFMul, &IRBuilder<>::CreateMul);
}

Value *IR1::gen_div(Value *a, Value *b, const char *name) {
  switch(a->getType()->getTypeID()) {
  case Type::FloatTyID:
  case Type::DoubleTyID:
    return _builder->CreateFDiv(a, b, name);
  case Type::IntegerTyID:
    // TODO: how to distinct the signed and unsigned div
    return _builder->CreateSDiv(a, b, name);
  default:
    return nullptr;
  }
}

Value *IR1::gen_mod(Value *a, Value *b, const char *name) {
  switch(a->getType()->getTypeID()) {
  case Type::FloatTyID:
  case Type::DoubleTyID:
    return _builder->CreateFRem(a, b, name);
  case Type::IntegerTyID:
    // TODO: how to distinct the signed and unsigned div
    return _builder->CreateSRem(a, b, name);
  default:
    return nullptr;
  }
}

Value *IR1::gen_cast_value(Instruction::CastOps castop, Value *from, Type *astype, const char *name) {    
  Value *castedvalue = builder().CreateCast(castop, from, astype, name);
  return castedvalue;
}

} // namespace ir_codegen

