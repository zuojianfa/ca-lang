#include "ca_parser.h"
#include "ca_types.h"
#include "symtable.h"
#include "ca.tab.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <ios>
#include <limits>
#include <memory>
#include <set>
#include <sstream>
#include <stack>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <utility>
#include <vector>
#include <unordered_map>

#include "type_system.h"

#include <unordered_map>

BEGIN_EXTERN_C
extern int glineno;
extern int gcolno;
extern SymTable *curr_symtable;
END_EXTERN_C

extern std::unordered_map<std::string, int> s_token_map;
extern TypeImplInfo *current_type_impl;

// static char *s_symname_buffer = NULL;
static std::vector<char> s_symname_buffer;
static std::unordered_map<std::string, int> s_symname_name2pos;

static std::stack<std::unique_ptr<ST_ArgListActual>> s_actualarglist_stack;
static std::stack<std::unique_ptr<ST_ArgList>> s_tuplelist_stack;
static std::stack<ASTNode *> s_ifstmt_stack;

struct SourceInfo {
  std::vector<std::string> content;
  size_t size;
  std::vector<char> buffer;
};

SourceInfo g_source_info;

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

ST_ArgList *tuplelist_current() {
  return s_tuplelist_stack.top().get();
}

ST_ArgList *tuplelist_new_push() {
  auto aa = std::make_unique<ST_ArgList>();
  aa->argc = 0;
  aa->contain_varg = 0;
  aa->symtable = curr_symtable;

  s_tuplelist_stack.push(std::move(aa));
  return s_tuplelist_stack.top().get();
}

void tuplelist_pop() {
  s_tuplelist_stack.pop();
}

ASTNode *ifstmt_current() {
  return s_ifstmt_stack.top();
}

ASTNode *ifstmt_new_push() {
  ASTNode *p = new_ifstmt_node();
  p->ifn.conds = vec_new();
  p->ifn.bodies = vec_new();

  s_ifstmt_stack.push(p);
  return p;
}

