#include "ca.h"

#include "symtable.h"
#include "type_system.h"
#include "type_system_llvm.h"

#include "llvm/ir1.h"
#include <cstdint>
#include <cstdio>

#ifdef __cplusplus
BEGIN_EXTERN_C
#endif

#include "ca.tab.h"

void yyerror(const char *s, ...);

#ifdef __cplusplus
END_EXTERN_C
#endif

#include <unordered_map>

#include <stdint.h>
#include <stdlib.h>

extern int glineno;
extern int gcolno;
extern ir_codegen::IR1 ir1;

using namespace llvm;
std::unordered_map<std::string, int> s_token_primitive_map {
  {"void",   VOID},
  {"int",    I32},
  {"i32",    I32},
  {"i64",    I64},
  {"uint",   U32},
  {"u32",    U32},
  {"u64",    U64},
  {"float",  F32},
  {"f32",    F32},
  {"double", F64},
  {"f64",    F64},
  {"bool",   BOOL},
  {"i8",     CHAR},
  {"char",   CHAR},
  {"u8",     UCHAR},
  {"uchar",  UCHAR},
};

BEGIN_EXTERN_C

enum TypeType {
  TT_Primitive,
  TT_Alias,
  TT_Array,
  TT_Pointer,
  TT_Struct,
  TT_Enum,
  TT_Ref,
  TT_Trait,
  TT_Composite,
};

struct TypeDB {
  const char *name;
  int symname;
  int typeindex;
  int size;
  int typetok;
  TypeType tt;
};

TypeDB g_typedb[] = {
  {"void",  0, 0, 0, VOID, TT_Primitive},
  {"i32",   0, 1, 4, I32,  TT_Primitive},
  {"int",   0, 1, 4, I32,  TT_Alias},
  {"i64",   0, 2, 8, I64,  TT_Primitive},
  {"u32",   0, 3, 4, U32,  TT_Primitive},
  {"uint",  0, 3, 4, U32,  TT_Alias},
  {"u64",   0, 4, 8, U64,  TT_Primitive},
  {"f32",   0, 5, 4, F32,  TT_Primitive},
  {"float", 0, 5, 4, F32,  TT_Alias},
  {"f64",   0, 6, 8, F64,  TT_Primitive},
  {"double",0, 6, 8, F64,  TT_Alias},
  {"bool",  0, 7, 1, BOOL, TT_Primitive},
  {"i8",    0, 8, 1, CHAR, TT_Primitive},
  {"char",  0, 8, 1, CHAR, TT_Alias},
  {"u8",    0, 9, 1, UCHAR,TT_Primitive},
  {"uchar", 0, 9, 1, UCHAR,TT_Alias},
};

const char *get_type_string(int tok) {
  switch (tok) {
  case VOID:
    return "void";
  case I32:
    return "i32";
  case I64:
    return "i64";
  case U32:
    return "u32";
  case U64:
    return "u64";
  case F32:
    return "f32";
  case F64:
    return "f64";
  case BOOL:
    return "bool";
  case CHAR:
    return "char";
  case UCHAR:
    return "uchar";
  default:
    yyerror("bad type token: %d", tok);
    return nullptr;
  }
}

int sym_form_type_id_from_token(int tok) {
  char namebuf[16];
  const char *name = get_type_string(tok);
  sprintf(namebuf, "t:%s", name);
  return symname_check(namebuf);
}

int sym_primitive_token_from_id(int id) {
  const char *name = symname_get(id);
  auto itr = s_token_primitive_map.find(name);
  if (itr != s_token_primitive_map.end())
    return itr->second;

  yyerror("line: %d, col: %d: get primitive type token failed", glineno, gcolno);
  return -1;
}

END_EXTERN_C

std::unordered_map<std::string, int> s_token_map = {
#if 0
  {"void",   VOID},
  {"int",    I32},
  {"i32",    I32},
  {"i64",    I64},
  {"uint",   U32},
  {"u32",    U32},
  {"u64",    U64},
  {"float",  F32},
  {"f32",    F32},
  {"double", F64},
  {"f64",    F64},
  {"bool",   BOOL},
  {"i8",     CHAR},
  {"char",   CHAR},
  {"u8",     UCHAR},
  {"uchar",  UCHAR},
#endif

  {">=",     GE},
  {"<=",     LE},
  {"==",     EQ},
  {"!=",     NE},
  {"while",  WHILE},
  {"if",     IF},
  {"ife",    IFE},
  {"else",   ELSE},
  {"print",  PRINT},
  {"goto",   GOTO},
  {"fn",     FN},
  {"extern", EXTERN},
  {"return", RET},
  {"let",    LET},
  {"...",    VARG},
  {"struct", STRUCT},
  {"type",   TYPE},
  {"as",     AS},
};

