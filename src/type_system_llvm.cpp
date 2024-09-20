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

#include "ca_parser.h"

#include "type_system.h"
#include "type_system_llvm.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Value.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/ir1.h"

#include <unordered_map>
#include <map>

extern ir_codegen::IR1 ir1;

using namespace llvm;

/*
 * This data structure is used to store the type signature
 * to LLVM type map, aiming to shorten the generation of named
 * structs and speed up map performance.
 */
std::unordered_map<typeid_t, llvm::Type *> g_llvmtype_map;

std::vector<CALiteral> *arraylit_deref(CAArrayLit obj);

Value *tidy_value_with_arith(Value *v, int typetok) {
  if (typetok == F32)
    v = ir1.builder().CreateFPExt(v, ir1.float_type<double>());

  if (typetok == BOOL)
    v = ir1.builder().CreateZExt(v, ir1.int_type<int>());

  return v;
}

Type *llvmtype_from_token(int tok) {
  switch (tok) {
  case VOID:
    return ir1.void_type();
  case I16:
    return ir1.int_type<int16_t>();
  case I32:
    return ir1.int_type<int>();
  case I64:
    return ir1.int_type<int64_t>();
  case U16:
    return ir1.int_type<uint16_t>();
  case U32:
    return ir1.int_type<uint32_t>();
  case U64:
    return ir1.int_type<uint64_t>();
  case F32:
    return ir1.float_type<float>();
  case F64:
    return ir1.float_type<double>();
  case BOOL:
    return ir1.bool_type();
  case I8:
    return ir1.int_type<int8_t>();
  case U8:
    return ir1.int_type<uint8_t>();
  default:
    return nullptr;
  }
}

static Type *llvmtype_from_catype_inner(CADataType *catype, std::map<CADataType *, Type *> &rcheck) {
  switch (catype->type) {
  case POINTER: {
    // create llvm pointer type
    Type *kerneltype = nullptr;
    auto itr = rcheck.find(catype->pointer_layout->type);
    if (itr != rcheck.end())
      kerneltype = itr->second;
    else
      kerneltype = llvmtype_from_catype_inner(catype->pointer_layout->type, rcheck);

    if (!kerneltype) {
      fprintf(stderr, "create kernel type for pointer failed: %s\n",
	      symname_get(catype->pointer_layout->type->signature));
      return nullptr;
    }

    Type *ptrtype = kerneltype;
    int i = catype->pointer_layout->dimension;
    while(i-- > 0)
      ptrtype = PointerType::get(ptrtype, 0);

    return ptrtype;
  }
  case ARRAY: {
    /*
     * Create LLVM array type.
     * It seems there is no need for type checking here, as the array
     * kernel type should not be recursively defined, nor should
     * the size be unlimited. The type system analysis should
     * have already blocked such cases.
     */
    assert(rcheck.find(catype->pointer_layout->type) == rcheck.end());
    Type *kerneltype = llvmtype_from_catype_inner(catype->array_layout->type, rcheck);

    if (!kerneltype) {
      fprintf(stderr, "create kernel type for array failed: %s\n",
	      symname_get(catype->array_layout->type->signature));
      return nullptr;
    }

    Type *arrtype = kerneltype;
    int i = catype->array_layout->dimension;
    while (--i > -1)
      arrtype = ArrayType::get(arrtype, catype->array_layout->dimarray[i]);

    return arrtype;
  }
  case SLICE:
  case STRUCT: {
    // create llvm struct type
    size_t fieldnum = catype->struct_layout->fieldnum;
    std::vector<Type *> fields;
    StringRef name = symname_get(catype->formalname);
    bool pack = false;

    StructType *sttype = nullptr;
    auto itr = g_llvmtype_map.find(catype->signature);
    if (itr != g_llvmtype_map.end()) {
      sttype = static_cast<StructType *>(itr->second);
    } else {
      sttype = StructType::create(ir1.ctx(), name);
      g_llvmtype_map.insert(std::make_pair(catype->signature, static_cast<Type *>(sttype)));
    }

    rcheck.insert(std::make_pair(catype, sttype));
    for (int i = 0; i < catype->struct_layout->fieldnum; ++i) {
      Type *fieldtype = llvmtype_from_catype_inner(catype->struct_layout->fields[i].type, rcheck);
      fields.push_back(fieldtype);
    }
    rcheck.erase(catype);
    sttype->setBody(fields, pack);

    // following code generated unnamed struct, but not used yet
    //StructType *sttype = StructType::get(ir1.ctx(), fields, pack);
    return sttype;
  }
  case RANGE: {
    switch (catype->range_layout->type) {
    case FullRange:
      // should not come here
      //yyerror("should not come here for full range");
      //return nullptr;
      return ir1.int_type<int>();
    case InclusiveRange:
    case RightExclusiveRange:
    case InclusiveRangeTo:
    case RightExclusiveRangeTo:
    case RangeFrom:
      return llvmtype_from_catype_inner(catype->range_layout->range, rcheck);
    default:
      yyerror("bad range type: %d", catype->range_layout->type);
      return nullptr;
    }
  }
  default:
    return llvmtype_from_token(catype->type);
  }
}

