#include "ca.h"
#include "symtable.h"
#include "ca.tab.h"
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

#define EXTERN_C extern "C" {

#ifdef __cplusplus
EXTERN_C
#endif

void yyerror(const char *s, ...);
extern int glineno;
extern int gcolno;

#ifdef __cplusplus
}
#endif


// static char *s_symname_buffer = NULL;
static std::vector<char> s_symname_buffer;
static std::unordered_map<std::string, int> s_symname_name2pos;
static std::unordered_map<std::string, int> s_token_map = {
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
  {"char",   CHAR},
  {"uchar",  UCHAR},
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

// name to CADatatype map
static std::unordered_map<int, CADataType *> s_type_map;

// used for literal value convertion, left side is lexical literal value (I64
// stand for negative integer value, U64 stand for positive integer value, F64
// stand for floating point value) right side is real literal value
// VOID I32 I64 U32 U64 F32 F64 BOOL CHAR UCHAR STRUCT ATOMTYPE_END
static int s_type_convertable_table[ATOMTYPE_END - VOID + 1][ATOMTYPE_END - VOID + 1] = {
  {0, }, // VOID -> other-type, means convert from VOID type to other type
  {0, },   // I32 -> other-type
  {0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0}, // I64 ->
  {0, }, // U32 ->
  {0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0}, // U64 ->
  {0, }, // F32 ->
  {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0}, // F64 ->
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}, // BOOL ->
  {0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0}, // CHAR ->
  {0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 0}, // UCHAR ->
  {0, }, // STRUCT
  {0, }, // ATOMTYPE_END
};

static int type_convertable(int from, int to) {
  return s_type_convertable_table[from-VOID][to-VOID];
}