static CADataType *catype_make_type(const char *name, int type, int size);
int catype_init() {
  CADataType *datatype;
  int name;
  datatype = catype_make_type("t:void", VOID, 0); // void
  datatype = catype_make_type("t:i32", I32, 4); // i32

  name = symname_check_insert("t:int");
  catype_put_by_name(name, datatype);         // int

  catype_make_type("t:i64", I64, 8);            // i64

  datatype = catype_make_type("t:u32", U32, 4); // u32

  name = symname_check_insert("t:uint");
  catype_put_by_name(name, datatype);         // uint

  catype_make_type("t:u64", U64, 8);            // u64

  datatype = catype_make_type("t:f32", F32, 4); // f32
  name = symname_check_insert("t:float");
  catype_put_by_name(name, datatype);         // float

  datatype = catype_make_type("t:f64", F64, 8); // f64
  name = symname_check_insert("t:double");
  catype_put_by_name(name, datatype);         // double

  catype_make_type("t:bool", BOOL, 1);          // bool
  catype_make_type("t:char", CHAR, 1);          // char
  catype_make_type("t:uchar", UCHAR, 1);        // uchar

  return 0;
}

BEGIN_EXTERN_C
// inference and set the literal type for the literal, when the literal have no
// a determined type, different from `determine_literal_type`, the later is used by passing a defined type  
int inference_literal_type(CALiteral *lit) {
  if (lit->fixed_type) {
    // no need inference, should report an error
    return lit->datatype->type;
  }

  const char *text = symname_get(lit->textid);
  int littypetok = lit->littypetok;
  int badscope = 0;
  int intentdeftype = 0;

  // handle non-fixed type literal value
  switch (littypetok) {
  case I64:
    intentdeftype = I32;
    lit->u.i64value = atoll(text);
    badscope = check_i64_value_scope(lit->u.i64value, I32);
    break;
  case U64:
    intentdeftype = I32;
    sscanf(text, "%lu", &lit->u.i64value);
    badscope = check_u64_value_scope((uint64_t)lit->u.i64value, I32);
    break;
  case F64:
    intentdeftype = F64;
    badscope = check_f64_value_scope(lit->u.f64value, F64);
    lit->u.f64value = atof(text);
    break;
  case BOOL:
    intentdeftype = BOOL;
    lit->u.i64value = atoll(text) ? 1 : 0;
    break;
  case CHAR:
    intentdeftype = CHAR;
    lit->u.i64value = (char)parse_lexical_char(text);
    badscope = check_char_value_scope(lit->u.i64value, CHAR);
    break;
  case UCHAR:
    intentdeftype = UCHAR;
    lit->u.i64value = (uint8_t)parse_lexical_char(text);
    badscope = check_uchar_value_scope(lit->u.i64value, UCHAR);
    break;
  default:
    yyerror("line: %d, col: %d: void type have no literal value", glineno, gcolno);
    return 0;
  }

  if (badscope) {
    yyerror("line: %d, col: %d: bad literal value definition: %s cannot be %s",
	    glineno, gcolno, get_type_string(littypetok), get_type_string(intentdeftype));
    return 0;
  }

  //const char *name = get_type_string(intentdeftype);
  int typesym = sym_form_type_id_from_token(intentdeftype);
  lit->datatype = catype_get_by_name(typesym // symname_check(name)
				     );
  lit->fixed_type = 1;

  return intentdeftype;
}

