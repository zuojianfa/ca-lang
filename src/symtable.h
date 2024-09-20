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

/**
 * @file The symbol table module - C interface file.
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

/*
 * @brief Represents the source code location.
 */
typedef struct SLoc {
  int row; /// row or lineno
  int col; /// column
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
  CADT_None,    /// not normalized
  CADT_Orig,    /// primitive type, no need expanding or compacting
  CADT_Expand,  /// normalized by expanding
  CADT_Compact, /// normalized by compacting
} CADTStatus;

typedef struct CADataType {
  tokenid_t type;      /// type type: I16 I32 I64 ... STRUCT ARRAY, RANGE, SLICE
  typeid_t formalname; /// type name symname_xxx
  size_t size;         /// type size
  typeid_t signature;  /// The signature of the type, used to avoid storing, multiple instances. It is utilized in the symbol table.
  CADTStatus status;   /// only if status is not None, the signature can be used directly
  union {
    struct CAStruct *struct_layout;  /// when type is STRUCT, TUPLE, SLICE
    struct CAArray *array_layout;    /// when type is ARRAY,
    struct CAPointer *pointer_layout;/// when type is POINTER
    struct CARange *range_layout;    /// when type is RANGE, the llvm::Value can be of single type or a tuple type infact
  };
} CADataType;

/**
 * The inner form of a slice can be represented in two ways:
 *
 * 1. As a structure:
 *    struct TSlice {
 *        T *ptr;  // Pointer to the sliced array or memory
 *        int len;  // Length of the slice
 *    };
 *
 * 2. Using two llvm::Value instances:
 *    - One for the pointer:
 *      Value *ptr;
 *    - One for the length:
 *      Value *len;
 *
 * When using a structure with llvm value:
 *    struct {
 *        Value *ptr;  // Pointer to the sliced array or memory
 *        Value *len;  // Length of the slice
 *    } Value;
 *
 * For the CAStruct representing SLICE, `fieldnum` is limited to `2`:
 * - `fields[0].type` is the pointer type to the sliced array or memory.
 * - `fields[1].type` is the length type, which is always i64.
 *
 * Typedefinition for CASlice:
 * typedef struct CASlice {
 *     CADataType *ptr_type;  // Type of the pointer
 *     CADataType *len_type;   // Length type (can be fixed to u32)
 * } CASlice;
 */
typedef enum GeneralRangeType {
  FullRange,                 /// ..
  InclusiveRange,            /// a..=b
  InclusiveRangeTo,          /// ..=b
  RightExclusiveRange,       /// a..b
  RightExclusiveRangeTo,     /// ..b
  RangeFrom,                 /// a..
} GeneralRangeType;

typedef struct CARange {
  GeneralRangeType type;
  int inclusive;
  CADataType *start;
  CADataType *end;

  /**
   * When both start and end have values, the range type is used,
   * and it is represented as a tuple type.
   */
  CADataType *range;
} CARange;

typedef struct CAStructField {
  int name;           /// field name
  size_t offset;      /// field address offset to the beginning of struct
  CADataType *type;   /// field type
  //typeid_t type;
} CAStructField;

typedef enum CAStructType {
  Struct_NamedStruct, /// 0: common structure
  Struct_NamedTuple,  /// 1: tuple,
  Struct_GeneralTuple,/// 2: general tuple (unnamed)
  Struct_Slice,       /// 3: slice
  Struct_Union,       /// 4: union type
  Struct_Enum,        /// 5: enum type, NOTE: enum type seems not suitable here

  /**
   * n: You can add a new enum variant before this.
   * This variant will represent an undetermined general struct form,
   * such as slice, enum, union, etc.
   */
  Struct_General,
} CAStructType;

typedef struct CAStruct {
  CAStructType type; 
  int name;
  int fieldnum;
  int capacity;  /// private
  int packed;    /// 0: default, 1: pack 1, ...
  int fieldmaxalign;
  struct CAStructField *fields;
} CAStruct;

/*
 * The C language treats multiple dimension arrays or typedef-ed arrays
 * as the same type. For example:
 *
 * typedef int aint[3][4];
 * typedef aint aaint[3];
 * aaint ca[6];
 *
 * In GDB:
 * (gdb) ptype ca
 * type = int [6][3][4]
 *
 * For pointers, the behavior is similar:
 * typedef int *pint;
 * typedef pint *ppint;
 * typedef ppint *pppint;
 * pppint *a;
 *
 * In GDB:
 * (gdb) ptype a
 * type = int ****
 *
 * For pointer plus array types:
 * aaint *pca[4];
 * aaint *pc;
 *
 * In GDB:
 * (gdb) ptype pca
 * type = int (*[4])[3][4]
 * (gdb) ptype pc
 * type = int (*)[3][4]
 *
 * Dimensions may need to be compacted, e.g., ((int [3])[4])[5] becomes
 * int [3][4][5].
 *
 * Complex conditions combining arrays and pointers:
 * typedef pppint apppint[5];
 * typedef apppint *papppint;
 * papppint *ppap;
 *
 * In GDB:
 * (gdb) ptype ppap
 * type = int ***(**)[5]
 */
