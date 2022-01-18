#ifndef __IR_generator_h__
#define __IR_generator_h__

#include "ca_types.h"
#include "ca.h"
#include "symtable.h"

#include <llvm/IR/Value.h>

enum OperandType {
  OT_Const,
  OT_Calc,
  OT_Load,
  OT_Store,
  OT_Alloc,
  OT_CallInst,
  OT_PHINode,
};

struct CalcOperand {
CalcOperand() : type(OT_Alloc), operand(nullptr), catype(nullptr) {}

CalcOperand(OperandType t, llvm::Value *v, CADataType *dt) : type(t), operand(v), catype(dt) {}

  OperandType type;
  CADataType *catype;
  llvm::Value *operand;
};

#endif