// determine and set the literal type for the literal for a specified type,
// different from `inference_literal_type` which have no a defined type
// parameter
void determine_literal_type(CALiteral *lit, int typetok) {
  if (!typetok || typetok == VOID)
    return;

  int littypetok = lit->littypetok;

  // check convertable
  if (!literal_type_convertable(littypetok, typetok)) {
    yyerror("line: %d, col: %d: bad literal value definition: %s cannot be %s",
	    glineno, gcolno,
	    get_type_string(littypetok), get_type_string(typetok));
    return;
  }

  const char *text = symname_get(lit->textid);

  int badscope = 0;

  // check literal value scope
  switch (littypetok) {
  case I64: // I64 stand for positive integer value in lexical
    lit->u.i64value = atoll(text);
    badscope = check_i64_value_scope(lit->u.i64value, typetok);
    break;
  case U64:
    sscanf(text, "%lu", &lit->u.i64value);
    badscope = check_u64_value_scope((uint64_t)lit->u.i64value, typetok);
    break;
  case F64:
    lit->u.f64value = atof(text);
    badscope = check_f64_value_scope(lit->u.f64value, typetok);
    break;
  case BOOL:
    lit->u.i64value = atoll(text) ? 1 : 0;
    badscope = (typetok != BOOL);
    break;
  case CHAR:
    lit->u.i64value = (char)parse_lexical_char(text);
    badscope = check_char_value_scope(lit->u.i64value, typetok);
    break;
  case UCHAR:
    lit->u.i64value = (uint8_t)parse_lexical_char(text);
    badscope = check_uchar_value_scope(lit->u.i64value, typetok);
    break;
  default:
    yyerror("line: %d, col: %d: %s type have no lexical value",
	    glineno, gcolno, get_type_string(littypetok));
    break;
  }

  if (badscope) {
    yyerror("line: %d, col: %d: bad literal value definition: %s cannot be %s",
	    glineno, gcolno, get_type_string(littypetok), get_type_string(typetok));
    return;
  }

  //const char *name = get_type_string(typetok);
  int typesym = sym_form_type_id_from_token(typetok);
  lit->datatype = catype_get_by_name(typesym // symname_check(name)
				     );
  lit->fixed_type = 1;
}

CADataType *catype_clone_thin(const CADataType *type) {
  auto dt = new CADataType;
  dt->formalname = type->formalname;
  dt->type = type->type;
  dt->size = type->size;
  dt->signature = type->signature;
  switch (type->type) {
  case POINTER:
    dt->pointer_layout = new CAPointer;
    dt->pointer_layout->type = type->pointer_layout->type;
    dt->pointer_layout->dimension = type->pointer_layout->dimension;
    break;
  case STRUCT:
    // TODO: 
    dt->struct_layout = type->struct_layout;
    break;
  case ARRAY:
    dt->array_layout = new CAArray;
    dt->array_layout->type = type->array_layout->type;
    dt->array_layout->dimension = type->array_layout->dimension;
    for (int i = 0; i < dt->array_layout->dimension; ++i)
      dt->array_layout->dimarray[i] = type->array_layout->dimarray[i];

    break;
  default:
    dt->array_layout = nullptr;
    break;
  }

  return dt;
}

CADataType *catype_make_type_symname(int name, int type, int size) {
  auto dt = new CADataType;
  dt->formalname = name;
  dt->type = type;
  dt->size = size;
  dt->signature = name;
  dt->struct_layout = nullptr;
  catype_put_by_name(name, dt);
  return dt;
}

CADataType *catype_make_unknown_type(int name, int size) {
  CADataType *dt = catype_make_type_symname(name, TYPE_UNKNOWN, size);
  
}

// type + '*'
// i32 + '*' => *i32, *type + '*' => **type, [type;n] + '*' => *[type;n]
// [[[type1;n1];n2];n3] + '*' => *[[[type1;n1];n2];n3]
// [*i32;n], [**i32;n], *[*i32;n], [*[*i32;n1];n2], *[[*i32;n1];n2]
static int form_datatype_signature(CADataType *type, int plus, uint64_t len) {
  char buf[1024];
  const char *name = symname_get(type->signature);
  switch (plus) {
  case '*':
    // pointer
    buf[0] = '*';
    strcpy(buf + 1, name);
    break;
  case '[':
    // array
    sprintf(buf, "[%s;%lu]", name, len);
    break;
  case 's':
    // structure signature
  default:
    yyerror("bad signature input");
    return 0;
  }

  return symname_check_insert(buf);
}

