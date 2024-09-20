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

/**
 * @file The LLVM intermediate represention manager.
 */

#ifndef __codegen_ir1_h__
#define __codegen_ir1_h__

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/GlobalValue.h"
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APSInt.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>
//#include <bits/stdint-intn.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
//#include <llvm/IR/PassManager.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>

#include <memory>
#include <string>
#include <type_traits>
#include <vector>
#include <map>
#include <iostream>

namespace llvm {
typedef Value *(IRBuilder<>::*two_op_fn_t)(Value *a, Value *b, const Twine &name, bool nuw, bool nsw);
typedef Value *(IRBuilder<>::*two_op_cmp_fn_t)(Value *a, Value *b, const Twine &name);
typedef Value *(IRBuilder<>::*two_fop_fn_t)(Value *a, Value *b, const Twine &name, MDNode *FPMD);
}

namespace ir_codegen {

using namespace llvm;

class IR1 {
public:
  IR1();
  virtual ~IR1();
  void init_module_and_passmanager(const char *modname);

public:
  // generate variable
  Function *gen_function(Type *retty, const char *name, std::vector<Type *> params,
			 std::vector<const char *> *param_names = nullptr,
			 bool ispublic = true, bool isvararg = false);
  Function *gen_extern_fn(Type *retty, const char *name, std::vector<Type *> params,
			  std::vector<const char *> *param_names = nullptr, bool is_vararg = false);

  PHINode *gen_phi(Type *type, BasicBlock *thenbb, Value *v1, BasicBlock *elsebb, Value *v2);

  AllocaInst *gen_entry_block_var(Function *fn, Type *type, const char *name = "", Value *defval = nullptr);

  AllocaInst *gen_var(Type *type, const char *name = "", Value *defval = nullptr);

  GlobalVariable *gen_global_var(Type *type, const char *name = "", Value *defval = nullptr, bool isconst = false, bool zeroinitial = false);

  Constant *get_global_string(const std::string &s);

  StoreInst *store_var(Value *ptr, Value *v) {
    return _builder->CreateStore(v, ptr);
  }

  LoadInst *load_var(Value *ptr, const char *vname) {
    return _builder->CreateLoad(ptr, vname);
  }

  Value *gen_bool(bool value) {
    // TODO: handle real bool value 
    Type *type =  bool_type();
    APInt intv = APInt(1, value);
    Value *v1 = ConstantInt::getIntegerValue(type, intv);

    return v1;
  }

  template <typename I> Value *gen_int(I value) {
    Value *v1;
    Type *type = Type::getIntNTy(*_ctx, 8 * sizeof(I));
    APInt intv = APInt(8 * sizeof(I), value, std::is_signed<I>::value);
    v1 = ConstantInt::getIntegerValue(type, intv);
    /*
    if (std::is_signed<I>::value) {
      intv = APSInt(intv, false);
      v1 = ConstantInt::getIntegerValue(type, intv);
    } else {
      intv = APSInt(intv, true);
      v1 = ConstantInt::getIntegerValue(type, intv);
    }
    */

    return v1;
  }

  template <typename F> Value *gen_float(F value) {
    static_assert(std::is_same<F, float>::value || std::is_same<F, double>::value,
		  "F must be float type (float, double)");

    Type *ftype = float_type<F>();
    //APFloat fv = APFloat(value);
    return ConstantFP::get(ftype, value);
    // return ConstantFP::get(*_ctx, fv);
  }

  template <typename F>
  Value *gen_add(F a, F b, const char *name = "add");
  Value *gen_add(Value *a, Value *b, const char *name = "add");

  template <typename F>
  Value *gen_sub(F a, F b, const char *name = "sub");
  Value *gen_sub(Value *a, Value *b, const char *name = "sub");

  template <typename F>
  Value *gen_mul(F a, F b, const char *name = "mul");
  Value *gen_mul(Value *a, Value *b, const char *name = "mul");

  template <typename F>
  Value *gen_div(F a, F b, const char *name = "div");
  Value *gen_div(Value *a, Value *b, const char *name = "div");

  template <typename F>
  Value *gen_mod(F a, F b, const char *name = "mod");
  Value *gen_mod(Value *a, Value *b, const char *name = "mod");

  BasicBlock *gen_bb(const char *name, Function *parent = nullptr, BasicBlock *insert_before = nullptr) {
    return BasicBlock::Create(*_ctx, name, parent, insert_before);
  }

  Type *void_type() {
    return Type::getVoidTy(*_ctx);
  }

  Type *voidptr_type() {
    return PointerType::get(void_type(), 0);
  }

  template<typename I>
  Type *int_type() {
    if (std::is_same<I, int>::value)
      return Type::getInt32Ty(*_ctx);

    if (std::is_same<I, int64_t>::value)
      return Type::getInt64Ty(*_ctx);

    if (std::is_same<I, int16_t>::value)
      return Type::getInt16Ty(*_ctx);

    if (std::is_same<I, int8_t>::value)
      return Type::getInt8Ty(*_ctx);

    return Type::getIntNTy(*_ctx, 8 * sizeof(I));
  }

  template<typename I>
  Type *intptr_type() {
    if (std::is_same<I, int>::value)
      return Type::getInt32PtrTy(*_ctx);

    if (std::is_same<I, int64_t>::value)
      return Type::getInt64PtrTy(*_ctx);

    if (std::is_same<I, int16_t>::value)
      return Type::getInt16PtrTy(*_ctx);

    if (std::is_same<I, int8_t>::value ||
	std::is_same<I, char>::value)
      return Type::getInt8PtrTy(*_ctx);

    return Type::getIntNPtrTy(*_ctx, sizeof(I));
  }

