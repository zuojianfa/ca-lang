#include "ca.h"
#include "symtable.h"
#include "ca.tab.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <sstream>
#include <stack>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

#include "type_system.h"

#include <unordered_map>

BEGIN_EXTERN_C
void yyerror(const char *s, ...);
extern int glineno;
extern int gcolno;
END_EXTERN_C

extern std::unordered_map<std::string, int> s_token_map;

// static char *s_symname_buffer = NULL;
static std::vector<char> s_symname_buffer;
static std::unordered_map<std::string, int> s_symname_name2pos;

static std::stack<std::unique_ptr<ST_ArgListActual>> s_actualarglist_stack;

ST_ArgListActual *actualarglist_current() {
  return s_actualarglist_stack.top().get();
}

ST_ArgListActual *actualarglist_new_push() {
  auto aa = std::make_unique<ST_ArgListActual>();
  aa->argc = 0;
  s_actualarglist_stack.push(std::move(aa));
  return s_actualarglist_stack.top().get();
}

void actualarglist_pop() {
  s_actualarglist_stack.pop();
}

static int symname_insert(const std::string &s) {
  size_t tsize = s_symname_buffer.size() + s.size() + 1;

  if (tsize > s_symname_buffer.capacity())
    s_symname_buffer.reserve(tsize * 2);

  size_t bsize = s_symname_buffer.size();
  s_symname_buffer.resize(tsize);
  strncpy(&s_symname_buffer[bsize], s.data(), s.size());
  s_symname_buffer[tsize-1] = (char)0;

  s_symname_name2pos.insert(std::make_pair(s, (int)bsize));
  return bsize;
}

using SymTableInner = std::unordered_map<int, std::unique_ptr<STEntry>>;

std::vector<CALiteral> *arraylit_deref(CAArrayLit obj) {
  auto *v = static_cast<std::vector<CALiteral> *>(obj.data);
  return v;
}

std::vector<ASTNode *> *arrayexpr_deref(CAArrayExpr obj) {
  auto *v = static_cast<std::vector<ASTNode *> *>(obj.data);
  return v;
}

std::vector<void *> *structexpr_deref(CAStructExpr obj) {
  auto *v = static_cast<std::vector<void *> *>(obj.data);
  return v;
}

BEGIN_EXTERN_C
int lexical_init() {
  return 0;
}

int find_lexical_keyword(const char *name) {
  auto itr = s_token_map.find(name);
  if (itr != s_token_map.end())
    return itr->second;

  return -1;
}

void set_litbuf(LitBuffer *litb, const char *text, int len, int typetok) {
  int name = symname_check_insert(text);

  litb->typetok = typetok;
  litb->len = len;
  litb->text = name;
}

void set_litbuf_symname(LitBuffer *litb, int name, int len, int typetok) {
  litb->typetok = typetok;
  litb->len = len;
  litb->text = name;
}

CAArrayExpr arrayexpr_new() {
  std::vector<ASTNode *> *v = new std::vector<ASTNode *>;
  CAArrayExpr expr = { static_cast<void *>(v) };
  return expr;
}

CAArrayExpr arrayexpr_append(CAArrayExpr obj, ASTNode *expr) {
  auto *v = static_cast<std::vector<ASTNode *> *>(obj.data);
  v->push_back(expr);
  return obj;
}

size_t arrayexpr_size(CAArrayExpr obj) {
  std::vector<ASTNode *> *vs = static_cast<std::vector<ASTNode *> *>(obj.data);
  size_t size = vs->size();
  return size;
}

ASTNode *arrayexpr_get(CAArrayExpr obj, int idx) {
  return static_cast<std::vector<ASTNode *> *>(obj.data)->at(idx);
}

CAVariable *cavar_create(int name, typeid_t datatype) {
  CAVariable *var = new CAVariable;
  var->datatype = datatype;
  var->loc = (SLoc){glineno, gcolno};
  var->name = name;
  var->llvm_value = nullptr;
  var->global = 0;
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

int symname_binary_check_insert(const char *name, int len) {
  auto s = std::string(name, len);
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

STEntry *sym_gettypesym_by_name(SymTable *st, const char *name, int parent) {
  typeid_t id = sym_form_type_id_by_str(name);
  STEntry *entry = sym_getsym(st, id, parent);
  if (!entry || entry->sym_type != Sym_DataType) {
    yyerror("the symbol name `%s` is not a type", name);
    return nullptr;
  }

  return entry;
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

static std::vector<char> &buffer_prepare(void *handle, int len, int &oldsize) {
  std::vector<char> &v = *static_cast<std::vector<char> *>(handle);
  oldsize = v.size();
  size_t newsize = oldsize + len;
  if (newsize > v.capacity())
    v.reserve(newsize * 2);

  v.resize(newsize);
  return v;
}

void *buffer_create() {
  auto *pv = new std::vector<char>;
  pv->reserve(4096);
  void *v = static_cast<void *>(pv);
  return v;
}

void buffer_destroy(void *handle) {
  std::vector<char> *v = static_cast<std::vector<char> *>(handle);
  delete v;
}

void buffer_append(void *handle, const char *text, int len) {
  int oldsize = 0;
  auto &v = buffer_prepare(handle, len, oldsize);
  strncpy(&v[oldsize], text, len);
}

void buffer_append_char(void *handle, int ch) {
  int oldsize = 0;
  auto &v = buffer_prepare(handle, 1, oldsize);
  v[oldsize] = (char)ch;
}

int buffer_binary_end(void *handle, int *len) {
  int oldsize = 0;
  auto &v = buffer_prepare(handle, 1, oldsize);
  v[oldsize] = (char)0;

  *len = v.size() - 1;
  int name = symname_binary_check_insert(v.data(), v.size());

  buffer_destroy(handle);
  return name;
}

const char *buffer_end(void *handle, int *len) {
  int name = buffer_binary_end(handle, len);
  return symname_get(name);
}

void *vec_new() {
  std::vector<void *> *vec = new std::vector<void *>;
  return static_cast<void *>(vec);
}

void vec_append(void *handle, void *item) {
  std::vector<void *> *vec = static_cast<std::vector<void *> *>(handle);
  vec->push_back(item);
}

size_t vec_size(void *handle) {
  std::vector<void *> *vec = static_cast<std::vector<void *> *>(handle);
  return vec->size();
}

void *vec_at(void *handle, int index) {
  std::vector<void *> *vec = static_cast<std::vector<void *> *>(handle);
  return vec->at(index);
}

void vec_reverse(void *handle) {
  std::vector<void *> *vec = static_cast<std::vector<void *> *>(handle);
  std::reverse(vec->begin(), vec->end());
}

END_EXTERN_C