CADataType *catype_make_pointer_type(CADataType *datatype) {
  int signature = form_datatype_signature(datatype, '*', 0);
  CADataType *type = catype_get_by_name(signature);
  if (type)
    return type;

  // create new CADataType object here and put it into datatype table
  switch (datatype->type) {
  case POINTER:
    // make pointer's pointer
    type = catype_clone_thin(datatype);
    type->formalname = signature;
    type->signature = signature;
    type->pointer_layout->dimension++;
    catype_put_by_name(signature, type);
    break;
  case ARRAY:
    // array's pointer
  case STRUCT:
    // structure's pointer
  default:
    // array and struct type can directly append the signature
    type = catype_make_type_symname(signature, POINTER, sizeof(void *));
    type->pointer_layout = new CAPointer;
    type->pointer_layout->type = datatype;
    type->pointer_layout->dimension = 1;
    break;
  }

  return type;
}

// handle signature for structure, use current symbol table name@address as the
// signature test pointer type parsing, realize array type, struct type:
// [[[i32;2];3];4] <==> [i32;2;3;4], [[*[i32;2];3];4] <==> [*[i32;2];3;4]
// the array representation using the later which is the compact one
CADataType *catype_make_array_type(CADataType *datatype, uint64_t len) {
  int signature = form_datatype_signature(datatype, '[', len);
  CADataType *dt = catype_get_by_name(signature);
  if (dt)
    return dt;

  // create new CADataType object here and put it into datatype table
  switch (datatype->type) {
  case ARRAY:
    // array and struct type can directly append the signature
    dt = catype_clone_thin(datatype);
    dt->size = len * datatype->size;
    dt->formalname = signature;
    dt->signature = signature;
    dt->array_layout->dimarray[dt->array_layout->dimension++] = len;
    catype_put_by_name(signature, dt);
    break;
  case POINTER:
  case STRUCT:
  default:
    dt = catype_make_type_symname(signature, ARRAY, len * datatype->size);
    dt->array_layout = new CAArray;
    dt->array_layout->type = datatype;
    dt->array_layout->dimension = 1;
    dt->array_layout->dimarray[0] = len;
    break;
  }

  return dt;
}

CADataType *catype_make_struct_type(int symname, ST_ArgList *arglist) {
  CADataType *dt = catype_make_type_symname(symname, STRUCT, 0);
  dt->struct_layout = new CAStruct;
  dt->struct_layout->fieldnum = arglist->argc;
  dt->struct_layout->fields = new CAStructField[arglist->argc];

  for (int i = 0; i < arglist->argc; ++i) {
    STEntry *entry = sym_getsym(arglist->symtable, arglist->argnames[i], 0);
    if (!entry) {
      yyerror("line: %d, col: %d: can not find entry for args `%s`",
	      glineno, gcolno, symname_get(arglist->argnames[i]));
      return NULL;
    }

    CAVariable *cav = entry->u.var;

    dt->size += cav->datatype->size;
    dt->struct_layout->fields[i].name = cav->name;
    dt->struct_layout->fields[i].type = cav->datatype;
  }

  return dt;
}

END_EXTERN_C

Value *tidy_value_with_arith(Value *v, int typetok) {
  if (typetok == F32)
    v = ir1.builder().CreateFPExt(v, ir1.float_type<double>());

  return v;
}

Type *gen_type_from_token(int tok) {
  switch (tok) {
  case VOID:
    return ir1.void_type();
  case I32:
    return ir1.int_type<int>();
  case I64:
    return ir1.int_type<int64_t>();
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
  case CHAR:
    return ir1.int_type<int8_t>();
  case UCHAR:
    return ir1.int_type<uint8_t>();
  default:
    return nullptr;
  }
}

static int64_t parse_to_int64(CALiteral *value) {
  if (catype_is_float(value->littypetok))
    return (int64_t)value->u.f64value;
  else
    return value->u.i64value;
}

static double parse_to_double(CALiteral *value) {
  if (catype_is_float(value->littypetok))
    return value->u.f64value;
  else
    return (double)value->u.i64value;
}