// check if specified type: typetok can accept literal value
int check_i64_value_scope(int64_t lit, int typetok) {
  // the match table should match the corrsponding line of array s_type_convertable_table
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
  // the match table should match the corrsponding line of array s_type_convertable_table
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
  // the match table should match the corrsponding line of array s_type_convertable_table
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

static int symname_insert(const std::string &s) {
  size_t tsize = s_symname_buffer.size() + s.size() + 1;

  if (tsize > s_symname_buffer.capacity())
    s_symname_buffer.reserve(tsize * 2);

  size_t bsize = s_symname_buffer.size();
  s_symname_buffer.resize(tsize);
  strcpy(&s_symname_buffer[bsize], s.c_str());

  s_symname_name2pos.insert(std::make_pair(s, (int)bsize));
  return bsize;
}

using SymTableInner = std::unordered_map<int, std::unique_ptr<STEntry>>;

#ifdef __cplusplus
EXTERN_C
#endif

int lexical_init() {
  return 0;
}

int find_lexical_keyword(const char *name) {
  auto itr = s_token_map.find(name);
  if (itr != s_token_map.end())
    return itr->second;

  return -1;
}

static CADataType *catype_make_type(const char *name, int type, int size) {
  auto datatype = new CADataType;
  int formalname = symname_check_insert(name);
  datatype->formalname = formalname;
  datatype->type = type;
  datatype->size = size;
  datatype->struct_layout = nullptr;
  catype_put_by_name(formalname, datatype);
  return datatype;
}

int catype_init() {
  CADataType *datatype;
  int name;
  datatype = catype_make_type("void", VOID, 0); // void
  datatype = catype_make_type("i32", I32, 4); // i32

  name = symname_check_insert("int");
  catype_put_by_name(name, datatype);         // int

  catype_make_type("i64", I64, 8);            // i64

  datatype = catype_make_type("u32", U32, 4); // u32

  name = symname_check_insert("uint");
  catype_put_by_name(name, datatype);         // uint

  catype_make_type("u64", U64, 8);            // u64
  catype_make_type("f32", F32, 4);            // f32
  catype_make_type("f64", F64, 8);            // f64
  catype_make_type("bool", BOOL, 1);          // bool
  catype_make_type("char", CHAR, 1);          // char
  catype_make_type("uchar", UCHAR, 1);        // uchar

  return 0;
}

int catype_put_by_name(int name, CADataType *datatype) {
  s_type_map.insert(std::move(std::make_pair(name, datatype)));
  return 0;
}

CADataType *catype_get_by_name(int name) {
  auto itr = s_type_map.find(name);
  if (itr == s_type_map.end())
    return nullptr;

  return itr->second;
}

int catype_put_by_token(int token, CADataType *datatype) {
  // TODO:
  return 0;
}

CADataType *catype_get_by_token(int token) {
  // TODO:
  return nullptr;
}

int catype_is_float(int typetok) {
  return (typetok == F32 || typetok == F64);
}

static int parse_lexical_char(const char *text) {
  if (text[0] != '\\')
    return text[0];

  switch(text[1]) {
  case 'r':
    return '\r';
  case 'n':
    return '\n';
  case 't':
    return '\t';
  default:
    yyerror("unimplemented special character");
    return -1;
  }
}

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

// TODO: check if text match the typetok, example: 'a' means char, and it cannot apply any postfix
// true, false means boolean, it cannot apply any postfix
// if manualtypetok == -1, means only get type from littypetok or both typetok will be considered to check the error messages

// def_lit_type
// U64 stand for positive integer value in lexical
// I64 stand for positive integer value in lexical
// F64 stand for floating point number in lexical
// BOOL stand for boolean point number in lexical
// UCHAR stand for \. transfermation value in lexical
// CHAR stand for any character value in lexical

// literal type depends on the input of
// 1) littypetok: it's the literal type by itself, I64 for negative integer
// value, U64 for positive integer value, F64 for floating point value, BOOL is
// true false value, CHAR is 'x' value, UCHAR is '\x' value.
// 2) manualtypetok: it's the literal type in the postfix of the literal, e.g.
// 43243u32 4343243.432f32 43243.343f64 -4332i64 3f64 ..., the scope or type of
// manualtypetok must compitable with the littypetok type. e.g. when literal
// value is 4324324321433u32 then the manualtypetok is U32, it is a bad value
// and will report an error because U32 is out of the range the literal. and
// when literal value is 43243243.343i32 it also report the error, because
// floating point literal value cannot be i32 type, but 432432f32 is right
// value.
// 3) borning_var_type: if not 0 value, it means a variable is borning
// (creating) and the variable's type is borning_var_type, it will guide the
// final literal type.
//
// The 3 parameters will affect the inference of the literal type by following
// rules:
// 1). if all the operands of an operator are non-fixed literal value
// (`lit->fixed_type == 0` or manualtypetok is not provided (-1 value)), it
// uses the variable's type (`borning_var_type`)
// 2) when one of the operand is the fixed literal then the other non-fixed
// literal value's type will be the fixed literal value's type. when have
// multiple different fixed type in the expression, then report an error
// 3) when the variable not specify a value, the literal's type will be
// inferenced by the right ( = right-expr) expression's type. It tries to uses
// the first value that with the fixed type as the expression's type. When the
// other part of the expression have different type then report an error
// final literal's type
//
// so the final literal type should better be determined in the walk routines,
// for the first scan is hard to determine the types, because the expression may
// have multiple part and the pre part don't know the later part's type

void create_literal(CALiteral *lit, const char *text, int littypetok, int manualtypetok) {
  const char *typestr;

  int typetok;
  if (manualtypetok == -1) {
    lit->fixed_type = 0;
  } else {
    lit->intent_type = 0;
    lit->fixed_type = 1;
  }

  if (lit->fixed_type) {
    const char *name = get_type_string(manualtypetok);
    // check convertable
    if (!type_convertable(littypetok, manualtypetok)) {
      yyerror("line: %d, col: %d: bad literal value definition: %s cannot be %s",
	      glineno, gcolno,
	      get_type_string(littypetok), get_type_string(manualtypetok));
      return;
    }

    int badscope = 0;
    // check literal value scope
    switch (littypetok) {
    case I64: // I64 stand for positive integer value in lexical
      lit->u.i64value = atoll(text);
      badscope = check_i64_value_scope(lit->u.i64value, manualtypetok);
      break;
    case U64:
      sscanf(text, "%lu", &lit->u.i64value);
      badscope = check_u64_value_scope((uint64_t)lit->u.i64value, manualtypetok);
      break;
    case F64:
      lit->u.f64value = atof(text);
      badscope = check_f64_value_scope(lit->u.f64value, manualtypetok);
      break;
    case BOOL:
      lit->u.i64value = atoll(text) ? 1 : 0;
      badscope = (manualtypetok != BOOL);
      break;
    case CHAR:
      lit->u.i64value = (char)parse_lexical_char(text);
      badscope = check_char_value_scope(lit->u.i64value, manualtypetok);
      break;
    case UCHAR:
      lit->u.i64value = (uint8_t)parse_lexical_char(text);
      badscope = check_uchar_value_scope(lit->u.i64value, manualtypetok);
    default:
       yyerror("line: %d, col: %d: %s type have no lexical value",
	       glineno, gcolno, get_type_string(littypetok));
      break;
    }

    if (badscope) {
      yyerror("line: %d, col: %d: bad literal value definition: %s cannot be %s",
	      glineno, gcolno, get_type_string(littypetok), get_type_string(manualtypetok));
      return;
    }
      
    lit->datatype = catype_get_by_name(symname_check(name));
    return;
  }

  // handle non-fixed type literal value
  switch (littypetok) {
  case I64:
    lit->intent_type = I32;
    lit->datatype = catype_get_by_name(symname_check("i64"));
    lit->u.i64value = atoll(text);
    break;
  case U64:
    lit->intent_type = I32;
    lit->datatype = catype_get_by_name(symname_check("u64"));
    sscanf(text, "%lu", &lit->u.i64value);
    break;
  case F64:
    lit->intent_type = F64;
    lit->datatype = catype_get_by_name(symname_check("f64"));
    lit->u.f64value = atof(text);
    break;
  case BOOL:
    lit->intent_type = BOOL;
    lit->datatype = catype_get_by_name(symname_check("bool"));
    lit->u.i64value = atoll(text) ? 1 : 0;
    break;
  case CHAR:
    lit->intent_type = CHAR;
    lit->datatype = catype_get_by_name(symname_check("char"));    
    lit->u.i64value = (char)parse_lexical_char(text);
    break;
  case UCHAR:
    lit->intent_type = CHAR;
    lit->datatype = catype_get_by_name(symname_check("uchar"));
    lit->u.i64value = (uint8_t)parse_lexical_char(text);
    break;
  default:
    yyerror("line: %d, col: %d: void type have no literal value", glineno, gcolno);
    break;
  }
}

void set_litbuf(LitBuffer *litb, const char *text, int len, int typetok) {
  if (len > 1023)
    yyerror("too long literal length: %d", len);

  litb->typetok = typetok;
  litb->len = len;
  memcpy(litb->text, text, len);
  litb->text[len] = 0;
}

int def_lit_type(int typetok) {
  switch (typetok) {
  case U64:
  case I64:
    return I32;
  case F64:
  default:
    return typetok;
  }
}

CAVariable *cavar_create(int name, CADataType *datatype) {
  CAVariable *var = new CAVariable;
  var->datatype = datatype;
  var->name = name;
  var->llvm_value = nullptr;
  return var;
}

void cavar_destroy(CAVariable **var) {
  delete *var;
  *var = nullptr;
}

int symname_init() {
  s_symname_buffer.reserve(1024);
  s_symname_name2pos.clear();
  return 0;
}

int symname_check_insert(const char *name) {
  auto s = std::string(name);
  auto itr = s_symname_name2pos.find(s);
  if (itr != s_symname_name2pos.end())
    return itr->second;

  return symname_insert(s);
}

int symname_check(const char *name) {
  auto itr = s_symname_name2pos.find(name);
  if (itr != s_symname_name2pos.end())
    return itr->second;

  return -1;
}

const char *symname_get(int pos) { return &s_symname_buffer[pos]; }

///////////////////////////////////////////////////////

int sym_init(SymTable *st, SymTable *parent) {
  st->parent = parent;
  st->opaque = new SymTableInner;
  SymTableInner *t = (SymTableInner *)st->opaque;

  return 0;
}

int sym_check_insert_withname(SymTable *st, const char *name, SymType type) {
  SymTableInner *t = ((SymTableInner *)st->opaque);
  int pos = symname_check_insert(name);

  auto itr = t->find(pos);
  if (itr == t->end()) {
    auto entry = std::make_unique<STEntry>();
    entry->sym_name = pos;
    t->insert(std::make_pair(pos, std::move(entry)));
  }

  return pos;
}

STEntry *sym_check_insert(SymTable *st, int name, SymType type) {
  SymTableInner *t = ((SymTableInner *)st->opaque);

  auto itr = t->find(name);
  if (itr == t->end()) {
    auto entry = std::make_unique<STEntry>();
    entry->sym_name = name;
    entry->sym_type = type;
    auto result = t->insert(std::make_pair(name, std::move(entry)));
    return result.first->second.get();
  }

  return itr->second.get();
}

STEntry *sym_insert(SymTable *st, int name, SymType type) {
  SymTableInner *t = ((SymTableInner *)st->opaque);
  auto entry = std::make_unique<STEntry>();
  entry->sym_name = name;
  entry->sym_type = type;
  // entry->sloc = ?; TODO: assign a location
  entry->u.var = nullptr;
  auto result = t->insert(std::make_pair(name, std::move(entry)));

  return result.first->second.get();
}

int sym_dump(SymTable *st, FILE *file) {
  SymTableInner *t = (SymTableInner *)st->opaque;
  int tlen = (int)t->size();
  int totallen = tlen;
  fwrite(&tlen, 1, sizeof(tlen), file);

  for (auto itr = t->begin(); itr != t->end(); ++itr) {
    const char *sym = symname_get(itr->first);
    int len = strlen(sym);
    fwrite(&len, 1, sizeof(len), file);
    fwrite(sym, 1, len, file);
    totallen += sizeof(len) + len;    
  }
  
  return totallen;
}

STEntry *sym_getsym(SymTable *st, int idx, int parent) {
  while (st) {
    SymTableInner *t = ((SymTableInner *)st->opaque);
    auto itr = t->find(idx);
    if (itr == t->end()) {
      if (parent) {
	st = st->parent;
	continue;
      } else {
	return NULL;
      }
    }

    return itr->second.get();
  }

  return NULL;
}

int sym_tablelen(SymTable *st) {
  return (int)((SymTableInner *)st->opaque)->size();
}

SymType sym_gettype(SymTable *st, int idx, int parent) {
  return sym_getsym(st, idx, parent)->sym_type;
}

SLoc sym_getsloc(SymTable *st, int idx, int parent) {
  return sym_getsym(st, idx, parent)->sloc;
}

void sym_setsloc(SymTable *st, int idx, SLoc loc) {
  sym_getsym(st, idx, 0)->sloc = loc;
}

void sym_destroy(SymTable *st) {
  SymTableInner *table = (SymTableInner *)st->opaque;
  delete table;
}

#ifdef __cplusplus
}
#endif



