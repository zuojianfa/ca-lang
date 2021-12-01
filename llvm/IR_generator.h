#ifndef __IR_generator_h__
#define __IR_generator_h__

#include "ca_types.h"

#include <llvm/IR/Value.h>

enum OperandType {
  OT_Const,
  OT_Variable,
  OT_Load,
  OT_Store,
  OT_Alloc,
  OT_CallInst,
  OT_PHINode,
};

struct CalcOperand{
CalcOperand(OperandType t, llvm::Value *v, tokenid_t typetok) :
  type(t), operand(v), datatypetok(typetok) {}
  OperandType type;
  tokenid_t datatypetok;
  llvm::Value *operand;
};

#endif

