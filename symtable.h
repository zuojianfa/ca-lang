#ifndef __symtable_h__
#define __symtable_h__

#include <stdint.h>
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
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
} SymType;

#define MAX_ARGS 16

typedef struct CADataType {
  int type;       // type type: I32 I64 ... STRUCT ARRAY
  int formalname; // type name symname_xxx
  int size;       // type size
  union {
    struct CAStruct *struct_layout;  // when type is STRUCT
    struct CAArray *array_layout;    // when type is ARRAY
  };
} CADataType;

struct CAStructField {
  int name;           // field name
  CADataType *type;   // field type
};

struct CAStruct {
  int fieldnum;
  struct CAStructField *fields;
};

struct CAArray {
  int size;           // array size
  CADataType *type;   // array type
};

typedef struct CALiteral {
  // specify if literal type is defined (fixed) with postfix (u32,f64, ...)
  int fixed_type;

  // when fixed_type is false, it stand for the type of the default literal value
  // when have no any context
  int intent_type;

  // the literal I64 for negative integer value, U64 for positive integer value,
  // F64 for floating point value, BOOL is true false value, CHAR is 'x' value,
  // UCHAR is '\x' value
  int littypetok;

  // the just being creating variable's type, used to guide the literal type
  int borning_var_type;

  int postfixtypetok;      // the postfix type when postfix is set

  // text id in symname table, text is used for latering literal type inference
  int textid;

  CADataType *datatype;
  union {
    int64_t  i64value;      // store either integer type value include unsigned
    double   f64value;      // store floating value
    void    *structvalue;
    void    *arrayvalue;
  } u;
} CALiteral;

typedef struct LitBuffer {
  int typetok;
  int len;
  int text;
} LitBuffer;

typedef struct IdToken {
  int symnameid;
  int typetok;
} IdToken;

typedef struct CAVariable {
  CADataType *datatype;
  int name;

  // opaque memory, for storing llvm Value * type, used here for code generation, in code generation it will be filled and used, when type is Variable
  void *llvm_value;
} CAVariable;

typedef struct ST_ArgList {
  int argc;                 // function argument count
  int contain_varg;         // contain variable argument
  int argnames[MAX_ARGS];   // function argument name
} ST_ArgList;

typedef enum ArgType {
  AT_Literal,
  AT_Variable,
} ArgType;

typedef struct ActualArg {
  ArgType type;
  struct STEntry *entry; // when type is AT_Variable it is used
  union {
    /* literal value, only value so not in symbol table */
    struct CALiteral litv;
    int symnameid; /* variable value */
  };
} ActualArg;

typedef struct ST_ArgListActual {
  int argc;                 // function argument count
  ActualArg args[MAX_ARGS]; // function argument name
} ST_ArgListActual;

// for the labels the symbol name will append a prefix of 'l:' which is
// impossible to be as a variable name. example: l:l1
// for function it will append a prefix of 'f:'. example: f:fibs
typedef struct STEntry {
  int sym_name;		// symbol name index in global symbol table
  SymType sym_type;	// symbol type
  SLoc sloc;		// symbol definition line number and column
  union {
    ST_ArgList *arglists; // when type is Sym_ArgList
    CAVariable *var;
  } u;
} STEntry;

typedef struct SymTable {
  void *opaque;
  struct SymTable *parent;
} SymTable;

// type checking
int check_i64_value_scope(int64_t lit, int typetok);
int check_u64_value_scope(uint64_t lit, int typetok);
int check_f64_value_scope(double lit, int typetok);
int check_char_value_scope(char lit, int typetok);
int check_uchar_value_scope(uint8_t lit, int typetok);

// type finding
int catype_init();
int catype_put_by_name(int name, CADataType *datatype);
CADataType *catype_get_by_name(int name);
int catype_put_by_token(int token, CADataType *datatype);
CADataType *catype_get_by_token(int token);
int catype_is_float(int typetok);

const char *get_type_string(int tok);
void create_literal(CALiteral *lit, int textid, int littypetok, int postfixtypetok);
void set_litbuf(LitBuffer *litb, char *text, int len, int typetok);
int def_lit_type(int typetok);

CAVariable *cavar_create(int name, CADataType *datatype);
void cavar_destroy(CAVariable **var);

// the globally symbol name table, it store names and it's index
int symname_init();
int symname_insert(const char *name);
int symname_check(const char *name);
int symname_check_insert(const char *name);
const char *symname_get(int pos);

int sym_init(SymTable *t, SymTable *parent);
STEntry *sym_check_insert(SymTable *st, int encode, SymType type);
int sym_check_insert_withname(SymTable *t, const char *name, SymType type);

// insert without check
STEntry *sym_insert(SymTable *st, int encode, SymType type);
int sym_dump(SymTable *st, FILE *file);

// parent: if search parent symtable
STEntry *sym_getsym(SymTable *st, int idx, int parent);
int sym_tablelen(SymTable *t);
SymType sym_gettype(SymTable *t, int idx, int parent);
SLoc sym_getsloc(SymTable *t, int idx, int parent);
void sym_setsloc(SymTable *st, int idx, SLoc loc);

SymTable *load_symtable(char *buf, int len);
void sym_destroy(SymTable *t);

int lexical_init();
int find_lexical_keyword(const char *name);

#ifdef __cplusplus
}
#endif

#endif