Type *llvmtype_from_catype(CADataType *catype) {
  std::map<CADataType *, Type *> rcheck;
  Type *type = llvmtype_from_catype_inner(catype, rcheck);
  return type;
}

static Value *gen_primitive_literal_value(CALiteral *value, tokenid_t typetok, SLoc *loc) {
  // check if literal value type matches the given typetok, if not match, report error
  if (!value->fixed_type && !can_type_binding(value, typetok)) {
    caerror(loc, NULL, "literal value type '%s' not match the variable type '%s'",
	    get_type_string(typetok), get_type_string(typetok));
    return nullptr;
  }

  switch (typetok) {
  case VOID:
    caerror(loc, NULL, "void type have no literal value");
    return nullptr;
  case I16:
    return ir1.gen_int((int16_t)parse_to_int64(value));
  case I32:
    return ir1.gen_int((int)parse_to_int64(value));
  case I64:
    return ir1.gen_int(parse_to_int64(value));
  case U16:
    return ir1.gen_int((uint16_t)parse_to_int64(value));
  case U32:
    return ir1.gen_int((uint32_t)parse_to_int64(value));
  case U64:
    return ir1.gen_int((uint64_t)parse_to_int64(value));
  case F32:
    return ir1.gen_float((float)parse_to_double(value));
  case F64:
    return ir1.gen_float(parse_to_double(value));
  case BOOL:
    return ir1.gen_bool(!!parse_to_int64(value));
  case I8:
    return ir1.gen_int((char)parse_to_int64(value));
  case U8:
    return ir1.gen_int((uint8_t)parse_to_int64(value));
  default:
    return nullptr;
  }
}

Value *gen_zero_literal_value(CADataType *catype) {
  // TODO: implement it
  return nullptr;
}

Value *gen_array_literal_value(CALiteral *lit, CADataType *catype, SLoc loc) {
  // using expand formalized catype object
  if(catype->array_layout->dimension != 1) {
    yyerror("(internal) the array dimension is not normalized into 1");
    return nullptr;
  }
  assert(lit->littypetok == ARRAY);
  int len = catype->array_layout->dimarray[0];
  std::vector<CALiteral> *lits = arraylit_deref(lit->u.arrayvalue);
  assert(len == lits->size());

  std::vector<Constant *> subvalues;
  CADataType *subtype = catype->array_layout->type;
  for (int i = 0; i < len; ++i) {
    CALiteral *sublit = &lits->at(i);
    Value *subvalue = gen_literal_value(sublit, subtype, loc);
    subvalues.push_back((Constant *)subvalue);
  }

  Type *arraytype = llvmtype_from_catype(catype);
  Constant *arrayconst = ConstantArray::get((ArrayType *)arraytype, subvalues);
  //GlobalValue *g = ir1.gen_global_var(arraytype, "constarray", arrayconst, true);

  return arrayconst;
}

