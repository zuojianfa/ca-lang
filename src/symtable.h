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

#ifndef __symtable_h__
#define __symtable_h__

#include <stdint.h>
#include <stdio.h>

#include "ca_types.h"

#ifdef __cplusplus
BEGIN_EXTERN_C
#endif

#define MAX_SYMS 1024

/* source code location */
typedef struct SLoc {
  int row; /* row or lineno */
  int col; /* column */
} SLoc;

typedef enum SymType {
  Sym_Variable,
  Sym_Label,
  Sym_Label_Hanging,
  Sym_ArgList,
  Sym_FnDecl,
  Sym_FnDef,
  Sym_DataType,
  Sym_Member,
  Sym_TraitDef,
  Sym_TraitImpl,
} SymType;

#define MAX_ARGS 16

typedef enum CADTStatus {
  CADT_None,    // not normalized
  CADT_Orig,    // primitive type, no need expanding or compacting
  CADT_Expand,  // normalized by expanding
  CADT_Compact, // normalized by compacting
} CADTStatus;

typedef struct CADataType {
  tokenid_t type;       // type type: I16 I32 I64 ... STRUCT ARRAY, RANGE, SLICE
  typeid_t formalname; // type name symname_xxx
  size_t size;       // type size
  typeid_t signature;  // the signature of the type, which is used to avoid store multiple instance, it used in the symbol table
  CADTStatus status;   // only when status is not None, the signature can be used directly
  union {
    struct CAStruct *struct_layout;  // when type is STRUCT, TUPLE, SLICE
    struct CAArray *array_layout;    // when type is ARRAY,
    struct CAPointer *pointer_layout;// when type is POINTER
    struct CARange *range_layout;    // when type is RANGE, the llvm::Value can be of single type or a tuple type infact
  };
} CADataType;

// the inner form of slice can be of the form struct TSlice { T *ptr; int len; }
// or with 2 llvm::Value one ptr, one len
// when using structure llvm value:
// struct {
// Value *ptr;
// Value *len;
// } Value;
//
// when using 2 value
// Value *ptr;
// Value *len;
//
// here use CAStruct for SLICE, but limited the `fieldnum` to `2` and
// `fields[0].type` is pointer type to the sliced array or memory
// `fields[1].type` is the length type which always be i64
//typedef struct CASlice {
//  CADataType *ptr_type;
// CADataType *len_type; // can be make fixed u32 type
//} CASlice;

typedef enum GeneralRangeType {
  FullRange,                 // ..
  InclusiveRange,            // a..=b
  InclusiveRangeTo,          // ..=b
  RightExclusiveRange,       // a..b
  RightExclusiveRangeTo,     // ..b
  RangeFrom,                 // a..
} GeneralRangeType;

typedef struct CARange {
  GeneralRangeType type;
  int inclusive;
  CADataType *start;
  CADataType *end;
  CADataType *range; // when start end have value then range type is used, and it is a tuple type
} CARange;

typedef struct CAStructField {
  int name;           // field name
  size_t offset;      // field address offset to the struct beginning
  CADataType *type;   // field type
  //typeid_t type;
} CAStructField;

typedef enum CAStructType {
  Struct_NamedStruct, // 0: common structure
  Struct_NamedTuple,  // 1: tuple, 
  Struct_GeneralTuple,// 2: general tuple (unnamed)
  Struct_Slice,       // 3: slice
  Struct_Union,       // 4: union type
  Struct_Enum,        // 5: enum type, NOTE: enum type seems not suitable here
  Struct_General,     // n: can add new enum variant before this, this stand for the undetermined general struct form, like slice, enum, union etc
} CAStructType;

typedef struct CAStruct {
  CAStructType type; 
  int name;
  int fieldnum;
  int capacity;  // private
  int packed;    // 0: default, 1: pack 1, ...
  int fieldmaxalign;
  struct CAStructField *fields;
} CAStruct;

