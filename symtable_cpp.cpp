#include "ca.h"
#include "symtable.h"
#include "ca.tab.h"
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
  {"int",    I32},
  {"i32",    I32},
  {"i64",    I64},
  {"uint",   U32},
  {"u32",    U32},
  {"u64",    U64},
  {"float",  F32},
  {"double", F64},
  {"bool",   BOOL},
  {"char",   CHAR},
  {"uchar",  UCHAR},
  {">=",     GE},
  {"<=",     LE},
  {"==",     EQ},
  {"!=",     NE},
  {"while",  WHILE},
  {"if",     IF},
  {"else",   ELSE},
  {"print",  PRINT},
  {"goto",   GOTO},
  {"fn",     FN},
  {"extern", EXTERN},
  {"return", RET},
  {"let",    LET},
  {"...",    VARG},
};

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

int lexical_init() {
  return 0;
}

int find_lexical_keyword(const char *name) {
  auto itr = s_token_map.find(name);
  if (itr != s_token_map.end())
    return itr->second;

  return -1;
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

STEntry *sym_check_insert(SymTable *st, int encode, SymType type) {
  SymTableInner *t = ((SymTableInner *)st->opaque);

  auto itr = t->find(encode);
  if (itr == t->end()) {
    auto entry = std::make_unique<STEntry>();
    entry->sym_name = encode;
    entry->sym_type = type;
    auto result = t->insert(std::make_pair(encode, std::move(entry)));
    return result.first->second.get();
  }

  return itr->second.get();
}

STEntry *sym_insert(SymTable *st, int encode, SymType type) {
  SymTableInner *t = ((SymTableInner *)st->opaque);
  auto entry = std::make_unique<STEntry>();
  entry->sym_name = encode;
  entry->sym_value = 0;
  entry->sym_type = type;
  // entry->sloc = ?; TODO: assign a location
  entry->u.llvm_value = NULL;
  auto result = t->insert(std::make_pair(encode, std::move(entry)));

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

int sym_getvalue(SymTable *st, int idx, int parent) {
  return sym_getsym(st, idx, parent)->sym_value;
}

void sym_setvalue(SymTable *st, int idx, int value) {
  sym_getsym(st, idx, 0)->sym_value = value;
}

void sym_destroy(SymTable *st) {
  SymTableInner *table = (SymTableInner *)st->opaque;
  delete table;
}

#ifdef __cplusplus
}
#endif