Value *gen_literal_value(CALiteral *lit, CADataType *catype, SLoc loc) {
  /*
   * TODO: Use the type which coming from the symbol table as well,
   * when literals support arrays or structs, e.g., AA {d, b}.
   */
  Type *type = nullptr;
  Value *llvmvalue = nullptr;
  switch(catype->type) {
  case POINTER:
    if (lit->littypetok == CSTRING) {
      // create string literal
      const char *data = symname_get(lit->u.strvalue.text);
      llvm::StringRef strref(data, lit->u.strvalue.len);
      Constant *llvmconststr = ir1.get_global_string(strref.str());
      return llvmconststr;
    }

    type = llvmtype_from_catype(catype);

    if (lit->u.i64value == 0) {
      llvmvalue = ConstantPointerNull::get(static_cast<PointerType *>(type));

      // TODO: for other pointer literal value should not supported
      //yyerror("other pointer literal not implemented yet");
      //return nullptr;
    } else {
      llvmvalue = ir1.gen_int<int64_t>(lit->u.i64value);
      //llvmvalue = ir1.builder().CreatePointerCast(llvmvalue, type);
      llvmvalue = ir1.builder().CreateIntToPtr(llvmvalue, type);
    }

    return llvmvalue;
  case ARRAY:
    return gen_array_literal_value(lit, catype, loc);
  case SLICE:
    yyerror("the slice literal not implemented yet");
    return nullptr;
  case STRUCT:
    //ConstantStruct::get();
    // TODO:
    yyerror("the struct literal not implemented yet");
    return nullptr;
  default:
    return gen_primitive_literal_value(lit, catype->type, &loc);
  }
}

/**
 * row: VOID I16 I32 I64 U16 U32 U64 F32 F64 BOOL I8 U8 ATOMTYPE_END STRUCT ARRAY POINTER
 * col: < > GE LE NE EQ
 */
CmpInst::Predicate s_cmp_predicate[ATOMTYPE_END-VOID][6] = {
  {CmpInst::FCMP_FALSE, CmpInst::FCMP_FALSE, CmpInst::FCMP_FALSE, CmpInst::FCMP_FALSE, CmpInst::FCMP_FALSE, CmpInst::FCMP_FALSE}, // VOID
  {CmpInst::ICMP_SLT, CmpInst::ICMP_SGT, CmpInst::ICMP_SGE, CmpInst::ICMP_SLE, CmpInst::ICMP_NE, CmpInst::ICMP_EQ}, // I16
  {CmpInst::ICMP_SLT, CmpInst::ICMP_SGT, CmpInst::ICMP_SGE, CmpInst::ICMP_SLE, CmpInst::ICMP_NE, CmpInst::ICMP_EQ}, // I32
  {CmpInst::ICMP_SLT, CmpInst::ICMP_SGT, CmpInst::ICMP_SGE, CmpInst::ICMP_SLE, CmpInst::ICMP_NE, CmpInst::ICMP_EQ}, // I64
  {CmpInst::ICMP_ULT, CmpInst::ICMP_UGT, CmpInst::ICMP_UGE, CmpInst::ICMP_ULE, CmpInst::ICMP_NE, CmpInst::ICMP_EQ}, // U16
  {CmpInst::ICMP_ULT, CmpInst::ICMP_UGT, CmpInst::ICMP_UGE, CmpInst::ICMP_ULE, CmpInst::ICMP_NE, CmpInst::ICMP_EQ}, // U32
  {CmpInst::ICMP_ULT, CmpInst::ICMP_UGT, CmpInst::ICMP_UGE, CmpInst::ICMP_ULE, CmpInst::ICMP_NE, CmpInst::ICMP_EQ}, // U64
  {CmpInst::FCMP_OLT, CmpInst::FCMP_OGT, CmpInst::FCMP_OGE, CmpInst::FCMP_OLE, CmpInst::FCMP_ONE, CmpInst::FCMP_OEQ}, // F32
  {CmpInst::FCMP_OLT, CmpInst::FCMP_OGT, CmpInst::FCMP_OGE, CmpInst::FCMP_OLE, CmpInst::FCMP_ONE, CmpInst::FCMP_OEQ}, // F64
  {CmpInst::ICMP_ULT, CmpInst::ICMP_UGT, CmpInst::ICMP_UGE, CmpInst::ICMP_ULE, CmpInst::ICMP_NE, CmpInst::ICMP_EQ}, // BOOL
  {CmpInst::ICMP_SLT, CmpInst::ICMP_SGT, CmpInst::ICMP_SGE, CmpInst::ICMP_SLE, CmpInst::ICMP_NE, CmpInst::ICMP_EQ}, // I8
  {CmpInst::ICMP_ULT, CmpInst::ICMP_UGT, CmpInst::ICMP_UGE, CmpInst::ICMP_ULE, CmpInst::ICMP_NE, CmpInst::ICMP_EQ}, // U8
  //  {CmpInst::FCMP_FALSE, CmpInst::FCMP_FALSE, CmpInst::FCMP_FALSE, CmpInst::FCMP_FALSE, CmpInst::FCMP_FALSE, CmpInst::FCMP_FALSE}, // STRUCT
};