#define MAX_DIM 16

/*
 * For normalized array items, the dimension is always 1,
 * and the `type` can be another array.
 * This method makes it convenient to traverse values.
 */
typedef struct CAArray {
  CADataType *type;   // array type
  int dimension;      // array size
  int dimarray[MAX_DIM];      // dimension array 3, 5, 9
} CAArray;

/*
 * The dimensions need to be compacted when constructing the type.
 * For example, ((int **) *) *a; should be compacted to int ***a;
 */
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
  /*
   * When the value is == 0, use `data` as the array elements.
   * When the value is > 0, repeat the first element of `data`
   * for `repeat_count` times.
   */
  size_t repeat_count;
  void *data; /// Vector: each array element occupies one item in the vector
} CAArrayExpr;

typedef struct CAStructNamed {
  int name;
  struct ASTNode *expr;
} CAStructNamed;

typedef struct CAStructExpr {
  typeid_t name; /// struct name
  int named;     /// true: named field expression, false: unnamed expression
  void *data;    /// the handle of struct fields expression
} CAStructExpr;

typedef struct CALiteral {
  /**
   * Specify if the literal type is defined (fixed) with a postfix
   * (e.g., u32, f64, etc.). Indicate whether the literal type
   * is determined.
   *
   * If this field needs to be removed, simply use a null or non-null
   * datatype to indicate its existence.
   */
  int fixed_type;

  /**
   * Equal to -1 if there is no postfix type; otherwise, it indicates
   * the presence of a postfix type, such as in 33u32.
   */
  tokenid_t postfixtypetok;

  /**
   * Literal types:
   * - I64: Represents negative integer values.
   * - U64: Represents positive integer values.
   * - F64: Represents floating-point values.
   * - BOOL: Represents true/false values.
   * - I8: Represents 'x' character values.
   * - U8: Represents '\x' values.
   */
  tokenid_t littypetok;

  /// Text id in the symname table; text is used for later literal type inference.
  int textid;

  /// When the literal type is already determined, the datatype is not NULL.
  typeid_t datatype;

  /// when littypetok is STRUCT ARRAY POINTER CSTRING, and fixed_type true, then the field is used
  CADataType *catype;

  SLoc begloc; /// the literal start location
  SLoc endloc; /// the literal end location
  union {
    int64_t  i64value;       /// store either a signed or unsigned integer value
    double   f64value;       /// store floating point value

    CAStringLit strvalue;    /// string literal value
    CAStructLit structvalue; /// struct literal value
    CAArrayLit  arrayvalue;  /// array literal value
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
  int global; /// is global variable

  /**
   * Opaque memory for storing LLVM Value* type.
   * Used during code generation; it will be filled and utilized
   * when the type is Variable.
   */
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

  /*
   * Used for locating the field of a struct or tuple (with numeric field names).
   * This is only utilized in the parent Pattern to determine the child
   * Pattern's field position in the type and to perform binding.
   */
  int fieldname;

  void *morebind; /// vec int
  SLoc loc;
  int name;  /// struct name, tuple name or variable name
  PatternGroup *items; /// vec for CAPattern *
} CAPattern;

typedef struct ST_ArgList {
  int argc;                   /// function argument count
  int contain_varg;           /// contain variable argument
  union {
    int argnames[MAX_ARGS];   /// function argument name
    typeid_t types[MAX_ARGS]; /// for tuple type list
  };
  struct SymTable *symtable;
} ST_ArgList;

// TODO: will use later
typedef struct ST_MemberList {
  struct ST_ArgList member;
  int visibility[MAX_ARGS];   /// member visibilities
} ST_MemberList;

typedef struct ST_RunableList {
  void *opaque; /// storing method / function list
} ST_RunableList;

typedef enum ArgType {
  AT_Literal,
  AT_Variable,
  AT_Expr,
} ArgType;

typedef struct ST_ArgListActual {
  int argc;                 /// function argument count
  struct ASTNode *args[MAX_ARGS];  /// function argument name
} ST_ArgListActual;

typedef struct CAVariableShielding {
  CAVariable *current;
  void *varlist;
} CAVariableShielding;

typedef struct TraitFnList {
  int trait_id;
  int count;

  /*
   * Vector: each element is a function prototype or a function's default
   * implementation with ASTNode* type.
   */
  void *data;
} TraitFnList, TTraitFnList;

typedef struct TypeImplInfo {
  int class_id;
  int trait_id;
  /*
   * The ID of the trait implementation for the struct, created from
   * class_id and trait_id. When it is not a trait implementation,
   * its value is typeid_novalue. The ID in this struct is used for
   * performance purposes.
   */
  typeid_t trait_impl_id;

  /*
   * Controls the common function recursive definition count.
   * When count > 0, it implements a common function;
   * otherwise, it implements the struct method.
   * They will use different naming conventions.
   */
  int fn_def_recursive_count;
} TypeImplInfo;

typedef int CAFunctionType;
#define CAFT_Function 0        /// The general function
#define CAFT_Method 1          /// The method impled for a type
#define CAFT_MethodForTrait 2  /// The method impled for a trait
#define CAFT_MethodInTrait 4   /// The default method impled for a trait
#define CAFT_GenericFunction 8 /// The generic function

#define IS_GENERIC_FUNCTION(FUNC_TYPE) ((FUNC_TYPE) | CAFT_GenericFunction)

/*
 * For labels, the symbol name will append a prefix of 'l:',
 * which cannot be used as a variable name. Example: l:l1.
 * For functions, it will append a prefix of 'f:'. Example: f:fibs.
 */
typedef struct STEntry {
  int sym_name;		/// symbol name index in global symbol table
  SymType sym_type;	/// symbol type
  SLoc sloc;		/// symbol definition line number and column
  union {
    struct {
      ST_ArgList *arglists; /// when type is Sym_ArgList
      typeid_t rettype;
      typeid_t mangled_id;
      CAFunctionType ca_func_type;
      void *generic_types; /// int vector, null when the function is generic typed
    } f;                   /// when type is Sym_FnDef (Sym_ArgList and contains return type)
    //CAVariable *var;     /// when sym_type are Sym_Variable Sym_Member
    CAVariableShielding varshielding;  // when sym_type are Sym_Variable Sym_Member
    //CADataType *datatype; /// when sym_type is Sym_DataType
    struct {
      CAStructType tuple;  /// 1: When it is a tuple type; otherwise, it is a struct
      typeid_t id;         /// when sym_type is Sym_DataType

      /// When id is of struct type, it has members.
      // TODO: Refactor to make it extensible to other complex types like enum, etc.
      ST_ArgList *members;

      /*
       * TODO: Assign symtable value for the id and use it when unwinding
       * the type. Not using the symbol table during type definition may
       * lead to bugs, as unwinding could find the symbol in another
       * scope level.
       */
      struct SymTable *idtable;
      struct ST_RunableList runables;
    } datatype;

    struct {
      struct ASTNode *node; /// the node->type must be TTE_TraitFn
      void *trait_entry;
    } trait_def;            /// when type is Sym_TraitDef

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
  /**
   * Assoc table is related to the struct plus a level of symbol table,
   * which stores Self or template parameter definitions.
   */
  struct SymTable *assoc_table;
  void *extra_id_list; /// this is a set type of std::set<int>
} SymTableAssoc;

/**
 * @brief The symbol table structure
 */
typedef struct SymTable {
  void *opaque;

  /**
   * For the association Symbol Table, some node traversal requires
   * more than one symbol table, such as with generic types and
   * trait methods, because the nodes come from several parts.
   * For example, when implementing a struct and using the trait
   * method's default implementation, one part comes from the
   * trait method symbol table, while another comes from the
   * struct method symbol table. Therefore, an association type
   * is needed.
   *
   * When assoc is null, use the normal symbol table search path.
   * When assoc is not null, the operation will depend on the
   * assoc type. The assoc acts like a hook: when used, it takes
   * effect; when not used, it does not take effect.
   */
  SymTableAssoc *assoc;
  struct SymTable *parent;
} SymTable;

// The general range inner object, have no type definition grammar, just have
// inner type:
// when both start and end are NULL, then it is

/*
 * The general range inner object has no type definition grammar, just an inner
 * type. When both start and end are NULL, then it is a general tuple.
 */
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

/*
 * Parameter handling:
 * Because function calls can use an embedded form, such as:
 * let a = func1(1 + func2(2 + func3(3, 4))), the actual argument
 * list should use a stack structure for handling. This utilizes
 * the function defined in symtable.h.
 */

/// Get current actual argument list object
ST_ArgListActual *actualarglist_current();

/// Create a new actual argument list object and push it onto the layer.
ST_ArgListActual *actualarglist_new_push();

/// Pop an actual argument list object and destroy it
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

/// The global symbol name table, it stores names and it's index
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

// parent: if searching parent symtable
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