  template<typename F>
  Type *float_type() {
    if (std::is_same<F, float>::value)
      return Type::getFloatTy(*_ctx);

    if (std::is_same<F, double>::value)
      return Type::getDoubleTy(*_ctx);

    if (std::is_same<F, long double>::value)
      return Type::getFP128Ty(*_ctx);

    return Type::getFloatTy(*_ctx);
  }

  template<typename F>
  Type *floatptr_type() {
    if (std::is_same<F, float>::value)
      return Type::getFloatPtrTy(*_ctx);

    if (std::is_same<F, double>::value)
      return Type::getDoublePtrTy(*_ctx);

    if (std::is_same<F, long double>::value)
      return Type::getFP128PtrTy(*_ctx);

    return Type::getFloatPtrTy(*_ctx);
  }

  Type *bool_type() {
    return Type::getInt1Ty(*_ctx);
  }

  Value *gen_cast_value(Instruction::CastOps castop, Value *from, Type *astype, const char *name = "");

public:
  LLVMContext &ctx() { return *_ctx.get(); }
  std::unique_ptr<LLVMContext> move_ctx() { return std::move(_ctx); }
  IRBuilder<> &builder() { return *_builder; }
  std::unique_ptr<IRBuilder<>> move_builder() { return std::move(_builder); }
  Module &module() { return *_module; }
  std::unique_ptr<Module> move_module() { return std::move(_module); }
  legacy::FunctionPassManager &fpm() { return *_fpm; }
  std::unique_ptr<legacy::FunctionPassManager> move_fpm() { return std::move(_fpm); }
  legacy::PassManager &pm() { return *_pm; }
  std::unique_ptr<legacy::PassManager> move_pm() { return std::move(_pm); }

private:
  void init_llvm_env();
  Value *gen_two_ops_value(Value *a, Value *b, const char *name,
			   two_fop_fn_t floatfn, two_op_fn_t intfn);
private:
  std::unique_ptr<LLVMContext> _ctx;
  std::unique_ptr<Module> _module;
  std::unique_ptr<IRBuilder<>> _builder;
  std::map<std::string, Value *> _syms;
  std::unique_ptr<legacy::FunctionPassManager> _fpm;
  std::unique_ptr<legacy::PassManager> _pm;
  std::map<std::string, Constant *> _global_strs;
};

template <typename F>
inline Value *IR1::gen_add(F a, F b, const char *name) {
  return _builder->CreateAdd(gen_int(a), gen_int(b), name);
}

template <>
inline Value *IR1::gen_add<float>(float a, float b, const char *name) {
  return _builder->CreateFAdd(gen_float(a), gen_float(b), name);
}

template <>
inline Value *IR1::gen_add<double>(double a, double b, const char *name) {
  return _builder->CreateFAdd(gen_float(a), gen_float(b), name);
}

template <typename F>
inline Value *IR1::gen_sub(F a, F b, const char *name) {
  return _builder->CreateSub(gen_int(a), gen_int(b), name);
}

template <>
inline Value *IR1::gen_sub<float>(float a, float b, const char *name) {
  return _builder->CreateFSub(gen_float(a), gen_float(b), name);
}

template <>
inline Value *IR1::gen_sub<double>(double a, double b, const char *name) {
  return _builder->CreateFSub(gen_float(a), gen_float(b), name);
}

template <typename F>
inline Value *IR1::gen_mul(F a, F b, const char *name) {
  return _builder->CreateMul(gen_int(a), gen_int(b), name);
}

template <>
inline Value *IR1::gen_mul<float>(float a, float b, const char *name) {
  return _builder->CreateFMul(gen_float(a), gen_float(b), name);
}

template <>
inline Value *IR1::gen_mul<double>(double a, double b, const char *name) {
  return _builder->CreateFMul(gen_float(a), gen_float(b), name);
}

template <typename F>
inline Value *IR1::gen_div(F a, F b, const char *name) {
  if (std::is_signed<F>::value)
    return _builder->CreateSDiv(gen_int(a), gen_int(b), name);
  else
    return _builder->CreateUDiv(gen_int(a), gen_int(b), name);
}

template <>
inline Value *IR1::gen_div<float>(float a, float b, const char *name) {
  return _builder->CreateFDiv(gen_float(a), gen_float(b), name);
}

template <>
inline Value *IR1::gen_div<double>(double a, double b, const char *name) {
  return _builder->CreateFDiv(gen_float(a), gen_float(b), name);
}

template <typename F>
inline Value *IR1::gen_mod(F a, F b, const char *name) {
  if (std::is_signed<F>::value)
    return _builder->CreateSRem(gen_int(a), gen_int(b), name);
  else
    return _builder->CreateURem(gen_int(a), gen_int(b), name);
}

template <>
inline Value *IR1::gen_mod<float>(float a, float b, const char *name) {
  return _builder->CreateFRem(gen_float(a), gen_float(b), name);
}

template <>
inline Value *IR1::gen_mod<double>(double a, double b, const char *name) {
  return _builder->CreateFRem(gen_float(a), gen_float(b), name);
}

} // namespace ir_codegen

#endif

