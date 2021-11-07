#ifndef __type_system_llvm_h__
#define __type_system_llvm_h__

#include "llvm/IR/Type.h"
#include "llvm/IR/Instructions.h"

#ifdef __cplusplus
BEGIN_EXTERN_C
#endif

#include "ca.tab.h"

#ifdef __cplusplus
END_EXTERN_C
#endif

using ICO = llvm::Instruction::CastOps;

extern llvm::CmpInst::Predicate s_cmp_predicate[ATOMTYPE_END-VOID][6];

llvm::Instruction::CastOps gen_cast_ops(int fromtok, int totok);

llvm::Value *tidy_value_with_arith(llvm::Value *v, int typetok);
llvm::Type *gen_type_from_token(int tok);
llvm::Value *gen_literal_value(CALiteral *value, int typetok, SLoc loc);
llvm::Value *create_def_value(int typetok);
llvm::Value *generate_cmp_op(int typetok, llvm::Value *v1, llvm::Value *v2, int op);

#endif