static int can_type_binding(CALiteral *lit, int typetok) {
  switch (lit->datatype->type) {
  case I64:
    return !check_i64_value_scope(lit->u.i64value, typetok);
  case U64:
    return !check_u64_value_scope((uint64_t)lit->u.i64value, typetok);
  case F64:
    return !check_f64_value_scope(lit->u.f64value, typetok);
  case BOOL:
    return (typetok == BOOL);
  case CHAR:
    return !check_char_value_scope(lit->u.i64value, typetok);
  case UCHAR:
    return !check_uchar_value_scope(lit->u.i64value, typetok);
  default:
    yyerror("bad lexical literal type: '%s'", get_type_string(lit->datatype->type));
    return 0;
  }
}

Value *gen_literal_value(CALiteral *value, int typetok, SLoc loc) {
  // check if literal value type matches the given typetok, if not match, report error
  if (value->fixed_type && value->datatype->type != typetok) {
    yyerror("line: %d, col: %d: literal value type '%s' not match the variable type '%s'",
	    loc.row, loc.col, get_type_string(value->datatype->type), get_type_string(typetok));
    return nullptr;
  }

  if (!value->fixed_type && !can_type_binding(value, typetok)) {
    yyerror("line: %d, col: %d: literal value type '%s' not match the variable type '%s'",
	    loc.row, loc.col, get_type_string(value->datatype->type), get_type_string(typetok));
    return nullptr;
  }

  switch (typetok) {
  case VOID:
    yyerror("line: %d, col: %d: void type have no literal value", loc.row, loc.col);
    return nullptr;
  case I32:
    return ir1.gen_int((int)parse_to_int64(value));
  case I64:
    return ir1.gen_int(parse_to_int64(value));
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
  case CHAR:
    return ir1.gen_int((char)parse_to_int64(value));
  case UCHAR:
    return ir1.gen_int((uint8_t)parse_to_int64(value));
  default:
    return nullptr;
  }
}

const char *get_printf_format(int type) {
  switch (type) {
  case VOID:
    yyerror("void type have no format value");
    return "\n";
  case I32:
    return "%d";
  case I64:
    return "%ld";
  case U32:
    return "%u";
  case U64:
    return "%lu";
  case F32:
    return "%f";
  case F64:
    return "%lf";
  case CHAR:
    return "%c";
  case UCHAR:
    return "%c";
  case BOOL:
    return "%1d";
  default:
    return "\n";
  }
}

int is_unsigned_type(int type) {
  return type == U32 || type == U64 || type == UCHAR;
}

