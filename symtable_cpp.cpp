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
extern int borning_var_type;

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

int type_convertable(int from, int to) {
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

  datatype = catype_make_type("f32", F32, 4); // f32
  name = symname_check_insert("float");
  catype_put_by_name(name, datatype);         // float

  datatype = catype_make_type("f64", F64, 8); // f64
  name = symname_check_insert("double");
  catype_put_by_name(name, datatype);         // double

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

void set_litbuf(LitBuffer *litb, char *text, int len, int typetok) {
  int name = symname_check_insert(text);

  litb->typetok = typetok;
  litb->len = len;
  litb->text = name;
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