// the c language treat mutiple dimension array or typedef-ed array as the same
// array, e.g. typedef int aint[3][4]; typedef aint aaint[3]; aaint ca[6];
// (gdb) ptype ca:
// type = int [6][3][3][4]
//
// for pointer it is similar with array:
// typedef int *pint; typedef pint *ppint; typedef ppint *pppint; pppint *a;
// (gdb) ptype a
// type = int ****
//
// for pointer plus array type: aaint *pca[4]; aaint *pc;
// (gdb) ptype pca
// type = int (*[4])[3][3][4]
// (gdb) ptype pc
// type = int (*)[3][3][4]
// the dimension may need compact, ((int [3])[4])[5], after compact, int
// [3][4][5] complex condition combine array and pointer:
// typedef pppint apppint[5]; typedef apppint *papppint; papppint *ppap;
// (gdb) ptype ppap
// type = int ***(**)[5]
//
#define MAX_DIM 16
// for normalized array item the dimension always be 1, and the `type` can be
// another array. Using this method it is convenient to walk values

typedef struct CAArray {
  CADataType *type;   // array type
  int dimension;      // array size
  int dimarray[MAX_DIM];      // dimension array 3, 5, 9
} CAArray;

// the dimension need compact when constructing type, because
// e.g. ((int *) *) *a; after compact, it should be int ***a;
typedef enum CAPointerAllocPos {
  PP_Stack,
  PP_Heap,
} CAPointerAllocPos;

typedef struct CAPointer {
  CADataType *type;
  int dimension;
  CAPointerAllocPos allocpos;
} CAPointer;

typedef struct CAStringLit {
  int text;
  int len;
} CAStringLit;

typedef struct CAArrayLit {
  void *data;
} CAArrayLit;

typedef struct CAStructLit {
  void *data;
} CAStructLit;

typedef struct CAArrayExpr {
  // when == 0, just use the `data` as array elements
  // when  > 0, repeat the first element of `data` `repeat_count` times
  size_t repeat_count;
  void *data; // vector, each array element occupies one vector item
} CAArrayExpr;

typedef struct CAStructNamed {
  int name;
  struct ASTNode *expr;
} CAStructNamed;

typedef struct CAStructExpr {
  typeid_t name; // struct name
  int named;    // true: named field expression, false: unnamed expression
  void *data;    // struct fields expression vec_new handle
} CAStructExpr;

typedef struct CALiteral {
  // specify if literal type is defined (fixed) with postfix (u32,f64, ...).
  // indicate if the literal type is determined
  // if need remove this field, just use datatype null or not null for the existance
  int fixed_type;

  // -1 when have no postfix type, else have postfix type, like 33u32
  tokenid_t postfixtypetok;

  // the literal I64 for negative integer value, U64 for positive integer value,
  // F64 for floating point value, BOOL is true false value, I8 is 'x' value,
  // U8 is '\x' value
  tokenid_t littypetok;

  // text id in symname table, text is used for latering literal type inference
  int textid;

  // when the literal type already determined then datatype is not NULL 
  typeid_t datatype;

  // when littypetok is STRUCT ARRAY POINTER CSTRING, and fixed_type true, then it is used
  CADataType *catype;

  SLoc begloc; // the literal start location
  SLoc endloc; // the literal end location
  union {
    int64_t  i64value;      // store either signed or unsigned integer value
    double   f64value;      // store floating point value

    CAStringLit strvalue;      // string literal value
    CAStructLit structvalue;  // struct literal value
    CAArrayLit  arrayvalue;   // array literal value
  } u;
} CALiteral;

typedef struct LitBuffer {
  tokenid_t typetok;
  int len;
  int text;
} LitBuffer;

typedef struct IdToken {
  int symnameid;
  int typetok;
} IdToken;

typedef struct CAVariable {
  typeid_t datatype;
  SLoc loc;
  int name;
  int global; // is global variable

  // opaque memory, for storing llvm Value * type, used here for code generation, in code generation it will be filled and used, when type is Variable
  void *llvm_value;
} CAVariable;

typedef struct IdGroup {
  void *groups; // vec_new
} IdGroup;

typedef enum PatternType {
  PT_Var,
  PT_Array,
  PT_Tuple,
  PT_GenTuple,
  PT_Struct,
  PT_IgnoreOne,
  PT_IgnoreRange,
} PatternType;

typedef struct PatternGroup {
  int size;
  int capacity;
  struct CAPattern **patterns;
} PatternGroup;

