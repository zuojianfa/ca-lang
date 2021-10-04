#ifndef __IR_generator_h__
#define __IR_generator_h__

#include <llvm/IR/Value.h>

enum OperandType {
  OT_Const,
  OT_Variable,
  OT_Load,
  OT_Store,
  OT_Alloc,
  OT_CallInst,
};

struct CalcOperand{
CalcOperand(OperandType t, llvm::Value *v) : type(t), operand(v) {}
  OperandType type;
  llvm::Value *operand;
};

#endif

