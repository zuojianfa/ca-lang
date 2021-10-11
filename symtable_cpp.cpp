#include "ca.h"
#include "symtable.h"
#include "ca.tab.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

//static char *s_symname_buffer = NULL;
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
};

// name to CADatatype map
static std::unordered_map<int, CADataType*> s_type_map;

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

#define EXTERN_C extern "C" {

#ifdef __cplusplus
EXTERN_C
#endif

void yyerror(const char *s, ...);

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

void create_literal(CALiteral *lit, const char *text, int littypetok, int manualtypetok) {
  // TODO: check if text match the typetok, example: 'a' means char, and it cannot apply any postfix
  // true, false means boolean, it cannot apply any postfix
  // if manualtypetok == -1, means only get type from littypetok or both typetok will be considered to check the error messages
  int typetok;
  if (manualtypetok == -1)
    typetok = littypetok;
  else
    typetok = manualtypetok;

  switch (typetok) {
  case VOID:
    yyerror("void type have no literal value");
  case I32:
    lit->datatype = catype_get_by_name(symname_check("i32"));
    lit->u.i64value = atoi(text);
    break;
  case I64:
    lit->datatype = catype_get_by_name(symname_check("i64"));
    lit->u.i64value = atoll(text);
    break;
  case U32:
    lit->datatype = catype_get_by_name(symname_check("u32"));
    sscanf(text, "%lu", &lit->u.i64value);
    break;
  case U64:
    lit->datatype = catype_get_by_name(symname_check("u64"));
    sscanf(text, "%lu", &lit->u.i64value);
    break;
  case F32:
    lit->datatype = catype_get_by_name(symname_check("f32"));
    lit->u.f64value = atof(text);
    break;
  case F64:
    lit->datatype = catype_get_by_name(symname_check("f64"));
    lit->u.f64value = atof(text);
    break;
  case BOOL:
    lit->datatype = catype_get_by_name(symname_check("bool"));
    lit->u.i64value = atoll(text) ? 1 : 0;
    break;
  case CHAR:
    lit->datatype = catype_get_by_name(symname_check("char"));    
    lit->u.i64value = (char)parse_lexical_char(text);
    break;
  case UCHAR:
    lit->datatype = catype_get_by_name(symname_check("uchar"));
    lit->u.i64value = (uint8_t)parse_lexical_char(text);
    break;
  default:
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