// row: VOID I32 I64 U32 U64 F32 F64 BOOL CHAR UCHAR ATOMTYPE_END STRUCT ARRAY POINTER
// col: < > GE LE NE EQ
CmpInst::Predicate s_cmp_predicate[ATOMTYPE_END-VOID][6] = {
  {CmpInst::FCMP_FALSE, CmpInst::FCMP_FALSE, CmpInst::FCMP_FALSE, CmpInst::FCMP_FALSE, CmpInst::FCMP_FALSE, CmpInst::FCMP_FALSE}, // VOID
  {CmpInst::ICMP_SLT, CmpInst::ICMP_SGT, CmpInst::ICMP_SGE, CmpInst::ICMP_SLE, CmpInst::ICMP_NE, CmpInst::ICMP_EQ}, // I32
  {CmpInst::ICMP_SLT, CmpInst::ICMP_SGT, CmpInst::ICMP_SGE, CmpInst::ICMP_SLE, CmpInst::ICMP_NE, CmpInst::ICMP_EQ}, // I64
  {CmpInst::ICMP_ULT, CmpInst::ICMP_UGT, CmpInst::ICMP_UGE, CmpInst::ICMP_ULE, CmpInst::ICMP_NE, CmpInst::ICMP_EQ}, // U32
  {CmpInst::ICMP_ULT, CmpInst::ICMP_UGT, CmpInst::ICMP_UGE, CmpInst::ICMP_ULE, CmpInst::ICMP_NE, CmpInst::ICMP_EQ}, // U64
  {CmpInst::FCMP_OLT, CmpInst::FCMP_OGT, CmpInst::FCMP_OGE, CmpInst::FCMP_OLE, CmpInst::FCMP_ONE, CmpInst::FCMP_OEQ}, // F32
  {CmpInst::FCMP_OLT, CmpInst::FCMP_OGT, CmpInst::FCMP_OGE, CmpInst::FCMP_OLE, CmpInst::FCMP_ONE, CmpInst::FCMP_OEQ}, // F64
  {CmpInst::ICMP_ULT, CmpInst::ICMP_UGT, CmpInst::ICMP_UGE, CmpInst::ICMP_ULE, CmpInst::ICMP_NE, CmpInst::ICMP_EQ}, // BOOL
  {CmpInst::ICMP_SLT, CmpInst::ICMP_SGT, CmpInst::ICMP_SGE, CmpInst::ICMP_SLE, CmpInst::ICMP_NE, CmpInst::ICMP_EQ}, // CHAR
  {CmpInst::ICMP_ULT, CmpInst::ICMP_UGT, CmpInst::ICMP_UGE, CmpInst::ICMP_ULE, CmpInst::ICMP_NE, CmpInst::ICMP_EQ}, // UCHAR
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

Value *create_def_value(int typetok) {
  switch(typetok) {
  case I32:
    return ir1.gen_int<int>(0);
    break;
  case I64:
    return ir1.gen_int<int64_t>(0);
    break;
  case U32:
    return ir1.gen_int<uint32_t>(0);
    break;
  case U64:
    return ir1.gen_int<uint64_t>(0);
    break;
  case CHAR:
    return ir1.gen_int<int8_t>(0);
    break;
  case UCHAR:
    return ir1.gen_int<uint8_t>(0);
    break;
  case BOOL:
    return ir1.gen_bool(true);
    break;
  case VOID: {
    //Type *voidty = Type::getVoidTy(ir1.ctx());
    //return Constant::getNullValue(voidty);
    return nullptr;
    break;
  }
  default:
    yyerror("return type `%s` not implemented", get_type_string(typetok));
    return nullptr;
    break;
  }
}

// VOID I32 I64 U32 U64 F32 F64 BOOL CHAR UCHAR ATOMTYPE_END STRUCT ARRAY POINTER
// Trunc ZExt SExt FPToUI FPToSI UIToFP SIToFP FPTrunc FPExt PtrToInt IntToPtr BitCast AddrSpaceCast
// CastOpsBegin stand for no need convert, CastOpsEnd stand for cannot convert
static Instruction::CastOps
llvmtype_cast_table[ATOMTYPE_END - VOID][ATOMTYPE_END - VOID] = {
  { // Begin VOID
    (ICO)0,            /* VOID */
    (ICO)-1,           /* I32 */
    (ICO)-1,           /* I64 */
    (ICO)-1,           /* U32 */
    (ICO)-1,           /* U64 */
    (ICO)-1,           /* F32 */
    (ICO)-1,           /* F64 */
    (ICO)-1,           /* BOOL */
    (ICO)-1,           /* CHAR */
    (ICO)-1,           /* UCHAR */
    //    (ICO)-1            /* STRUCT */
  },                   // VOID -> ?
  { // Begin I32
    (ICO)-1,           /* VOID */
    (ICO)0,            /* I32 */
    ICO::SExt,         /* I64 */
    ICO::BitCast,      /* U32 */
    ICO::SExt,         /* U64 */
    ICO::SIToFP,       /* F32 */
    ICO::SIToFP,       /* F64 */
    (ICO)-1,           /* BOOL */
    ICO::Trunc,        /* CHAR */
    ICO::Trunc,        /* UCHAR */
    //    (ICO)-1,           /* STRUCT */
  },                   // I32 ->
  { // Begin I64
    (ICO)-1,           /* VOID */
    ICO::Trunc,	       /* I32 */
    (ICO)0,            /* I64 */
    ICO::Trunc,	       /* U32 */
    ICO::BitCast,      /* U64 */
    ICO::SIToFP,       /* F32 */
    ICO::SIToFP,       /* F64 */
    (ICO)-1,           /* BOOL */
    ICO::Trunc,	       /* CHAR */
    ICO::Trunc,	       /* UCHAR */
    //    (ICO)-1,           /* STRUCT */
  },                   // I64 ->
  { // Begin U32
    (ICO)-1,           /* VOID */
    ICO::BitCast,      /* I32 */
    ICO::ZExt,	       /* I64 */
    (ICO)0,            /* U32 */
    ICO::ZExt,	       /* U64 */
    ICO::UIToFP,       /* F32 */
    ICO::UIToFP,       /* F64 */
    (ICO)-1,           /* BOOL */
    ICO::Trunc,	       /* CHAR */
    ICO::Trunc,	       /* UCHAR */
    //    (ICO)-1            /* STRUCT */
  },                   // U32 ->
  { // Begin U64
    (ICO)-1,           /* VOID */
    ICO::Trunc,	       /* I32 */
    ICO::BitCast,      /* I64 */
    ICO::Trunc,	       /* U32 */
    (ICO)0,            /* U64 */
    ICO::UIToFP,       /* F32 */
    ICO::UIToFP,       /* F64 */
    (ICO)-1,           /* BOOL */
    ICO::Trunc,	       /* CHAR */
    ICO::Trunc,	       /* UCHAR */
    //    (ICO)-1            /* STRUCT */
  },                   // U64 ->
  { // Begin F32
    (ICO)-1,           /* VOID */
    ICO::FPToSI,       /* I32 */
    ICO::FPToSI,       /* I64 */
    ICO::FPToUI,       /* U32 */
    ICO::FPToUI,       /* U64 */
    (ICO)0,            /* F32 */
    ICO::FPExt,	       /* F64 */
    (ICO)-1,           /* BOOL */
    ICO::FPToSI,       /* CHAR */
    ICO::FPToUI,       /* UCHAR */
    //    (ICO)-1            /* STRUCT */
  },                   // F32 ->
  { // Begin F64
    (ICO)-1,           /* VOID */
    ICO::FPToSI,       /* I32 */
    ICO::FPToSI,       /* I64 */
    ICO::FPToUI,       /* U32 */
    ICO::FPToUI,       /* U64 */
    ICO::FPTrunc,      /* F32 */
    (ICO)0,            /* F64 */
    (ICO)-1,           /* BOOL */
    ICO::FPToSI,       /* CHAR */
    ICO::FPToUI,       /* UCHAR */
    //    (ICO)-1            /* STRUCT */
  },                   // F64 ->
  { // Begin BOOL
    (ICO)-1,           /* VOID */
    ICO::ZExt,	       /* I32 */
    ICO::ZExt,	       /* I64 */
    ICO::ZExt,	       /* U32 */
    ICO::ZExt,	       /* U64 */
    (ICO)-1,           /* F32 */
    (ICO)-1,           /* F64 */
    (ICO)0,            /* BOOL */
    ICO::ZExt,	       /* CHAR */
    ICO::ZExt,	       /* UCHAR */
    //    (ICO)-1            /* STRUCT */
  },                   // BOOL ->
  { // Begin CHAR
    (ICO)-1,           /* VOID */
    ICO::SExt,	       /* I32 */
    ICO::SExt,	       /* I64 */
    ICO::SExt,	       /* U32 */
    ICO::SExt,	       /* U64 */
    ICO::SIToFP,       /* F32 */
    ICO::SIToFP,       /* F64 */
    (ICO)-1 ,          /* BOOL */
    (ICO)0,            /* CHAR */
    ICO::BitCast,      /* UCHAR */
    //    (ICO)-1            /* STRUCT */
  },                   // CHAR ->
  { // Begin UCHAR
    (ICO)-1,           /* VOID */
    ICO::ZExt,	       /* I32 */
    ICO::ZExt,	       /* I64 */
    ICO::ZExt,	       /* U32 */
    ICO::ZExt,	       /* U64 */
    ICO::UIToFP,       /* F32 */
    ICO::UIToFP,       /* F64 */
    (ICO)-1,           /* BOOL */
    ICO::BitCast,      /* CHAR */
    (ICO)0,            /* UCHAR */
    //    (ICO)-1            /* STRUCT */
  },                   // UCHAR ->
#if 0
  { // Begin STRUCT
    (ICO)-1,           /* VOID */
    (ICO)-1,           /* I32 */
    (ICO)-1,           /* I64 */
    (ICO)-1,           /* U32 */
    (ICO)-1,           /* U64 */
    (ICO)-1,           /* F32 */
    (ICO)-1,           /* F64 */
    (ICO)-1,           /* BOOL */
    (ICO)-1,           /* CHAR */
    (ICO)-1,           /* UCHAR */
    (ICO)-1,           /* STRUCT */
  },                   // STRUCT ->
#endif
};

Instruction::CastOps gen_cast_ops(int fromtok, int totok) {
  return llvmtype_cast_table[fromtok-VOID][totok-VOID];
}

// name to CADatatype map
std::unordered_map<int, CADataType *> s_type_map;

// used for literal value convertion, left side is lexical literal value (I64
// stand for negative integer value, U64 stand for positive integer value, F64
// stand for floating point value) right side is real literal value
// VOID I32 I64 U32 U64 F32 F64 BOOL CHAR UCHAR ATOMTYPE_END STRUCT ARRAY POINTER
static int s_literal_type_convertable_table[ATOMTYPE_END - VOID + 1][ATOMTYPE_END - VOID + 1] = {
  {0, }, // VOID -> other-type, means convert from VOID type to other type
  {0, },   // I32 -> other-type
  {0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0}, // I64 ->
  {0, }, // U32 ->
  {0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0}, // U64 ->
  {0, }, // F32 ->
  {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0}, // F64 ->
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0}, // BOOL ->
  {0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0}, // CHAR ->
  {0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0}, // UCHAR ->
  //  {0, }, // STRUCT
  {0, }, // ATOMTYPE_END
};