static std::pair<int, const char *> cmp_op_index(int op) {
  switch(op) {
  case '<': return std::make_pair(0, "lt");
  case '>': return std::make_pair(1, "gt");
  case GE: return std::make_pair(2, "ge");
  case LE: return std::make_pair(3, "le");
  case NE: return std::make_pair(4, "ne");
  case EQ: return std::make_pair(5, "eq");
  default: return std::make_pair(0x7fffffff, "unknown");
  }
}

Value *generate_cmp_op(int typetok, Value *v1, Value *v2, int op) {
  auto pair = cmp_op_index(op);
  CmpInst::Predicate pred = s_cmp_predicate[typetok - VOID][pair.first];
  return ir1.builder().CreateCmp(pred, v1, v2, pair.second);
}

Value *create_default_integer_value(tokenid_t typetok, int64_t defv) {
  switch(typetok) {
  case I16:
    return ir1.gen_int<int16_t>(defv);
    break;
  case I32:
    return ir1.gen_int<int>(defv);
    break;
  case I64:
    return ir1.gen_int<int64_t>(defv);
    break;
  case U16:
    return ir1.gen_int<uint16_t>(defv);
    break;
  case U32:
    return ir1.gen_int<uint32_t>(defv);
    break;
  case U64:
    return ir1.gen_int<uint64_t>(defv);
    break;
  case I8:
    return ir1.gen_int<int8_t>(defv);
    break;
  case U8:
    return ir1.gen_int<uint8_t>(defv);
    break;
  case BOOL:
    return ir1.gen_bool(!defv);
    break;
  case VOID: {
    //Type *voidty = Type::getVoidTy(ir1.ctx());
    //return Constant::getNullValue(voidty);
    return nullptr;
    break;
  }
  default:
    yyerror("(internal) return type `%s` not implemented", get_type_string(typetok));
    return nullptr;
    break;
  }
}

/**
 * The table is used for value conversion expression in `as`
 * VOID I16 I32 I64 U16 U32 U64 F32 F64 BOOL I8 U8 ATOMTYPE_END STRUCT ARRAY POINTER CSTRING
 * Trunc ZExt SExt FPToUI FPToSI UIToFP SIToFP FPTrunc FPExt PtrToInt IntToPtr BitCast AddrSpaceCast
 * CastOpsBegin stand for no need convert, CastOpsEnd stand for cannot convert
 */