typedef struct CAP_GenTuple {
  PatternGroup *items; // vec for CAPattern *
} CAP_GenTuple;

typedef struct CAPattern {
  enum PatternType type;
  typeid_t datatype;
  // used for locating the field of struct or tuple (with numeric field name),
  // only used in parent CAPattern to determine the child CAPattern's field position in catype, and do binding
  int fieldname;

  void *morebind; // vec int
  SLoc loc;
  int name;  // struct name or tuple name or variable name
  PatternGroup *items; // vec for CAPattern *
} CAPattern;

typedef struct ST_ArgList {
  int argc;                 // function argument count
  int contain_varg;         // contain variable argument
  union {
    int argnames[MAX_ARGS];   // function argument name
    typeid_t types[MAX_ARGS]; // for tuple type list
  };
  struct SymTable *symtable;
} ST_ArgList;

typedef struct ST_MemberList { // TODO: will use later
  struct ST_ArgList member;
  int visibility[MAX_ARGS];   // member visibilities
} ST_MemberList;

typedef struct ST_RunableList {
  void *opaque; // storing method / function list
} ST_RunableList;

typedef enum ArgType {
  AT_Literal,
  AT_Variable,
  AT_Expr,
} ArgType;

typedef struct ST_ArgListActual {
  int argc;                 // function argument count
  struct ASTNode *args[MAX_ARGS];  // function argument name
} ST_ArgListActual;

typedef struct CAVariableShielding {
  CAVariable *current;
  void *varlist;
} CAVariableShielding;

typedef struct TraitFnList {
  int trait_id;
  int count;
  void *data; // vector, eacho element is a function proto or function default implementation with ASTNode *
} TraitFnList, TTraitFnList;

typedef struct TypeImplInfo {
  int class_id;
  int trait_id;
  // the id of trait implementation for struct, created from class_id and trait_id, when is not
  // trait implementation, it's value is typeid_novalue. The id in this struct just for performance
  typeid_t trait_impl_id;
  // for control the common function recursive define count, when count > 0 then it is implement
  // common function, else implement the struct method, and they will use different name convention
  int fn_def_recursive_count;
} TypeImplInfo;

typedef int CAFunctionType;
#define CAFT_Function 0        // The general function
#define CAFT_Method 1          // The method impled for a type
#define CAFT_MethodForTrait 2  // The method impled for a trait
#define CAFT_MethodInTrait 4   // The default method impled for a trait
#define CAFT_GenericFunction 8 // The generic function

#define IS_GENERIC_FUNCTION(FUNC_TYPE) ((FUNC_TYPE) | CAFT_GenericFunction)

// for the labels the symbol name will append a prefix of 'l:' which is
// impossible to be as a variable name. example: l:l1
// for function it will append a prefix of 'f:'. example: f:fibs
typedef struct STEntry {
  int sym_name;		// symbol name index in global symbol table
  SymType sym_type;	// symbol type
  SLoc sloc;		// symbol definition line number and column
  union {
    struct {
      ST_ArgList *arglists; // when type is Sym_ArgList
      typeid_t rettype;
      typeid_t mangled_id;
      CAFunctionType ca_func_type;
      void *generic_types; // int vector, null when the function is generic typed function
    } f;                // when type is Sym_FnDef (Sym_ArgList and contains return type)
    //CAVariable *var;    // when sym_type are Sym_Variable Sym_Member
    CAVariableShielding varshielding;  // when sym_type are Sym_Variable Sym_Member
    //CADataType *datatype; // when sym_type is Sym_DataType
    struct {
      CAStructType tuple; // 1: when it is a tuple else it is a struct
      typeid_t id;  // when sym_type is Sym_DataType
      ST_ArgList *members; // when id is of struct type it have members, TODO: refactor it make it extensible to other complex type like enum etc.
      struct SymTable *idtable; // TODO: assign symtable value for the id, and use it when unwinding type, or it have bug when not use the symbol table in the type is defining, because when unwinding, it may find the symbol in other level scope
      struct ST_RunableList runables;
    } datatype;

    struct {
      struct ASTNode *node; // the node->type must be TTE_TraitFn
      void *trait_entry;
    } trait_def;                // when type is Sym_TraitDef

    struct {
      TypeImplInfo impl_info;
    } trait_impl;
  } u;
} STEntry;

