#ifndef __type_system_llvm_h__
#define __type_system_llvm_h__

#include "ca_types.h"
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

llvm::Instruction::CastOps gen_cast_ops(CADataType *fromtype, CADataType *totype);
llvm::Instruction::CastOps gen_cast_ops_token(tokenid_t fromtok, tokenid_t totok);

llvm::Value *tidy_value_with_arith(llvm::Value *v, int typetok);
llvm::Value *gen_zero_literal_value(CADataType *catype);
llvm::Value *gen_literal_value(CALiteral *value, CADataType *catype, SLoc loc);
llvm::Value *create_default_integer_value(int typetok, int64_t defv = 0);
llvm::Value *generate_cmp_op(int typetok, llvm::Value *v1, llvm::Value *v2, int op);
llvm::Type *llvmtype_from_token(int tok);
llvm::Type *llvmtype_from_catype(CADataType *type);

#endif