static Instruction::CastOps
llvmtype_cast_table[CSTRING - VOID + 1][CSTRING - VOID + 1] = {
  { // Begin VOID
    (ICO)0,            /* VOID */
    (ICO)-1,           /* I16 */
    (ICO)-1,           /* I32 */
    (ICO)-1,           /* I64 */
    (ICO)-1,           /* U16 */
    (ICO)-1,           /* U32 */
    (ICO)-1,           /* U64 */
    (ICO)-1,           /* F32 */
    (ICO)-1,           /* F64 */
    (ICO)-1,           /* BOOL */
    (ICO)-1,           /* I8 */
    (ICO)-1,           /* U8 */
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    (ICO)-1,           /* POINTER */
    (ICO)-1,           /* CSTRING */
  },                   // VOID -> ?
  { // Begin I16
    (ICO)-1,           /* VOID */
    (ICO)0,            /* I16 */
    ICO::SExt,         /* I32 */
    ICO::SExt,         /* I64 */
    ICO::BitCast,      /* U16 */
    ICO::SExt,         /* U32 */
    ICO::SExt,         /* U64 */
    ICO::SIToFP,       /* F32 */
    ICO::SIToFP,       /* F64 */
    (ICO)-1,           /* BOOL */
    ICO::Trunc,        /* I8 */
    ICO::Trunc,        /* U8 */
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    ICO::IntToPtr,     /* POINTER */
    (ICO)-1,           /* CSTRING */
  },                   // I16 ->
  { // Begin I32
    (ICO)-1,           /* VOID */
    ICO::Trunc,        /* I16 */
    (ICO)0,            /* I32 */
    ICO::SExt,         /* I64 */
    ICO::Trunc,        /* U16 */
    ICO::BitCast,      /* U32 */
    ICO::SExt,         /* U64 */
    ICO::SIToFP,       /* F32 */
    ICO::SIToFP,       /* F64 */
    (ICO)-1,           /* BOOL */
    ICO::Trunc,        /* I8 */
    ICO::Trunc,        /* U8 */
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    ICO::IntToPtr,     /* POINTER */
    (ICO)-1,           /* CSTRING */
  },                   // I32 ->
  { // Begin I64
    (ICO)-1,           /* VOID */
    ICO::Trunc,	       /* I16 */
    ICO::Trunc,	       /* I32 */
    (ICO)0,            /* I64 */
    ICO::Trunc,	       /* U16 */
    ICO::Trunc,	       /* U32 */
    ICO::BitCast,      /* U64 */
    ICO::SIToFP,       /* F32 */
    ICO::SIToFP,       /* F64 */
    (ICO)-1,           /* BOOL */
    ICO::Trunc,	       /* I8 */
    ICO::Trunc,	       /* U8 */
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    ICO::IntToPtr,     /* POINTER */
    (ICO)-1,           /* CSTRING */
  },                   // I64 ->
  { // Begin U16
    (ICO)-1,           /* VOID */
    ICO::BitCast,      /* I16 */
    ICO::ZExt,         /* I32 */
    ICO::ZExt,	       /* I64 */
    (ICO)0,            /* U16 */
    ICO::ZExt,         /* U32 */
    ICO::ZExt,	       /* U64 */
    ICO::UIToFP,       /* F32 */
    ICO::UIToFP,       /* F64 */
    (ICO)-1,           /* BOOL */
    ICO::Trunc,	       /* I8 */
    ICO::Trunc,	       /* U8 */
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    ICO::IntToPtr,     /* POINTER */
    (ICO)-1,           /* CSTRING */
  },                   // U16 ->
  { // Begin U32
    (ICO)-1,           /* VOID */
    ICO::Trunc,        /* I16 */
    ICO::BitCast,      /* I32 */
    ICO::ZExt,	       /* I64 */
    ICO::Trunc,        /* U16 */
    (ICO)0,            /* U32 */
    ICO::ZExt,	       /* U64 */
    ICO::UIToFP,       /* F32 */
    ICO::UIToFP,       /* F64 */
    (ICO)-1,           /* BOOL */
    ICO::Trunc,	       /* I8 */
    ICO::Trunc,	       /* U8 */
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    ICO::IntToPtr,     /* POINTER */
    (ICO)-1,           /* CSTRING */
  },                   // U32 ->
  { // Begin U64
    (ICO)-1,           /* VOID */
    ICO::Trunc,	       /* I16 */
    ICO::Trunc,	       /* I32 */
    ICO::BitCast,      /* I64 */
    ICO::Trunc,	       /* U16 */
    ICO::Trunc,	       /* U32 */
    (ICO)0,            /* U64 */
    ICO::UIToFP,       /* F32 */
    ICO::UIToFP,       /* F64 */
    (ICO)-1,           /* BOOL */
    ICO::Trunc,	       /* I8 */
    ICO::Trunc,	       /* U8 */
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    ICO::IntToPtr,     /* POINTER */
    (ICO)-1,           /* CSTRING */
  },                   // U64 ->
  { // Begin F32
    (ICO)-1,           /* VOID */
    ICO::FPToSI,       /* I16 */
    ICO::FPToSI,       /* I32 */
    ICO::FPToSI,       /* I64 */
    ICO::FPToUI,       /* U16 */
    ICO::FPToUI,       /* U32 */
    ICO::FPToUI,       /* U64 */
    (ICO)0,            /* F32 */
    ICO::FPExt,	       /* F64 */
    (ICO)-1,           /* BOOL */
    ICO::FPToSI,       /* I8 */
    ICO::FPToUI,       /* U8 */
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    (ICO)-1,           /* POINTER */
    (ICO)-1,           /* CSTRING */
  },                   // F32 ->
  { // Begin F64
    (ICO)-1,           /* VOID */
    ICO::FPToSI,       /* I16 */
    ICO::FPToSI,       /* I32 */
    ICO::FPToSI,       /* I64 */
    ICO::FPToUI,       /* U16 */
    ICO::FPToUI,       /* U32 */
    ICO::FPToUI,       /* U64 */
    ICO::FPTrunc,      /* F32 */
    (ICO)0,            /* F64 */
    (ICO)-1,           /* BOOL */
    ICO::FPToSI,       /* I8 */
    ICO::FPToUI,       /* U8 */
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    (ICO)-1,           /* POINTER */
    (ICO)-1,           /* CSTRING */
  },                   // F64 ->
  { // Begin BOOL
    (ICO)-1,           /* VOID */
    ICO::ZExt,	       /* I16 */
    ICO::ZExt,	       /* I32 */
    ICO::ZExt,	       /* I64 */
    ICO::ZExt,	       /* U16 */
    ICO::ZExt,	       /* U32 */
    ICO::ZExt,	       /* U64 */
    (ICO)-1,           /* F32 */
    (ICO)-1,           /* F64 */
    (ICO)0,            /* BOOL */
    ICO::ZExt,	       /* I8 */
    ICO::ZExt,	       /* U8 */
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    (ICO)-1,           /* POINTER */
    (ICO)-1,           /* CSTRING */
  },                   // BOOL ->
  { // Begin I8
    (ICO)-1,           /* VOID */
    ICO::SExt,	       /* I16 */
    ICO::SExt,	       /* I32 */
    ICO::SExt,	       /* I64 */
    ICO::SExt,	       /* U16 */
    ICO::SExt,	       /* U32 */
    ICO::SExt,	       /* U64 */
    ICO::SIToFP,       /* F32 */
    ICO::SIToFP,       /* F64 */
    (ICO)-1 ,          /* BOOL */
    (ICO)0,            /* I8 */
    ICO::BitCast,      /* U8 */
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    (ICO)-1,           /* POINTER */
    (ICO)-1,           /* CSTRING */
  },                   // I8 ->
  { // Begin U8
    (ICO)-1,           /* VOID */
    ICO::ZExt,	       /* I16 */
    ICO::ZExt,	       /* I32 */
    ICO::ZExt,	       /* I64 */
    ICO::ZExt,	       /* U16 */
    ICO::ZExt,	       /* U32 */
    ICO::ZExt,	       /* U64 */
    ICO::UIToFP,       /* F32 */
    ICO::UIToFP,       /* F64 */
    (ICO)-1,           /* BOOL */
    ICO::BitCast,      /* I8 */
    (ICO)0,            /* U8 */
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    (ICO)-1,           /* POINTER */
    (ICO)-1,           /* CSTRING */
  },                   // U8 ->
  { // Begin ATOMTYPE_END
    (ICO)-1,           /* VOID */
    (ICO)-1,           /* I16 */
    (ICO)-1,           /* I32 */
    (ICO)-1,           /* I64 */
    (ICO)-1,           /* U16 */
    (ICO)-1,           /* U32 */
    (ICO)-1,           /* U64 */
    (ICO)-1,           /* F32 */
    (ICO)-1,           /* F64 */
    (ICO)-1,           /* BOOL */
    (ICO)-1,           /* I8 */
    (ICO)-1,           /* U8 */
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    (ICO)-1,           /* POINTER */
    (ICO)-1,           /* CSTRING */
  },                   // ATOMTYPE_END -> ?
  { // Begin STRUCT
    (ICO)-1,           /* VOID */
    (ICO)-1,           /* I16 */
    (ICO)-1,           /* I32 */
    (ICO)-1,           /* I64 */
    (ICO)-1,           /* U16 */
    (ICO)-1,           /* U32 */
    (ICO)-1,           /* U64 */
    (ICO)-1,           /* F32 */
    (ICO)-1,           /* F64 */
    (ICO)-1,           /* BOOL */
    (ICO)-1,           /* I8 */
    (ICO)-1,           /* U8 */
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    (ICO)-1,           /* POINTER */
    (ICO)-1,           /* CSTRING */
  },                   // STRUCT -> ?
  { // Begin ARRAY
    (ICO)-1,           /* VOID */
    (ICO)-1,           /* I16 */
    (ICO)-1,           /* I32 */
    (ICO)-1,           /* I64 */
    (ICO)-1,           /* U16 */
    (ICO)-1,           /* U32 */
    (ICO)-1,           /* U64 */
    (ICO)-1,           /* F32 */
    (ICO)-1,           /* F64 */
    (ICO)-1,           /* BOOL */
    (ICO)-1,           /* I8 */
    (ICO)-1,           /* U8 */
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    (ICO)0,            /* POINTER */
    (ICO)0,            /* CSTRING */
  },                   // ARRAY -> ?
  { // Begin POINTER
    (ICO)-1,           /* VOID */
    ICO::PtrToInt,     /* I16 */
    ICO::PtrToInt,     /* I32 */
    ICO::PtrToInt,     /* I64 */
    ICO::PtrToInt,     /* U16 */
    ICO::PtrToInt,     /* U32 */
    ICO::PtrToInt,     /* U64 */
    (ICO)-1,           /* F32 */
    (ICO)-1,           /* F64 */
    (ICO)-1,           /* BOOL */
    (ICO)-1,           /* I8 */
    (ICO)-1,           /* U8 */
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    ICO::BitCast,      /* POINTER */
    ICO::BitCast,      /* CSTRING */
  },                   // POINTER -> ?
  { // Begin CSTRING
    (ICO)-1,           /* VOID */
    (ICO)-1,           /* I16 */
    (ICO)-1,           /* I32 */
    (ICO)-1,           /* I64 */
    (ICO)-1,           /* U16 */
    (ICO)-1,           /* U32 */
    (ICO)-1,           /* U64 */
    (ICO)-1,           /* F32 */
    (ICO)-1,           /* F64 */
    (ICO)-1,           /* BOOL */
    (ICO)-1,           /* I8 */
    (ICO)-1,           /* U8 */
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    ICO::BitCast,      /* POINTER */
    ICO::BitCast,      /* CSTRING */
  },                   // CSTRING -> ?

#if 0
  { // Begin STRUCT
    (ICO)-1,           /* VOID */
    (ICO)-1,           /* I16 */
    (ICO)-1,           /* I32 */
    (ICO)-1,           /* I64 */
    (ICO)-1,           /* U16 */
    (ICO)-1,           /* U32 */
    (ICO)-1,           /* U64 */
    (ICO)-1,           /* F32 */
    (ICO)-1,           /* F64 */
    (ICO)-1,           /* BOOL */
    (ICO)-1,           /* I8 */
    (ICO)-1,           /* U8 */
    (ICO)-1,           /* STRUCT */
  },                   // STRUCT ->
#endif
};

Instruction::CastOps gen_cast_ops(CADataType *fromtype, CADataType *totype) {
  if (fromtype->signature == totype->signature)
    return (Instruction::CastOps)0;

  tokenid_t fromtok = fromtype->type;
  tokenid_t totok = totype->type;
  return llvmtype_cast_table[fromtok-VOID][totok-VOID];
}

Instruction::CastOps gen_cast_ops_token(tokenid_t fromtok, tokenid_t totok) {
  return llvmtype_cast_table[fromtok-VOID][totok-VOID];
}