typedef enum SymTableAssocType {
  STAT_Generic,
  STAT_Num,
} SymTableAssocType;

typedef struct SymTableAssoc {
  SymTableAssocType type;
  // assoc table is a table related to the struct plus a level of symbol table
  // which store Self or template parameter definitions
  struct SymTable *assoc_table;
  void *extra_id_list; // this is set type of std::set<int>
} SymTableAssoc;

typedef struct SymTable {
  void *opaque;

  // for association Symbol Table, some node travel need more than one symbol table
  // like generic type and trait method, because the node come from several part.
  // e.g. When implement struct with and use the trait method default implementation
  // one part is coming from the trait method symbol table, another part coming from
  // the struct method symbol table part. so here need association type.
  // when assoc is null, then just use the normal symbol table search path
  // when assoc is not null, then according to the assoc type to do the job.
  // the assoc just like a hook, when using it, it take affect, when not use, it will
  // not take affect
  SymTableAssoc *assoc;
  struct SymTable *parent;
} SymTable;

// The general range inner object, have no type definition grammar, just have
// inner type:
// when both start and end are NULL, then it is 

typedef struct GeneralRange {
  short type;  // GeneralRangeType
  short inclusive;
  struct ASTNode *start;
  struct ASTNode *end;
} GeneralRange;

typedef struct SymbolQueryParams {
  SymTable *st_normal;
  SymTable *st_extra;
  void *extra_id_list;
  int idx;
  int parent;
} SymbolQueryParams;

// parameter handling
// because function call can use embed form:
// let a = func1(1 + func2(2 + func3(3, 4))), so the actual argument list should
// use a stack struct for handling, just using function defined in symtable.h

// get current actual argument list object
ST_ArgListActual *actualarglist_current();

// create new actual argument list object and push into layer
ST_ArgListActual *actualarglist_new_push();

// popup a actual argument list object and destroy it
void actualarglist_pop();

ST_ArgList *tuplelist_current();
ST_ArgList *tuplelist_new_push();
void tuplelist_pop();

struct ASTNode *ifstmt_current();
struct ASTNode *ifstmt_new_push();
void ifstmt_pop(int isexpr);

// type checking
int check_i64_value_scope(int64_t lit, tokenid_t typetok);
int check_u64_value_scope(uint64_t lit, tokenid_t typetok);
int check_f64_value_scope(double lit, tokenid_t typetok);
int check_char_value_scope(char lit, tokenid_t typetok);
int check_uchar_value_scope(uint8_t lit, tokenid_t typetok);
int literal_type_convertable(tokenid_t from, tokenid_t to);
int as_type_convertable(tokenid_t from, tokenid_t to);

void set_litbuf(LitBuffer *litb, const char *text, int len, int typetok);
void set_litbuf_symname(LitBuffer *litb, int name, int len, int typetok);

CAArrayExpr arrayexpr_new();
CAArrayExpr arrayexpr_append(CAArrayExpr obj, struct ASTNode *expr);
size_t arrayexpr_size(CAArrayExpr obj);
struct ASTNode *arrayexpr_get(CAArrayExpr obj, int idx);
CAArrayExpr arrayexpr_fill(CAArrayExpr obj, struct ASTNode *expr, size_t n);

CAVariable *cavar_create(int name, typeid_t datatype);
CAVariable *cavar_create_with_loc(int name, typeid_t datatype, SLoc *loc);
CAVariable *cavar_create_self(int name);
void cavar_destroy(CAVariable **var);

CAPattern *capattern_new(int name, PatternType type, PatternGroup *pg);
void capattern_delete(CAPattern *cap);

PatternGroup *patterngroup_new();
void patterngroup_reinit(PatternGroup *pg);
void patterngroup_delete(PatternGroup *pg);
PatternGroup *patterngroup_push(PatternGroup *pg, CAPattern *cap);
void patterngroup_pop(PatternGroup *pg);
CAPattern *patterngroup_top(PatternGroup *pg);
CAPattern *patterngroup_at(PatternGroup *pg, int i);