void ifstmt_pop(int isexpr) {
  ASTNode *p = s_ifstmt_stack.top();
  p->ifn.isexpr = isexpr;
  p->ifn.ncond = vec_size(p->ifn.conds);

  ASTNode *cond1 = (ASTNode *)vec_at(p->ifn.conds, 0);
  ASTNode *lastn = NULL;
  if (p->ifn.remain)
    lastn = p->ifn.remain;
  else
    lastn = (ASTNode *)vec_at(p->ifn.bodies, p->ifn.ncond - 1);

  p->begloc = cond1->begloc;
  p->endloc = lastn->endloc;

  s_ifstmt_stack.pop();
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
  CAArrayExpr expr = {0, static_cast<void *>(v) };
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

CAArrayExpr arrayexpr_fill(CAArrayExpr obj, ASTNode *expr, size_t n) {
  std::vector<ASTNode *> *vs = static_cast<std::vector<ASTNode *> *>(obj.data);
  //vs->resize(n, expr);
  vs->push_back(expr);
  obj.repeat_count = n;
  return obj;
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

CAVariable *cavar_create_with_loc(int name, typeid_t datatype, SLoc *loc) {
  CAVariable *cavar = cavar_create(name, datatype);
  cavar->loc = *loc;
  return cavar;
}

CAVariable *cavar_create_self(int name) {
  SLoc stloc = {glineno, gcolno};
  if (!current_type_impl) {
    caerror(&stloc, NULL, "self used in non type implemention is not allowed");
    return nullptr;
  }

  const char *strname = symname_get(name);
  if (strcmp(strname, OSELF)) {
    caerror(&stloc, NULL, "for `%s`, please give it a type or using `self`", strname);
    return nullptr;
  }

  typeid_t datatype = current_type_impl->class_id;
  CAVariable *var = new CAVariable;
  var->datatype = datatype;
  var->loc = stloc;
  var->name = name;
  var->llvm_value = nullptr;
  var->global = 0;
  return var;
}

void cavar_destroy(CAVariable **var) {
  delete *var;
  *var = nullptr;
}

CAPattern *capattern_new(int name, PatternType type, PatternGroup *pg) {
  CAPattern *cap = new CAPattern;
  cap->type = type;
  cap->datatype = typeid_novalue;
  cap->fieldname = -1;
  cap->morebind = nullptr;
  cap->loc = (SLoc){glineno, gcolno};
  switch (type) {
  case PT_Var:
    cap->name = name;
    break;
  case PT_Array:
  case PT_GenTuple:
  case PT_Tuple:
  case PT_Struct:
    cap->name = name;
    cap->items = pg;
    break;
  case PT_IgnoreOne:
  case PT_IgnoreRange:
    break;
  default:
    caerror(&cap->loc, &cap->loc, "Unknown pattern type: `%d`", type);
    return nullptr;
  }

  return cap;
}

void capattern_delete(CAPattern *cap) {
  delete cap;
}

PatternGroup *patterngroup_new() {
  PatternGroup *pg = new PatternGroup;
  pg->size = 0;
  pg->capacity = 0;
  pg->patterns = nullptr;
  return pg;
}

void patterngroup_reinit(PatternGroup *pg) {
  pg->size = 0;
  pg->capacity = 0;
  delete[] pg->patterns;
  pg->patterns = nullptr;
}

void patterngroup_delete(PatternGroup *pg) {
    delete[] pg->patterns;
    delete pg;
}

PatternGroup *patterngroup_push(PatternGroup *pg, CAPattern *cap) {
  if (pg->size >= pg->capacity) {
    int newsize = (pg->size + 1) * 2;
    pg->patterns = (CAPattern **)realloc(pg->patterns, sizeof(CAPattern *) * newsize);
    pg->capacity = newsize;
  }

  pg->patterns[pg->size++] = cap;
  return pg;
}

void patterngroup_pop(PatternGroup *pg) {
  pg->size -= 1;
}

CAPattern *patterngroup_top(PatternGroup *pg) {
  return pg->patterns[pg->size-1];
}

CAPattern *patterngroup_at(PatternGroup *pg, int i) {
  if (i >= pg->size || i < 0)
    return nullptr;

  return pg->patterns[i];
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
  entry->u.varshielding.current = nullptr;
  entry->u.varshielding.varlist = nullptr;
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

int sym_is_sub_symtable(SymTable *sub, SymTable *root) {
  while (sub != NULL && sub != root)
    sub = sub->parent;

  if (sub)
    return 1;

  return 0;
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

void *vec_front(void *handle) {
  std::vector<void *> *vec = static_cast<std::vector<void *> *>(handle);
  return vec->empty() ? nullptr : vec->front();
}

void vec_pushfront(void *handle, void *item) {
  std::vector<void *> *vec = static_cast<std::vector<void *> *>(handle);
  vec->insert(vec->begin(), item);
}

void *vec_popfront(void *handle) {
  std::vector<void *> *vec = static_cast<std::vector<void *> *>(handle);
  if (vec->empty())
    return nullptr;

  auto front = vec->front();
  vec->erase(vec->begin());
  return front;
}

void *vec_popback(void *handle) {
  std::vector<void *> *vec = static_cast<std::vector<void *> *>(handle);
  if (vec->empty())
    return nullptr;

  auto back = vec->back();
  vec->pop_back();
  return back;
}

size_t vec_size(void *handle) {
  if (!handle)
    return 0;

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

void vec_drop(void *handle) {
  std::vector<void *> *vec = static_cast<std::vector<void *> *>(handle);
  delete vec;
}

static std::string &string_deref(void *handle) {
  std::string *s = static_cast<std::string *>(handle);
  return *s;
}

void *string_new() {
  std::string *s = new std::string;
  return static_cast<void *>(s);
}

void string_append(void *handle, const char *s) {
  string_deref(handle).append(s);
}

void string_append_char(void *handle, int ch) {
  string_deref(handle).append(1, ch);
}

const char *string_c_str(void *handle) { return string_deref(handle).c_str(); }

void string_pop_back(void *handle) {
  string_deref(handle).pop_back();
}

void string_drop(void *handle) {
  std::string *s = &string_deref(handle);
  delete s;
}

void source_info_init(const char *srcpath) {
  std::ifstream ifs(srcpath, std::ios::binary | std::ios::in);
  if (ifs.fail()) {
    yyerror("read source file failed for source info");
    return;
  }

  std::string line;
  while (std::getline(ifs, line, '\n')) {
    g_source_info.content.push_back(line);
  }

  struct stat statbuf;
  lstat(srcpath, &statbuf);
  g_source_info.size = statbuf.st_size;

  ifs.close();

  g_source_info.buffer.resize(g_source_info.size + 1);

#if 0
  printf("%zu\n", g_source_info.size());

  std::for_each(g_source_info.begin(), g_source_info.end(), [](const std::string &s) {
    printf("%s\n", s.c_str());
  });
#endif
}

const char *source_line(int lineno) {
  if (lineno < 1 || lineno > g_source_info.content.size())
    return "";

  return g_source_info.content[lineno-1].c_str();
}

const char *source_lines(int linefrom, int lineto) {
  if (linefrom < 1 || linefrom > g_source_info.content.size() ||
      lineto < 1 || lineto > g_source_info.content.size() ||
      linefrom > lineto)
    return "";

  if (linefrom == lineto)
    g_source_info.content[linefrom-1].c_str();

  char *buffer = &g_source_info.buffer[0];
  size_t startpos = 0;
  for (int i = linefrom-1; i < lineto; ++i) {
    std::string &data = g_source_info.content[i];
    memcpy(buffer + startpos, data.c_str(), data.length());
    startpos += data.length();
    buffer[startpos++] = '\n';
  }

  buffer[startpos] = 0;

  return buffer;
}

const char *source_region(SLoc beg, SLoc end) {
  if (beg.row < 1 || beg.row > g_source_info.content.size() ||
      end.row < 1 || end.row > g_source_info.content.size() ||
      beg.row > end.row)
    return "";

  return source_lines(beg.row, end.row);
}

char *source_buffer() {
  return &g_source_info.buffer[0];
}

void *set_new() {
  std::set<void *> *set = new std::set<void *>;
  return static_cast<void *>(set);
}

void set_insert(void *handle, void *item) {
  std::set<void *> *set = static_cast<std::set<void *> *>(handle);
  set->insert(item);
}

int set_exists(void *handle, void *item) {
  std::set<void *> *set = static_cast<std::set<void *> *>(handle);
  if (set->find(item) != set->end())
    return true;

  return false;
}

void set_drop(void *handle) {
  std::set<void *> *set = static_cast<std::set<void *> *>(handle);
  delete set;
}

GeneralRange *general_range_init(GeneralRange *gr, short inclusive,
				 struct ASTNode *start, struct ASTNode *end) {
  gr->inclusive = inclusive;
  gr->start = start;
  gr->end = end;

  GeneralRangeType type = FullRange;
  if (!start && !end)
    gr->type = FullRange;
  else if (start && end)
    gr->type = inclusive ? InclusiveRange : RightExclusiveRange;
  else if (start)
    gr->type = RangeFrom;
  else
    gr->type = inclusive ? InclusiveRangeTo : RightExclusiveRangeTo;

  return gr;
}

END_EXTERN_C