int literal_type_convertable(int from, int to) {
  return s_literal_type_convertable_table[from-VOID][to-VOID];
}

// for rust the `as` cannot convert others to bool and
// int/float to any primitive type, to pointer * but cannot to reference &, bool
// bool can to int type, but cannot float type, char type
// only u8 can cast to char, but char can to int
// 
int as_type_convertable(int from, int to) {
  if (to == BOOL)
    return from == BOOL;

  switch (from) {
  case BOOL:
    return (to != F32 && to != F64);
  default:
    return true;
  }
}

// check if specified type: typetok can accept literal value
int check_i64_value_scope(int64_t lit, int typetok) {
  // the match table should match the corrsponding line of array s_literal_type_convertable_table
  switch(typetok) {
  case I32:
    if (lit < std::numeric_limits<int>::min())
      return 1;
    return 0;
  case I64:
  case F32:
  case F64:
    return 0;
  default:
    yyerror("i64 lexcial value incompatible with %s", get_type_string(typetok));
    return -1;
  }
}

int check_u64_value_scope(uint64_t lit, int typetok) {
  // the match table should match the corrsponding line of array s_literal_type_convertable_table
  switch(typetok) {
  case I32:
    if (lit > std::numeric_limits<int>::max())
      return -1;
    return 0;
  case I64:
    if (lit > std::numeric_limits<int64_t>::max())
      return -1;
    return 0;
  case U64:
  case F32:
  case F64:
    return 0;
  case U32:
    if (lit > std::numeric_limits<uint32_t>::max())
      return -1;
    return 0;
  case CHAR:
    if (lit > std::numeric_limits<char>::max())
      return -1;
    return 0;
  case UCHAR:
    if (lit > std::numeric_limits<uint8_t>::max())
      return -1;
    return 0;
  default:
    yyerror("u64 lexcial value incompatible with %s", get_type_string(typetok));
    return -1;
  }
}

int check_f64_value_scope(double lit, int typetok) {
  // the match table should match the corrsponding line of array s_literal_type_convertable_table
  switch(typetok) {
  case F32:
    if (lit < std::numeric_limits<float>::min() || lit > std::numeric_limits<float>::max())
      return 1;
    return 0;
  case F64:
    return 0;
  default:
    yyerror("f64 lexcial value incompatible with %s", get_type_string(typetok));
    return -1;
  }
}

int check_char_value_scope(char lit, int typetok) {
  if (typetok == UCHAR && lit < 0)
    return 1;

  return 0;
}

int check_uchar_value_scope(uint8_t lit, int typetok) {
  if (typetok == CHAR && lit > 127)
    return -1;

  return 0;
}

int catype_is_float(int typetok) {
  return (typetok == F32 || typetok == F64);
}

static CADataType *catype_make_type(const char *name, int type, int size) {
  auto datatype = new CADataType;
  int formalname = symname_check_insert(name);
  datatype->formalname = formalname;
  datatype->type = type;
  datatype->size = size;
  datatype->signature = formalname;
  datatype->struct_layout = nullptr;
  catype_put_by_name(formalname, datatype);
  return datatype;
}

