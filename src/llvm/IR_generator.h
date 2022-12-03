#ifndef __IR_generator_h__
#define __IR_generator_h__

#include "ca_types.h"
#include "ca_parser.h"
#include "symtable.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DebugInfoMetadata.h"

#include <llvm/IR/Value.h>

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

CalcOperand(OperandType t, llvm::Value *v, CADataType *dt) : type(t), operand(v), catype(dt) {}

  OperandType type;
  CADataType *catype;
  llvm::Value *operand;
};

struct LexicalScope {
LexicalScope() : discope(nullptr), difn(nullptr) {}

  llvm::DIScope *discope;
  llvm::DISubprogram *difn;
};

struct LoopControlInfo {
  enum LoopType {
    LT_Loop,
    LT_While,
    LT_For,
  };

LoopControlInfo(LoopType looptype, int name, llvm::BasicBlock *condbb, llvm::BasicBlock *outbb) :
  looptype(looptype), name(name), condbb(condbb), outbb(outbb) {}

  LoopType looptype;
  int name;
  llvm::BasicBlock *condbb;
  llvm::BasicBlock *outbb;
};

#endif