// the globally symbol name table, it store names and it's index
int symname_init();
int symname_insert(const char *name);
int symname_check(const char *name);
int symname_check_insert(const char *name);
const char *symname_get(int pos);
const char *sg(int pos);

int sym_init(SymTable *t, SymTable *parent);
STEntry *sym_check_insert(SymTable *st, int encode, SymType type);
int sym_check_insert_withname(SymTable *t, const char *name, SymType type);

// insert without check
STEntry *sym_insert(SymTable *st, int encode, SymType type);
int sym_dump(SymTable *st, FILE *file);

// parent: if search parent symtable
SymTable *symtable_get_with_assoc(SymTable *symtable, int idx);
STEntry *sym_getsym_with_symtable(SymTable *st, int idx, int parent, SymTable **entry_st);
STEntry *sym_getsym(SymTable *st, int idx, int parent);
STEntry *sym_getsym_st2_with_symtable(SymbolQueryParams *params, SymTable **entry_st);
STEntry *sym_getsym_st2(SymbolQueryParams *params);
STEntry *sym_gettypesym_by_name(SymTable *st, const char *name, int parent);
STEntry *sym_get_function_entry_for_domainfn(struct ASTNode *name, struct ASTNode *args, STEntry **cls_entry, int *fnname);
STEntry *sym_get_function_entry_for_method_novalue(struct ASTNode *name, struct CADataType **struct_catype);
void runable_add_entry(TypeImplInfo *impl_info, STEntry *cls_entry, int fnname, int fnname_mangled, STEntry *nameentry);
struct MethodImplInfo *runable_find_entry(STEntry *cls_entry, int fnname, int trait_name);
int runable_is_method_in_struct(STEntry *cls_entry, int fnname);
void runable_add_entry_assoc(TypeImplInfo *impl_info, STEntry *cls_entry, int fnname, SymTableAssoc *assoc);
SymTableAssoc *runable_find_entry_assoc(STEntry *cls_entry, int fnname, int trait_name);
int sym_tablelen(SymTable *t);
SymType sym_gettype(SymTable *t, int idx, int parent);
SLoc sym_getsloc(SymTable *t, int idx, int parent);
void sym_setsloc(SymTable *st, int idx, SLoc loc);
int sym_is_sub_symtable(SymTable *sub, SymTable *root);

SymTable *load_symtable(char *buf, int len);
void sym_destroy(SymTable *t);
SymTable *sym_parent_or_global(SymTable *symtable);

int lexical_init();
int find_lexical_keyword(const char *name);

// create a string buffer
void *buffer_create();
void buffer_append(void *handle, const char *text, int len);
void buffer_append_char(void *handle, int ch);
// finished the buffer and return symname
const char *buffer_end(void *handle, int *len);
int buffer_binary_end(void *handle, int *len);

void *vec_new();
void vec_append(void *handle, void *item);
void *vec_front(void *handle);
void vec_pushfront(void *handle, void *item);
void *vec_popfront(void *handle);
void *vec_popback(void *handle);
size_t vec_size(void *handle);
void *vec_at(void *handle, int index);
void vec_reverse(void *handle);

void *string_new();
void string_append(void *handle, const char *s);
void string_append_char(void *handle, int ch);
const char *string_c_str(void *handle);
void string_pop_back(void *handle);
void string_drop(void *handle);

void source_info_init(const char *srcpath);
const char *source_line(int lineno);
const char *source_lines(int linefrom, int lineto);
const char *source_region(SLoc beg, SLoc end);
char *source_buffer();

void *set_new();
void set_insert(void *handle, void *item);
int set_exists(void *handle, void *item);
void set_drop(void *handle);

GeneralRange *general_range_init(GeneralRange *gr, short inclusive,
                                 struct ASTNode *start, struct ASTNode *end);

// create trait defines entry data for convenient use
void *sym_create_trait_defs_entry(struct ASTNode *node);
SymTableAssoc *new_SymTableAssoc(SymTableAssocType type, SymTable *assoc_table);
void sym_assoc_add_item(SymTableAssoc *assoc, int value);
void free_SymTableAssoc(SymTableAssoc *assoc);

#ifdef __cplusplus
END_EXTERN_C
#endif

#endif

