/* Type system design:
   string representation:
   internal primitive types: t:i8, t:i32, t:u8, t:u32, t:f32, t:bool, ...,
   user defined types: t:<typename>, t:abcd
   pointer types: t:*void, t:*i8, t:*i32, t:**[*i32; 100], t:*[**[*i32; 100];2], t:*atomic_t, t:*rect_t, t:**abc
   array types: t:[u64;12], t:[*i32; 100], t:[*[**[*i32; 100];2];3], t:[atomic_t,3], t:[*rect_t,6], t:[**abc, 8]
   struct & type defined types: t:atomic_t, t:rect_t, t:abc

   typeid_t representation:
   u8 id: xxx where "t:u8" == symname_get(xxx), ...

   the string and typeid_t representation is scope related, so it should associate with symbol table

   CADataType object:
   this is the unique object that stand for an datatype
   primitive type stored in catype map
   other type stored in symbol table, because they have scope

   about fillback of type:
   because exists type definition based on unknown type (undefined yet type), so need get a fillback list from
   unknown type, e.g. unknowntype, *unknowntype, **unknowntype, [unknowntype;3], *[unknowntype;4], etc. these
   type should form a list, and when unknown type determined, then should fill all the element in the lists
   put (unknowntype, CADataType(unknowntype)), ((*unknowntype, CADataType(*unknowntype))), ...
*/

#include "ca.h"
#include "strutil.h"

#include "ca_types.h"
#include "symtable.h"
#include "type_system.h"
#include "type_system_llvm.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Value.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/ir1.h"
#include <array>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string.h>
#include <string>
#include <utility>
#include <vector>

BEGIN_EXTERN_C
#include "ca.tab.h"
void yyerror(const char *s, ...);
END_EXTERN_C

#include <unordered_map>
#include <map>

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

extern int glineno;
extern int gcolno;
extern ir_codegen::IR1 ir1;

std::vector<CALiteral> *arraylit_deref(CAArrayLit obj);

// name to CADatatype map
std::unordered_map<typeid_t, CADataType *> s_symtable_type_map;
std::unordered_map<typeid_t, CADataType *> s_signature_type_map;
std::unordered_map<typeid_t, CADataType *> s_type_map;

// for handling post defined functions after the calling
std::unordered_map<typeid_t, void *> g_function_post_map;

// storing type signature to llvm type map for shorten the generation of named
// struct and speedup the map performance
std::unordered_map<typeid_t, llvm::Type *> g_llvmtype_map;

using namespace llvm;
std::unordered_map<std::string, int> s_token_primitive_map {
  {"void",   VOID},
  {"int",    I32},
  {"i32",    I32},
  {"i64",    I64},
  {"isize",  I64},
  {"uint",   U32},
  {"u32",    U32},
  {"u64",    U64},
  {"usize",  U64},
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

std::unordered_map<std::string, std::string> s_token_primitive_inner_map {
  {"void",   "void",},
  {"int",    "i32", },
  {"i32",    "i32", },
  {"i64",    "i64", },
  {"isize",  "i64", },
  {"uint",   "u32", },
  {"u32",    "u32", },
  {"u64",    "u64", },
  {"usize",  "u64", },
  {"float",  "f32", },
  {"f32",    "f32", },
  {"double", "f64", },
  {"f64",    "f64", },
  {"bool",   "bool",},
  {"i8",     "i8",  },
  {"char",   "i8",  },
  {"u8",     "u8",  },
  {"uchar",  "u8",  },
};

//std::unordered_map<typeid_t, CADataTypeList>()
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
  {"print",  DBGPRINT},
  {"dbgprint",  DBGPRINT},
  {"dbgprinttype",  DBGPRINTTYPE},
  {"goto",   GOTO},
  {"fn",     FN},
  {"extern", EXTERN},
  {"return", RET},
  {"let",    LET},
  {"...",    VARG},
  {"struct", STRUCT},
  {"type",   TYPE},
  {"as",     AS},
  {"sizeof", SIZEOF},
  {"typeof", TYPEOF},
  {"typeid", TYPEID},
  {"__zero_init__", ZERO_INITIAL},
};

static CADataType *catype_make_type(const char *name, int type, int size);

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

const char *get_inner_type_string_by_str(const char *name) {
  auto itr = s_token_primitive_inner_map.find(name);
  if (itr != s_token_primitive_inner_map.end())
    return itr->second.c_str();

  return name;
}

const char *get_inner_type_string(int id) {
  const char *name = symname_get(id);
  return get_inner_type_string_by_str(name);
}

static const char *get_type_string_common(int tok, bool forid) {
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
    // TODO: for all above using following one
    return forid ? get_inner_type_string_by_str("char") : "char";
  case UCHAR:
    return forid ? get_inner_type_string_by_str("uchar") : "uchar";
  case CSTRING:
    return "*i8";
  default:
    yyerror("bad type token: %d", tok);
    return nullptr;
  }
}

const char *get_type_string(int tok) {
  return get_type_string_common(tok, false);
}

const char *get_type_string_for_signature(int tok) {
  return get_type_string_common(tok, true);
}

typeid_t sym_form_type_id_from_token(tokenid_t tok) {
  char namebuf[16];
  const char *name = get_type_string_for_signature(tok);
  sprintf(namebuf, "t:%s", name);
  return symname_check_insert(namebuf);
}

tokenid_t sym_primitive_token_from_id(typeid_t id) {
  const char *name = symname_get(id);
  auto itr = s_token_primitive_map.find(name);
  if (itr != s_token_primitive_map.end())
    return itr->second;

  yyerror("line: %d, col: %d: get primitive type token failed", glineno, gcolno);
  return tokenid_novalue;
}

int catype_init() {
  CADataType *datatype;
  int name;
  datatype = catype_make_type("t:void", VOID, 0); // void
  datatype = catype_make_type("t:i32", I32, 4); // i32

  name = symname_check_insert("t:int");
  catype_put_primitive_by_name(name, datatype); // int

  catype_make_type("t:i64", I64, 8);            // i64

  datatype = catype_make_type("t:u32", U32, 4); // u32

  name = symname_check_insert("t:uint");
  catype_put_primitive_by_name(name, datatype); // uint

  catype_make_type("t:u64", U64, 8);            // u64

  datatype = catype_make_type("t:f32", F32, 4); // f32
  name = symname_check_insert("t:float");
  catype_put_primitive_by_name(name, datatype); // float

  datatype = catype_make_type("t:f64", F64, 8); // f64
  name = symname_check_insert("t:double");
  catype_put_primitive_by_name(name, datatype); // double

  catype_make_type("t:bool", BOOL, 1);          // bool
  datatype = catype_make_type("t:i8", CHAR, 1); // i8
  name = symname_check_insert("t:char");
  catype_put_primitive_by_name(name, datatype); // char

  datatype = catype_make_type("t:u8", UCHAR, 1);// u8
  name = symname_check_insert("t:uchar");
  catype_put_primitive_by_name(name, datatype); // uchar

  return 0;
}

int catype_put_primitive_by_name(typeid_t name, CADataType *datatype) {
  s_type_map.insert(std::move(std::make_pair(name, datatype)));
  return 0;
}

CADataType *catype_get_primitive_by_name(typeid_t name) {
  auto itr = s_type_map.find(name);
  if (itr == s_type_map.end())
    return nullptr;

  return itr->second;
}

int catype_put_primitive_by_token(tokenid_t token, CADataType *datatype) {
  // TODO: this function should never used
  typeid_t name = sym_form_type_id_from_token(token);
  catype_put_primitive_by_name(name, datatype);
  return 0;
}

CADataType *catype_get_primitive_by_token(tokenid_t token) {
  typeid_t name = sym_form_type_id_from_token(token);
  CADataType *dt = catype_get_primitive_by_name(name);
  return dt;
}

#define out_source stderr
//#define out_source stdout

void debug_print_lmbrace(int num) {
    while (num > 0) {
    fprintf(out_source, "[");
    --num;
  }
}

void debug_print_star(int num) {
    while (num > 0) {
    fprintf(out_source, "*");
    --num;
  }
}

void debug_print_tab(int num) {
  while (num > 0) {
    fprintf(out_source, "\t");
    --num;
  }
}

// pointer: *type
// array: [
//            [
//                i32;
//                4
//            ];
//            6
//        ]
//
// struct: {0xxxxx
//             a: i32,
//             b: [
//                    i32;
//                    5
//                ],
//             c: {0xxxxx
//                    a: i32
//                },
//         }
//
void debug_catype_datatype_aux(const CADataType *datatype, std::set<const CADataType *> &accessed, int identhead, int ident) {
  if (accessed.find(datatype) != accessed.end()) {
    // already accessed, only print address of the datatype
    debug_print_tab(identhead);
    fprintf(out_source, "%s@%p", symname_get(datatype->formalname), datatype);
    return;
  }

  switch(datatype->type) {
  case POINTER:
    debug_print_tab(identhead);
    debug_print_star(datatype->pointer_layout->dimension);
    debug_catype_datatype_aux(datatype->pointer_layout->type, accessed, 0, ident);
    break;
  case ARRAY:
    debug_print_tab(identhead);
    debug_print_lmbrace(datatype->array_layout->dimension);
    debug_catype_datatype_aux(datatype->array_layout->type, accessed, 0, ident);
    for (int i = 0; i < datatype->array_layout->dimension; ++i) {
      fprintf(out_source, "; %d]", datatype->array_layout->dimarray[i]);
      //debug_print_tab(ident + datatype->array_layout->dimension - i);
      //fprintf(out_source, "]");
    }
    break;
  case STRUCT:
    debug_print_tab(identhead);
    fprintf(out_source, "{%s@%p\n", symname_get(datatype->formalname), datatype);
    accessed.insert(datatype);
    for (int i = 0; i < datatype->struct_layout->fieldnum; ++i) {
      debug_print_tab(ident + 1);
      fprintf(out_source, "%s: ", symname_get(datatype->struct_layout->fields[i].name));
      debug_catype_datatype_aux(datatype->struct_layout->fields[i].type, accessed, 0, ident + 1);
      fprintf(out_source, ",\n");
    }

    debug_print_tab(ident);
    fprintf(out_source, "}");
    break;
  default:
    fprintf(out_source, "%s", get_type_string_for_signature(datatype->type));
    break;
  }
}

void debug_catype_datatype(const CADataType *datatype) {
  std::set<const CADataType *> accessed;
  debug_catype_datatype_aux(datatype, accessed, 0, 0);
  fprintf(out_source, "\n");
}

// type signature example:
// primitive: i8 i32 u32 f64 bool void ...
// pointer: *<anytype>, *i8 *u32 *f64 ...
// reference: &<anytype>, &i8 &u32 &f64 ...
// array: [<anytype>;N]
// struct: {Name; <name1>:<anytype>, <name2>:<anytype>, <name3>:<anytype>, }
// unnamed tuple: (<anytype>,<anytype>,<anytype>[,])
// named tuple: (Name; <anytype>,<anytype>,<anytype>[,]) # <anytype> can also include Name
// union: <Name; <name1>:<anytype>, <name2>:<anytype>, <name3>:<anytype>, >
// enum: #name, name=value, name#
// how to represent the recursive type?
// struct AA { a: *AA, b: {} }

// TODO: consider replace string style signature comparing with struct based, it
// means transfer the string signature stype into struct signature. But
// currently don't known if it have a higher performance.
// the signature is just used for indexing the type object, it can not be used
// for operation such as `->` not only because it have performance issue but
// also it have no the ability to know the exactly type in the recursive part do
// so when use the signature

static int catype_unwind_type_name(SymTable *symtable, const char *pch,
                                   const std::map<std::string, CADataType *> &prenamemap,
				   const std::set<std::string> &rcheckset,
                                   char *sigbuf, int &buflen, int *typesize,
				   CADataType **retdt = nullptr);

static int catype_unwind_type_array(SymTable *symtable, const char *pch,
				    const std::map<std::string, CADataType *> &prenamemap,
				    const std::set<std::string> &rcheckset,
                                    char *sigbuf, int &buflen, int *typesize,
				    CADataType **retdt = nullptr);

static int catype_unwind_type_struct(SymTable *symtable, const char *pch,
				     const std::map<std::string, CADataType *> &prenamemap,
				     const std::set<std::string> &rcheckset,
                                     char *sigbuf, int &buflen, int *typesize,
				     CADataType **retdt = nullptr);

static int str_assign_while(char *sigbuf, const char *&pch, int ch) {
  int i = 0;
  while(*pch == ch) {
    sigbuf[i++] = *pch++;
  }

  return i;
}

// buflen: input/output parameter, input for sigbuf size, output for used size
// typesize: when `typesize` is not NULL, it will store the typesize, return -1
// when have type is unlimited type
// prenamemap: for checking if the name already appear in upper tree
//   it always inherit values from parent.
// rcheckset: for checking if exists recursive.
//   when encouner '*' '&' it will discard (not use) the values from parent
// retdt: when retdt not null then return the CADataType object associated with
// signature, notice: the returned object `retdt` need normalized to become
// compacted.
// but when the object created directly from unwinded string, it not need to do
// the normalize, because the unwinded string is already normalized
//
// RETURN: -1: when error,  0 or positive value.the consumption index

// TODO: for CADataType design and fill back the signature
static int catype_unwind_type_signature_inner(SymTable *symtable, const char *caname,
					      const std::map<std::string, CADataType *> &prenamemap,
					      const std::set<std::string> &rcheckset,
					      char *sigbuf, int &buflen, int *typesize,
					      CADataType **retdt = nullptr)
{
  // t:i32 => return
  // t:*i32 t:**i32 =>
  // t:[i32;3]
  // type A = **B; type B = **[*i32;3] t:A => t:**B; t:

  // const char *tname = symname_get(name);
  // if (tname[0] == 'T' && tname[1] == ':') {
  //   ASTNode *expr = astnode_unwind_from_addr(tname+2);
  //   name = inference_expr_type(expr);
  // }

  // the unwinding algorithm according to the upper type definitions
  int i = 0;
  int sigi = 0;
  int ret = -1;
  char namebuf[128];
  const char *pch = caname;
  STEntry *entry = nullptr;
  CADataType *dt = nullptr;
  CADataType *addrdt = nullptr;
  CADataType **outdt = nullptr;
  int tbuflen = 0;
  int tmptypesize = 0;
  int sizefin = 0;
  *typesize = -1;
  std::set<std::string> rcheckseti;
  const std::set<std::string> *prcheckset;

  while(*pch) {
    switch (*pch) {
    case '*':
      // handling pointer: *AA
      ret = str_assign_while(sigbuf, pch, '*');
      sigi += ret;
      *typesize = sizeof(void *);
      sizefin = 1;

      if (retdt) {
	addrdt = catype_make_type_symname(typeid_novalue, POINTER, sizeof(void *));
	addrdt->pointer_layout = new CAPointer;
	//dt->pointer_layout->type = datatype;
	addrdt->pointer_layout->dimension = ret;
      }
      break;
    case '&':
      // handling reference: &AA
      // TODO: when implementing reference type, also for CADataType
      // & can reprensent the reference type or the get address operator, here
      // when in type name it must be the reference type
      sigi += str_assign_while(sigbuf, pch, '&');
      *typesize = sizeof(void *);
      sizefin = 1;
      break;
    default:
      tbuflen = buflen - sigi;
      if (sizefin) {
	// when type already determined (* & case), just use new check set
	prcheckset = &rcheckseti;
      } else {
	// inherit the parent recursive check set
	prcheckset = &rcheckset;
      }

      outdt = retdt ? &dt : nullptr;

      switch(*pch) {
      case '[':
	// handling array: [AA;N], [[AA];N1];N2], ...
	ret = catype_unwind_type_array(symtable, pch, prenamemap, *prcheckset, sigbuf + sigi, tbuflen, &tmptypesize, outdt);
	break;
      case '{':
	// handling structure: {Name; <name1>:<anytype>, <name2>:<anytype>, <name3>:<anytype>, }
	ret = catype_unwind_type_struct(symtable, pch, prenamemap, *prcheckset, sigbuf + sigi, tbuflen, &tmptypesize, outdt);
	break;
      case '(':
	// TODO: handling tuple - named or unnamed: (<anytype>,<anytype>,<anytype>[,])
	ret = -1;
	break;
      case '<':
	// TODO: handling union: <Name; <name1>:<anytype>, <name2>:<anytype>, <name3>:<anytype>, >
	ret = -1;
	break;
      case '#':
	// TODO: handling enum: #AA, BB, ... #
	ret = -1;
	break;
      default:
	ret = catype_unwind_type_name(symtable, pch, prenamemap, *prcheckset, sigbuf + sigi, tbuflen, &tmptypesize, outdt);
	break;
      }

      if (ret == -1) {
	yyerror("unwind type name `%s` failed", pch);
	return -1;
      }

      buflen = sigi + tbuflen;
      // when size already determined (when pointer or reference type) or size calculation failed
      if (!sizefin || tmptypesize == -1)
	*typesize = tmptypesize;

      if (retdt) {
	if (addrdt) {
	  addrdt->pointer_layout->type = *outdt;
	  *retdt = addrdt;
	} else {
	  *retdt = *outdt;
	}

        (*retdt)->signature = sym_form_type_id_by_str(sigbuf);
      }

      return (pch - caname) + ret;
    }
  }

  return -1;
}

static void castruct_add_member(CAStruct *castruct, int name, CADataType *dt) {
  if (castruct->fieldnum >= castruct->capacity) {
    CAStructField *fields = new CAStructField[castruct->capacity * 2];
    for (int i = 0; i < castruct->capacity; ++i)
      fields[i] = castruct->fields[i];

    delete[] castruct->fields;
    castruct->fields = fields;
    castruct->capacity *= 2;
  }

  castruct->fields[castruct->fieldnum].name = name;
  castruct->fields[castruct->fieldnum].type = dt;
  castruct->fieldnum++;
}

static int catype_unwind_type_struct(SymTable *symtable, const char *pchbegin,
				     const std::map<std::string, CADataType *> &prenamemap,
				     const std::set<std::string> &rcheckset,
				     char *sigbuf, int &buflen, int *typesize,
				     CADataType **retdt) {
  const char *pch = pchbegin;
  int sigi = 0;
  sigbuf[sigi++] = *pch++; // = '{';
  int i = 0;
  int tmptypesize = 0;
  int sizeerror = 0;
  int calcing = 0;
  *typesize = -1;

  char namebuf[128];
  while(isalnum(*pch)) {
    sigbuf[sigi++] = *pch;
    namebuf[i++] = *pch++;
  }
  namebuf[i] = '\0';

  if (i == 0) {
    yyerror("(internal) this struct type have no name `%s`", pch);
    return -1;
  }

  if (*pch != '}') {
    if (*pch != ';') {
      yyerror("(internal) bad format of struct `%s` != ';' or '}'", pch);
      return -1;
    }

    sigbuf[sigi++] = *pch++; // = ';';
  }

  std::map<std::string, CADataType *> namemap = prenamemap;
  std::set<std::string> checkset = rcheckset;
  checkset.insert(namebuf);

  CADataType *dt = nullptr;
  CADataType *addrdt = nullptr;
  CAStruct *castruct = nullptr;
  if (retdt) {
    int nameid = symname_check_insert(namebuf);
    addrdt = catype_make_type_symname(nameid, STRUCT, *typesize);
    castruct = new CAStruct;
    castruct->name = nameid;
    castruct->fieldnum = 0;
    castruct->capacity = 10;
    castruct->fields = new CAStructField[castruct->capacity];
    addrdt->struct_layout = castruct;
    namemap.insert(std::make_pair(namebuf, addrdt));
    *retdt = addrdt;
  }

  namemap.insert(std::make_pair(namebuf, addrdt));

  while (*pch != '}') {
    int tsize = 0;
    i = 0;
    while(isalnum(*pch)) {
      namebuf[i] = *pch;
      sigbuf[sigi++] = *pch++;
      ++i;
    }
    namebuf[i] = 0;

    if (i == 0) {
      yyerror("(internal) this struct type member have no name `%s`", pch);
      return -1;
    }

    if (*pch != ':') {
      yyerror("(internal) bad format of struct `%s` != ':'", pch);
      return -1;
    }

    sigbuf[sigi++] = *pch++; // = ':';

    int tbuflen = buflen - sigi;

    CADataType **outdt = retdt ? &dt : nullptr;
    int ret = catype_unwind_type_signature_inner(symtable, pch, namemap, checkset, sigbuf+sigi, tbuflen, &tsize, outdt);
    if (ret == -1) {
      yyerror("unwind type `%s` failed");
      return -1;
    }

    pch += ret;
    sigi += tbuflen;

    if (*pch == ',')
      sigbuf[sigi++] = *pch++; // = ',';

    if (tsize == -1)
      sizeerror = 1;
    else if (tsize == -2)
      calcing = 1;
    else
      tmptypesize += tsize;

    if (retdt)
      castruct_add_member(castruct, symname_check_insert(namebuf), *outdt);
  }

  sigbuf[sigi++] = *pch++; // = '}';
  buflen = sigi;

  if (sizeerror)
    *typesize = -1;
  else if (calcing)
    *typesize = -2;
  else
    *typesize = tmptypesize;

  // TODO: fillback typeid signature when possible or need to use these information
  if (retdt)
    addrdt->size = *typesize;

  return pch - pchbegin;
}

static int catype_unwind_type_array(SymTable *symtable, const char *pchbegin,
				    const std::map<std::string, CADataType *> &prenamemap,
				    const std::set<std::string> &rcheckset,
				    char *sigbuf, int &buflen, int *typesize,
				    CADataType **retdt) {
  const char *pch = pchbegin;
  int sigi = 0;
  int tmptypesize = 0;
  *typesize = -1;

  sigbuf[sigi++] = *pch++; // should be '['
  int tbuflen = buflen - sigi;

  CADataType *dt = nullptr;
  CADataType *addrdt = nullptr;
  CADataType **outdt = retdt ? &dt : nullptr;
  int ret = catype_unwind_type_signature_inner(symtable, pch, prenamemap, rcheckset, sigbuf+sigi, tbuflen, &tmptypesize, outdt);
  if (ret == -1) {
    yyerror("unwind type `%s` failed");
    return -1;
  }

  pch += ret;
  sigi += tbuflen;
  if (*pch != ';') {
    yyerror("(internal) ';' is expected but found '%c'", *pch);
    return -1;
  }
  sigbuf[sigi++] = *pch++; // = ';'

  int i = 0;
  char numbuf[16];
  while(isdigit(*pch)) {
    numbuf[i++] = *pch;
    sigbuf[sigi++] = *pch++;
  }
  numbuf[i] = '\0';

  if (i == 0) {
    yyerror("(internal) numeric value expected but not found");
    return -1;
  }

  if (*pch != ']') {
    yyerror("(internal) ']' is expected but found '%c'", *pch);
    return -1;
  }

  // TODO: the number can be used when form CADataType
  int elesize = atoi(numbuf);

  if (tmptypesize < 0)
    *typesize = tmptypesize;
  else
    *typesize = tmptypesize * elesize;

  sigbuf[sigi++] = *pch++; // = ']';
  buflen = sigi;

  if (retdt) {
    addrdt = catype_make_type_symname(typeid_novalue, ARRAY, *typesize);
    addrdt->array_layout = new CAArray;
    addrdt->array_layout->type = *outdt;
    addrdt->array_layout->dimension = 1;
    addrdt->array_layout->dimarray[0] = elesize;
    *retdt = addrdt;
  }

  return pch - pchbegin;
}

// when the struct type already unwinded then just append the name
// in struct different members when later member already exist in previous
// and use it, this strategy can only compare in strict mode, unstrict mode will
// need expand type in all struct member's even when previous member already expanded
// the same type. e.g.
// struct T1 { a: A, b: B, c: A}
// struct T2 { a: A, b: B, c: B}
// when the previous type A and B unwinded but the last A and B not unwind,
// it will can not compare in non-strict mode, because don't known if type A equal B
// so also unwind when the type is not in the recursive scope, so also need
// check if one type is under the recursive scope
// RETURN: -1: when error,  0 or positive value, the consumption index
static int catype_unwind_type_name(SymTable *symtable, const char *pch,
				   const std::map<std::string, CADataType *> &prenamemap,
				   const std::set<std::string> &rcheckset,
                                   char *sigbuf, int &buflen, int *typesize,
				   CADataType **retdt)
{
  *typesize = -1;

  if (pch[0] == '+' && pch[1] == ':') {
    int len = 0;
    ASTNode *expr = astnode_unwind_from_addr(pch, &len);
    typeid_t name = inference_expr_type(expr);
    pch = catype_get_type_name(name);
    catype_unwind_type_signature_inner(symtable, pch, prenamemap, rcheckset, sigbuf, buflen, typesize, retdt);
    return len;
  }

  char namebuf[128];

  // handling a named type: AA
  int i = strutil_parse_ident_str(pch, namebuf);
  if (!i)
    return -1; // format error

  typeid_t id = sym_form_type_id_by_str(namebuf);
  CADataType *dt = catype_get_primitive_by_name(id);
  if (dt) {
    strcpy(sigbuf, namebuf);
    buflen = i;
    *typesize = dt->size;
    if(retdt)
      *retdt = dt;

    return i;
  }

  // find from symbol table which may store the type
  STEntry *entry = sym_gettypesym_by_name(symtable, namebuf, 1);
  if (!entry)
    return -1;

  if (rcheckset.find(namebuf) != rcheckset.end()) {
    // this is the recursive a type definition, type size is recursive, so may have unlimited size
    // TODO: return error number and the upper logic can print different error message
    fprintf(stderr, "type name `%s` is recursively defined: ", namebuf);
    return -1;
  }

  // when the name already processed, then the name here should be the recursive name
  auto itr = prenamemap.find(namebuf);
  if (itr != prenamemap.end()) {
    strcpy(sigbuf, namebuf);
    buflen = i;
    // when type already appear previous, -2 means the type size is calculating upper
    // and the type size of this layer depends on the upper case
    *typesize = -2;

    if (!itr->second) {
      yyerror("line: %d, col: %d: type `%s` should recursive defining", glineno, gcolno, namebuf);
      return -1;
    }

    if (retdt)
      *retdt = itr->second;

    return i;
  }

  std::map<std::string, CADataType *> namemap = prenamemap;
  std::set<std::string> checkset = rcheckset;
  checkset.insert(namebuf);

  std::string scaname = catype_get_type_name(entry->u.datatype.id);
  const char *caname = scaname.c_str();

  // when type is not a structure type directly
  if (!entry->u.datatype.members) {
    int tbuflen = buflen;

    // when ever retdt is null, it still use this
    namemap.insert(std::make_pair(namebuf, nullptr));
    int ret = catype_unwind_type_signature_inner(entry->u.datatype.idtable, caname, namemap, checkset, sigbuf, tbuflen, typesize, retdt);
    if (ret == -1) {
      yyerror("unwind type `%s` failed", caname);
      return -1;
    }

    if (*(caname+ret)) {
      yyerror("type contains extra text when unwind type `%s`", caname);
      return -1;
    }

    buflen = tbuflen;
    return i; // + ret;
  }

  // if it is struct type definition then the `namebuf` should identical to the `caname`,
  // so when check with new set (checkset and namemap) it will always find it because the upper
  // just added it, and when check with old set (rcheckset and prenamemap), it always cannot find
  // because previously already find one time, so the struct case is special, so no need do
  // following checking
  CADataType *addrdt = nullptr;
  CAStruct *castruct = nullptr;
  if (retdt) {
    int nameid = symname_check_insert(namebuf);
    addrdt = catype_make_type_symname(nameid, STRUCT, *typesize);
    castruct = new CAStruct;
    castruct->name = nameid;
    castruct->fieldnum = 0;
    castruct->capacity = 10;
    castruct->fields = new CAStructField[castruct->capacity];
    addrdt->struct_layout = castruct;
    *retdt = addrdt;
  }

  namemap.insert(std::make_pair(namebuf, addrdt));

  int tmptypesize = 0;
  int sizeerror = 0;
  int calcing = 0;

  int sigi = sprintf(sigbuf, "{%s;", caname);
  ST_ArgList *members = entry->u.datatype.members;
  for (int j = 0; j < members->argc; ++j) {
    int tsize = 0;
    const char *argname = symname_get(members->argnames[j]);
    STEntry *nameentry = sym_getsym(members->symtable, members->argnames[j], 0);
    if (nameentry->sym_type != Sym_Member) {
      yyerror("(internal) symbol type is not struct member for argument: `%s`", argname);
      return -1;
    }

    sigi += sprintf(sigbuf + sigi, "%s:", argname);
    const char *mtypename = catype_get_type_name(nameentry->u.var->datatype);
    int tbuflen = buflen - sigi;

    CADataType **outdt = retdt ? &dt : nullptr;
    int ret = catype_unwind_type_signature_inner(members->symtable, mtypename,
						 namemap, checkset, sigbuf + sigi, tbuflen, &tsize, outdt);
    if (ret == -1) {
      yyerror("unwind type `%s` failed");
      return -1;
    }

    sigi += tbuflen;
    sigbuf[sigi++] = ','; // = ',';

    if (tsize == -1)
      sizeerror = 1;
    else if (tsize == -2)
      calcing = 1;
    else
      tmptypesize += tsize;

    if (retdt)
      castruct_add_member(castruct, members->argnames[j], *outdt);
  }

  sigbuf[sigi-1] = '}'; // remove last ',' or ';' when it is empty struct
  sigbuf[sigi] = '\0';

  buflen = sigi;

  if (sizeerror)
    *typesize = -1;
  else if (calcing)
    *typesize = -2;
  else
    *typesize = tmptypesize;

  if (retdt)
    addrdt->size = *typesize;

  return i;
}

static int catype_calculate_typesize(const char *sigbuf, int len) {
  return 0;
}

typeid_t catype_unwind_type_signature(SymTable *symtable, typeid_t name, int *typesize, CADataType **retdt) {
  char sigbuf[4096] = "t:";

  std::map<std::string, CADataType *> prenamemap;
  std::set<std::string> recursive_check_set;

  const char *caname = catype_get_type_name(name);
  int len = 4096 - 2;
  int ret = catype_unwind_type_signature_inner(symtable, caname, prenamemap, recursive_check_set, sigbuf+2, len, typesize, retdt);
  if (ret == -1) {
    yyerror("unwind type signature `%s` failed", caname);
    return typeid_novalue;
  }

  sigbuf[len+2] = '\0';

  // fixing the signature, the inner function may not set the signature correctly
  typeid_t signature = symname_check_insert(sigbuf);
  if (retdt && *retdt) {
    (*retdt)->signature = signature;
    (*retdt)->status = CADT_Expand;
  }

  return signature;
}

// skeleton, member-name, type-name
enum DataTypeCmpLevel {
  DTCL_Skeleton, // (0 0) the type skeleton must be the same, other factor can
                 // be different, can compare 2 type's member layout, when same
                 // means layout same
  DTCL_TypeName, // (0 1) the type skeleton, type name must be the same, struct
                 // member name can be different, cannot keep program rightly
                 // for use different name
  DTCL_Member, // (1 0) the type skeleton, struct member name must be the same,
                // type name can be different, can keep program rightly, ca
                // should use this model
  DTCL_Strict, // (1 1) the type skeleton, type name, struct member name must be
               // the same, like C
};

// return true when strictly compare type name
static int catypeaux_strict_typename(DataTypeCmpLevel level) {
  return level == DTCL_TypeName || level == DTCL_Strict;
}

// return true when strictly compare member name
static int catypeaux_strict_membername(DataTypeCmpLevel level) {
  return level == DTCL_Member || level == DTCL_Strict;
}

// compare 2 types' signature
// return value: 1: same, 0: not same, -1: when error (e.g. format issue)
// e.g.
// {A24;v1:{A10;b3:{A3;a:[f32;64]}},v2:{A21;c3:[{A18;c3:[{A12;b3:[{A3;a:[f32;64]};32]};32]};32]}}
// notice that: using namemap comparing the data type skeleton, it can only
// compare when the type name order is exchanged, cannot compare if the type
// name real have the same member layout, so it can used in a cache, such as
// data layout cache and when cache not hit then reconstruct the cache
static int catype_compare_type_by_signature_str(std::map<std::string, std::string> &prenamemap,
						const char *caname1, const char *caname2,
						DataTypeCmpLevel cmplevel)
{
  if (cmplevel == DTCL_Strict)
    return !strcmp(caname1, caname2);

  // non strict compare will ignoring the name of structure enum etc.
  std::map<std::string, std::string> namemap;

  // when ignoring type name compare only consider the name after '{' which is
  // the struct name, for primitive type name they should do strict compare.
  // although `rust` can redifinition primitive type but `ca` not support that

  const char *pch1 = caname1;
  const char *pch2 = caname2;

  char namebuf1[1024];
  char namebuf2[1024];
  int i = 0, j = 0;

  int stricttype = catypeaux_strict_typename(cmplevel);
  int strictmember = catypeaux_strict_membername(cmplevel);

  // here not parse the signature structure, just using simple algorithm to do
  // the job and without type validation checking, assuming the input is verified
  // with different prefix character, there are different actions, the core idea
  // is coping with id names, and identify if it is an type name or struct member
  // name:
  // '{': next is type name
  // ';': next is struct member name when is not a numeric
  // ',': next is struct member name
  // or when encounter an alphabetic char, then look back the previous char, and
  // according to the char to do actions:
  // '{': encounter a new name that not in the name map, then compare type name
  // ';' or ',': then compare struct member name
  // else: the follower is either a primitive type name or the struct name
  // previously already accessed

  if (isalpha(*pch1) && isalpha(*pch2))
    return !strcmp(pch1, pch2);

  while (true) {
    while (!isalpha(*pch1) && *pch1 != '\0' && *pch1 == *pch2) {
      pch1++;
      pch2++;
    }

    // check if anyone scanned over
    if (*pch1 == '\0' && *pch2 == '\0')
      return 1;
    else if (*pch1 == '\0' || *pch2 == '\0') // one over, one not over
      return 0;

    if (!isalpha(*pch1) || !isalpha(*pch2)) // means *pch1 != *pch2
      return 0;

    // handling alphabetic case
    i = strutil_parse_alnum_str(pch1, namebuf1);
    j = strutil_parse_alnum_str(pch2, namebuf2);
    if (!i || !j)
      return -1; // data type format error

    // when not encounter a structure start, it must be not identical
    switch (*(pch1-1)) {
    case '{':
      // compare sub-datatype, first encountered
      if (stricttype) {
	if (strcmp(namebuf1, namebuf2))
	  return 0;
      } else {
	namemap.insert(std::make_pair(namebuf1, namebuf2));
      }
      break;
    case ';':
    case ',':
      // compare member name
      if (strictmember) {
	if (strcmp(namebuf1, namebuf2))
	  return 0;
      } // else do nothing, not compare name or use namemap
      break;
    default:
      {
	// compare sub-datatype, later encountered
	auto itr = namemap.find(namebuf1);
	if (itr != namemap.end()) {
	  if (strcmp(itr->second.c_str(), namebuf2))
	    return 0;
	} else {
	  // when not in map, it must be an primitive type, so just compare them
	  if (strcmp(namebuf1, namebuf2))
	    return 0;
	}
      }
      break;
    }

    pch1 += i; pch2 += j;
  }

  return -1;
}

// compare 2 types' signature, strict means if do strict compare
static int catype_compare_type_by_signature(std::map<std::string, std::string> &prenamemap,
					    typeid_t name1, typeid_t name2,
					    DataTypeCmpLevel cmplevel)
{
  const char *caname1 = catype_get_type_name(name1);
  const char *caname2 = catype_get_type_name(name2);
  return catype_compare_type_by_signature_str(prenamemap, caname1, caname2, cmplevel);
}

// calculate the type line's closure: all the type's CADataType object
// in that related to the type and insert into table with signature. e.g.
// struct A { b: B, c: C }
// struct B { c: C }
// struct C { a: *A }
// type AA = A;
// type BB = AA;
//
// typeid(A) == {A; b: {B; c: {C; a: *A}}, c: {C; a: *A}}
// typeid(B) == {B; c: {C; a: *{A; b: B, c: C }}}
// typeid(C) == {C; a: *{A; b: {B; c: C}, c: C}}
// typeid(AA) == typeid(A)
// typeid(BB) == typeid(A)
// when input is 'struct A' it related to type B and C, so here calculate
// all the typeid of A B and C and finally put them into s_type_map
//
void catype_make_type_closure(SymTable *symtable, typeid_t id) {
  // TODO:
  //catype_get_by_name();

}

// compact CADataType object of pointer * or array [ into one object
static CADataType *catype_formalize_type_compact(CADataType *datatype) {
  switch(datatype->type) {
  case POINTER:
    while (datatype->pointer_layout->type->type == POINTER) {
      auto *tmp = datatype;
      int dim = tmp->pointer_layout->dimension;
      datatype = datatype->pointer_layout->type;
      datatype->pointer_layout->dimension += dim;

      delete tmp->pointer_layout;
      delete tmp; // free memory of prevous type
    }

    datatype->pointer_layout->type =
      catype_formalize_type_compact(datatype->pointer_layout->type);

    return datatype;
  case ARRAY:
    while (datatype->array_layout->type->type == ARRAY) {
      auto *tmp = datatype->array_layout->type;
      datatype->array_layout->type = tmp->array_layout->type;
      for (int i = 0, j = datatype->array_layout->dimension;
	   i < tmp->array_layout->dimension;
	   ++i, ++j) {
	datatype->array_layout->dimarray[j] = tmp->array_layout->dimarray[i];
      }
      datatype->array_layout->dimension += tmp->array_layout->dimension;

      delete tmp->array_layout;
      delete tmp;
    }

    datatype->array_layout->type =
      catype_formalize_type_compact(datatype->array_layout->type);

    return datatype;
  case STRUCT:
    for (int i = 0; i < datatype->struct_layout->fieldnum; ++i) {
      auto *&type = datatype->struct_layout->fields[i].type;
      type = catype_formalize_type_compact(type);
    }

    return datatype;
  default: // primitive type
    return datatype;
  }
}

// t:**type => t:*type
static typeid_t typeid_decrease_pointer(typeid_t type) {
  const char *name = catype_get_type_name(type);
  if(name[0] != '*') {
    yyerror("Assertion `name[0] == '*'' failed");
    return typeid_novalue;
  }
  typeid_t id = sym_form_type_id_by_str(name+1);
  return id;
}

// t:*type => t:**type
static typeid_t typeid_increase_pointer(typeid_t type) {
  char buf[1024];
  const char *name = catype_get_type_name(type);
  buf[0] = '*';
  strcpy(buf+1, name);
  typeid_t id = sym_form_type_id_by_str(buf);
  return id;
}

// t:[[type;num1];num2] => t:[type;num1]
static typeid_t typeid_decrease_array(typeid_t type) {
  const char *name = catype_get_type_name(type);
  int len = strlen(name);
  assert(name[0] == '[' && name[len-1] == ']');
  const char *lastsemi = strrchr(name, ';');
  assert(lastsemi != nullptr);
  char buf[1024];
  len = lastsemi - name - 1;
  strncpy(buf, name+1, len);
  buf[len] = 0;
  typeid_t id = sym_form_type_id_by_str(buf);
  return id;
}

// t:[type;num1] => t:[[type;num1];num2]
static typeid_t typeid_increase_array(typeid_t type, int num) {
  const char *name = catype_get_type_name(type);
  char buf[1024];
  sprintf(buf, "[%s;%d]", name, num);
  typeid_t id = sym_form_type_id_by_str(buf);
  return id;
}

// only need get the top level signature
static typeid_t typeid_get_top_down(CADataType *catype, std::set<CADataType *> &rcheck) {
  if (catype->status != CADT_None)
    return catype->signature;

  typeid_t subid = typeid_novalue;
  typeid_t id = typeid_novalue;
  const char *structname = nullptr;
  const char *subname = nullptr;
  const char *varname = nullptr;
  char buf[1024];
  int i = 0;
  int len = 0;

  switch (catype->type) {
  case CSTRING:
  case POINTER:
    subid = typeid_get_top_down(catype->pointer_layout->type, rcheck);
    subname = catype_get_type_name(subid);
    for (i = 0; i < catype->pointer_layout->dimension; ++i)
      buf[i] = '*';

    strcpy(buf + i, subname);
    catype->signature = sym_form_type_id_by_str(buf);
    catype->status = CADT_Expand;
    return catype->signature;
  case ARRAY:
    subid = typeid_get_top_down(catype->array_layout->type, rcheck);
    subname = catype_get_type_name(subid);
    for (i = 0; i < catype->array_layout->dimension; ++i)
      buf[i] = '[';

    len = strlen(subname);
    strcpy(buf + i, subname);
    len += i;
    for (i = catype->array_layout->dimension - 1; i >= 0; --i)
      len += sprintf(buf+len, ";%d]", catype->array_layout->dimarray[i]);

    catype->signature = sym_form_type_id_by_str(buf);
    catype->status = CADT_Expand;
    return catype->signature;
  case STRUCT:
    // t:{BB;a:f32}, t:{AA;a:i32,b:i64,c:{BB;a:f32}}
    if (rcheck.find(catype) != rcheck.end()) {
      return sym_form_type_id(catype->struct_layout->name);
    }

    rcheck.insert(catype);

    buf[0] = '{';
    structname = symname_get(catype->struct_layout->name);
    len = strlen(structname) + 1;
    strcpy(buf+1, structname);
    buf[len++] = ';';
    for (int i = 0; i < catype->struct_layout->fieldnum; ++i) {
      auto *&type = catype->struct_layout->fields[i].type;

      //rcheck.insert(catype);
      subid = typeid_get_top_down(type, rcheck);
      //rcheck.erase(catype);

      varname = symname_get(catype->struct_layout->fields[i].name);
      subname = catype_get_type_name(subid);
      len += sprintf(buf+len, "%s:%s,", varname, subname);
    }

    rcheck.erase(catype);

    buf[len-1] = '}';
    buf[len] = '\0';
    catype->signature = sym_form_type_id_by_str(buf);
    catype->status = CADT_Expand;
    return catype->signature;
  case VOID:
  case I32:
  case I64:
  case U32:
  case U64:
  case F32:
  case F64:
  case BOOL:
  case CHAR:
  case UCHAR:
  default:
    return catype->signature;
  }
}

static typeid_t typeid_from_catype(CADataType *catype) {
  // used for check circle when contains struct type
  std::set<CADataType *> rcheck;
  return typeid_get_top_down(catype, rcheck);
}

// expand compacted * or array [ into separate CADataType object
static CADataType *catype_formalize_type_expand(CADataType *datatype, std::set<CADataType *> &rcheck) {
  CADataType *nextdt = nullptr;
  CADataType *currdt = datatype;
  int dim = 0;
  int *parray = nullptr;

  while (true) {
    switch (currdt->type) {
    case POINTER:
      // TODO: make the expanded (use clone following) signature formalname to correct one
      dim = currdt->pointer_layout->dimension;
      currdt->pointer_layout->dimension = 1;
      for (int i = 1; i < dim; ++i) {
	CADataType *dt = catype_clone_thin(currdt);
	dt->signature = typeid_decrease_pointer(currdt->signature);
	dt->status = CADT_Expand;
	currdt->pointer_layout->type = dt;
	currdt = dt;
      }

      nextdt = currdt->pointer_layout->type;
      if (nextdt->status == CADT_None) {
	nextdt->signature = typeid_decrease_pointer(currdt->signature);
	nextdt->status = CADT_Expand;
      }

      currdt = nextdt;
      break;
    case ARRAY:
      // TODO: make the expanded (use clone following) signature formalname to correct one
      dim = currdt->array_layout->dimension;
      parray = currdt->array_layout->dimarray;
      currdt->array_layout->dimension = 1;
      for (int i = 1; i < dim; ++i) {
	CADataType *dt = catype_clone_thin(currdt);
	dt->signature = typeid_decrease_array(currdt->signature);
	dt->status = CADT_Expand;
	dt->array_layout->dimarray[0] = parray[i];
	currdt->array_layout->type = dt;
	currdt = dt;
      }

      nextdt = currdt->array_layout->type;
      if (nextdt->status == CADT_None) {
	nextdt->signature = typeid_decrease_array(currdt->signature);
	nextdt->status = CADT_Expand;
      }

      currdt = nextdt;
      break;
    case STRUCT:
      // NEXT TODO: handle catype status
      for (int i = 0; i < currdt->struct_layout->fieldnum; ++i) {
	auto *&type = currdt->struct_layout->fields[i].type;
	// when the field already expanded then do nothing
	if (rcheck.find(type) != rcheck.end())
	  continue;

	rcheck.insert(type);
	if (type->status == CADT_None)
	  type->signature = typeid_from_catype(type);

        // type =
	catype_formalize_type_expand(type, rcheck);
	rcheck.erase(type);
      }

      return datatype;
    default: // primitive type
      return datatype;
    }
  }

  return nullptr;
}

static CADataType *catype_formalize_type(CADataType *datatype, int compact) {
  if (compact)
    return catype_formalize_type_compact(datatype);
  else {
    std::set<CADataType *> rcheck;
    rcheck.insert(datatype);
    return catype_formalize_type_expand(datatype, rcheck);
  }
}

CADataType *catype_create_type_from_unwind(int unwind) {
  // TODO:
  const char *unwindstr = symname_get(unwind);
#if 0
  int i = 0;
  int sigi = 0;
  int n;
  int ret;
  char namebuf[128];
  const char *pch = unwindstr;
  STEntry *entry = nullptr;
  CADataType *dt = nullptr;
  int tbuflen = 0;
  while(*pch) {
    switch (*pch) {
    case '*':
      // handling pointer: *AA
      sigi += str_assign_while(sigbuf, pch, '*');
      break;
    case '&':
      // handling reference: &AA
      // TODO: when implementing reference type
      // & can reprensent the reference type or the get address operator, here
      // when in type name it must be the reference type
      sigi += str_assign_while(sigbuf, pch, '&');
      break;
    case '[':
      // handling array: [AA;N], [[AA];N1];N2], ...
    case '{':
      // handling structure: {Name; <name1>:<anytype>, <name2>:<anytype>, <name3>:<anytype>, }
      tbuflen = buflen - sigi;
      ret = (*pch == '[') ?
	catype_unwind_type_array(symtable, pch, prenamemap, sigbuf + sigi, tbuflen) :
	catype_unwind_type_struct(symtable, pch, prenamemap, sigbuf + sigi, tbuflen);

      if (ret == -1) {
	yyerror("unwind type name `%s` failed", pch);
	return -1;
      }

      buflen = sigi + tbuflen;
      return (pch - caname) + ret;
    case '(':
      // handling tuple - named or unnamed: (<anytype>,<anytype>,<anytype>[,])
    case '<':
      // handling union: <Name; <name1>:<anytype>, <name2>:<anytype>, <name3>:<anytype>, >
    case '#':
      // handling enum: #AA, BB, ... #
    default:
      tbuflen = buflen - sigi;
      ret = catype_unwind_type_name(symtable, pch, prenamemap, sigbuf + sigi, tbuflen);
      if (ret == -1) {
	yyerror("unwind type name `%s` failed", pch);
	return -1;
      }

      sigi += tbuflen;
      break;
    }
  }

  return -1;
#endif

  // TODO:
  yyerror("unwinding...");
  return nullptr;
}

// 1. how to check type circle?
// the checking algorithm can instance into calculate the type size, when type A
// depends on type B then need calculate type B's size,   A => B, B => C, C => A
// then there is a circle. when type X is a multiple members type such as
// struct, then type X will depends on all it's members, so become calculate on
// it's members' size
// 2. how to reprensent a type?
// because a type may be recursive defined (not a circle: e.g. struct A {a: *A})
// so it's hard or not easy to reprensent using structure way, so here can use
// the unwinded string to reprensent the type in CADataType object. and because
// the operation against complex type can also be easyly calculated using string
// represent, such as the *operator just return a step in types.
// 3. how to do complex type operation, include: * & . etc?
// 4. how to unwind a type of recursive defined?
// when encounter a struct name then check if the name already appearance
// previously in this definition, if so, then it's the end for this name,
// else register the just encountered name for later checkings
// 5. how to compare two signature?
// there are 2 style methods to compare them:
// 1) strict: compare the type structure also compare the name when contains
// struct
// 2) unstrict only compare the type structure, but ignore the name of
// contains struct or field
// Q: how to do comparing when to do unstrict compare?
// A: can use equalvalent exchange such as when encountering. e.g. There are
// type A signature, type B signature, each time when encounter name namingly
// nameA-1, nameB-1 in both signature, create map nameA-1 => nameB-1 and nameB-1
// => nameA-1, later when encounter another pair names then search in the map to
// see if they already exists and if they are the same, if so then the name are
// identical, else they are not identical
//

CADataType *catype_get_by_name(SymTable *symtable, typeid_t name) {
  // put the primitive type table and the table by signature into together, so the step will become
  // 1) find type object in s_symtable_type_map table for speeding up
  // 2) if not find, then unwind the typeid of the type
  // 3) find in the primitive (global datatype) table, if find then return the type object
  // 4) if not find, then uses the unwinded typeid create the type object
  // 5) put the object into global table and s_symtable_type_map table

  // step 1: find type from symtable type table
  typeid_t windst = sym_form_symtable_type_id(symtable, name);
  auto itr = s_symtable_type_map.find(windst);
  if (itr != s_symtable_type_map.end())
    return itr->second;

  // step 2: get unwind id
  int typesize = 0;
  CADataType *dt = nullptr;
  typeid_t unwind = catype_unwind_type_signature(symtable, name, &typesize, &dt);
  if (unwind == typeid_novalue) {
    return nullptr;
  }

  // step 3: find type from global (signature) table
  auto itr2 = s_type_map.find(unwind);
  if (itr2 != s_type_map.end()) {
    s_symtable_type_map.insert(std::make_pair(windst, itr2->second));
    return itr2->second;
  }

  // start testing
  //const char *tmpstr = symname_get(unwind);
  //printf("typesize: %d\t%s\n", typesize, tmpstr);
  // end testing

  // step 4: create type object
  dt = catype_formalize_type(dt, 0);

  // step 5: update symtable type table
  s_symtable_type_map.insert(std::make_pair(windst, dt));
  s_type_map.insert(std::make_pair(unwind, dt));
  return dt;
}

static typeid_t inference_primitive_literal_type(CALiteral *lit) {
  const char *text = symname_get(lit->textid);
  int badscope = 0;
  tokenid_t intentdeftype = 0;
  int cmplen = 0;
  int len = 0;
  const char *format = nullptr;
  const char *cmpformat = nullptr;

  // handle non-fixed type literal value
  switch (lit->littypetok) {
  case I64:
  case U64:
    len = strlen(text);
    cmplen = lit->littypetok == I64 ? 3 : 2;
    cmpformat = lit->littypetok == I64 ? "-0x" : "0x";
    if (len > cmplen && !strncmp(cmpformat, text, cmplen)) {
      intentdeftype = lit->littypetok;
      format = "%p";
    } else {
      intentdeftype = I32;
      format = "%lu";
    }

    sscanf(text, format, &lit->u.i64value);
    badscope = lit->littypetok == I64 ?
      check_i64_value_scope(lit->u.i64value, I32) :
      check_u64_value_scope((uint64_t)lit->u.i64value, I32);
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
    intentdeftype = CHAR;
    lit->u.i64value = (uint8_t)parse_lexical_char(text);
    badscope = check_uchar_value_scope(lit->u.i64value, UCHAR);
    break;
  default:
    yyerror("line: %d, col: %d: void type have no literal value", glineno, gcolno);
    return typeid_novalue;
  }

  if (badscope) {
    yyerror("line: %d, col: %d: bad literal value definition: %s cannot be %s",
	    glineno, gcolno, get_type_string(lit->littypetok), get_type_string(intentdeftype));
    return typeid_novalue;
  }

  //const char *name = get_type_string(intentdeftype);
  lit->datatype = sym_form_type_id_from_token(intentdeftype);
  lit->fixed_type = 1;
  return lit->datatype;
}

static typeid_t inference_array_literal(CALiteral *lit) {
  std::vector<CALiteral> *lits = arraylit_deref(lit->u.arrayvalue);
  size_t len = lits->size();

  CADataType *catype = nullptr;
  CADataType *precatype = nullptr;
  for (int i = 0; i < len; ++i) {
    CALiteral *sublit = &lits->at(i);

    // for the inference, here just need extract the first element and get it's
    // type. when the other elements have different type, the later use will check for that
    typeid_t subid = inference_literal_type(sublit);
    if (!sublit->fixed_type) {
      yyerror("after inference ot still cannot determine the literal type");
      return typeid_novalue;
    }

    if (sublit->catype)
      catype = sublit->catype;
    else
      catype = catype_get_primitive_by_name(sublit->datatype);

    if (i == 0)
      precatype = catype;

    if (precatype->signature != catype->signature) {
      yyerror("different array element type: idx %d: %p`%s`, idx %d: %p`%s`",
	      i-1, precatype, catype_get_type_name(precatype->signature),
	      i, catype, catype_get_type_name(catype->signature));
      return typeid_novalue;
    }

    precatype = catype;
  }

  catype = catype_make_array_type(catype, len, 0);

  lit->catype = catype;
  lit->datatype = lit->catype->signature;
  lit->fixed_type = 1;

  return lit->datatype;
}

// inference and set the literal type for the literal, when the literal have no
// a determined type, different from `determine_literal_type`, the later is used by passing a defined type
typeid_t inference_literal_type(CALiteral *lit) {
  if (lit->fixed_type) {
    // no need inference, may should report an error
    return lit->datatype;
  }

  switch(lit->littypetok) {
  case ARRAY:
    return inference_array_literal(lit);
  case STRUCT:
    // not implemented
    yyerror("line: %d, col: %d: not implemented the literal for struct type.", glineno, gcolno);
    return typeid_novalue;
  case POINTER:
    // should never come here
    yyerror("line: %d, col: %d: not implemented the literal for pointer type, should can never come here", glineno, gcolno);
    return typeid_novalue;
  default:
    return inference_primitive_literal_type(lit);
  }
}

static bool is_literal_zero_value(CALiteral *lit) {
  const char *v = symname_get(lit->textid);
  return lit->littypetok == U64 && !strcmp(v, "0");
}

// determine and set the literal type for the literal for a specified type,
// different from `inference_literal_type` which have no a defined type
// parameter
void determine_primitive_literal_type(CALiteral *lit, CADataType *catype) {
  tokenid_t typetok = catype->type;
  if (typetok == tokenid_novalue || typetok == VOID)
    return;

  tokenid_t littypetok = lit->littypetok;

  // check convertable
  if (!is_literal_zero_value(lit) &&
      !literal_type_convertable(littypetok, typetok)) {
    yyerror("line: %d, col: %d: bad literal value definition: %s cannot be %s",
	    glineno, gcolno,
	    get_type_string(littypetok), catype_get_type_name(catype->signature));
    return;
  }

  // TODO: how to check the scope of array struct and pointer?
  // here temporary just return true in the scope checking check_i64_value_scope ...
  switch (typetok) {
  case CSTRING:
  case POINTER:
  case ARRAY:
  case STRUCT:
    lit->datatype = catype->signature; // TODO: this signature here may not accurrate yet
    lit->catype = catype;
    break;
  default:
    lit->datatype = sym_form_type_id_from_token(typetok);
    break;
  }

  // folliwing is for primitive -> primitive type
  const char *text = symname_get(lit->textid);
  int badscope = 0;

  int cmplen = 0;
  int len = 0;
  const char *format = nullptr;
  const char *cmpformat = nullptr;

  // check literal value scope
  switch (littypetok) {
  case I64: // I64 stand for positive integer value in lexical
  case U64:
    len = strlen(text);
    cmplen = littypetok == I64 ? 3 : 2;
    cmpformat = littypetok == I64 ? "-0x" : "0x";
    if (len > cmplen && !strncmp(cmpformat, text, cmplen))
      format = "%p";
    else
      format = "%lu";

    sscanf(text, format, &lit->u.i64value);
    badscope = littypetok == I64 ?
      check_i64_value_scope(lit->u.i64value, typetok) :
      check_u64_value_scope((uint64_t)lit->u.i64value, typetok);
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

  lit->fixed_type = 1;
}

void determine_array_literal(CALiteral *lit, CADataType *catype) {
  if (lit->littypetok != ARRAY && catype->type != ARRAY) {
    yyerror("array type not identical: %d != %d\n", lit->littypetok, catype->type);
    return;
  }

  // assuming the catype object is expand normalized, so dimension must be 1
  assert(catype->array_layout->dimension == 1);
  int len = catype->array_layout->dimarray[0];
  std::vector<CALiteral> *lits = arraylit_deref(lit->u.arrayvalue);
  if (len != lits->size()) {
    yyerror("expected an array with a fixed size of %d elements, found one with %d elements",
	    len, lits->size());
    return;
  }

  for (int i = 0; i < len; ++i) {
    CALiteral *sublit = &lits->at(i);
    CADataType *subtype = catype->array_layout->type;
    determine_literal_type(sublit, subtype);
    if (sublit->fixed_type == 0 || sublit->datatype != subtype->signature) {
      yyerror("determine literal failed for array element: fixed: %d, signature(lit: %s, type: %s), catype (%p, %p)",
	      sublit->fixed_type, catype_get_type_name(sublit->datatype), catype_get_type_name(subtype->signature),
	      sublit->catype, subtype);
      return;
    }

    // no need check this
    // if (sublit->catype != subtype) {}
  }

  lit->fixed_type = 1;
  lit->datatype = catype->signature;
  lit->catype = catype;
}

void determine_literal_type(CALiteral *lit, CADataType *catype) {
  if (!catype)
    return;

  if (lit->fixed_type) {
    if (lit->datatype == catype->signature)
      return;
    else {
      yyerror("conflicting of determining literal type: literal already have a type `%s`, cannot determine into `%s`",
	      catype_get_type_name(lit->datatype), catype_get_type_name(catype->signature));
      return;
    }
  }

  tokenid_t littypetok = lit->littypetok;
  tokenid_t typetok = catype->type;

  // TODO: check if can convert
  switch (littypetok) {
  case ARRAY:
    determine_array_literal(lit, catype);
    break;
  case CSTRING:
  case POINTER:
  case STRUCT:
    // TODO: implementing these kind of type conversion when encounter
    yyerror("cannot convert complex type to any type, may need change the functionality later");
    break;
  default:
    determine_primitive_literal_type(lit, catype);
    break;
  }
}

CADataType *catype_clone_thin(const CADataType *type) {
  auto dt = new CADataType;
  dt->status = type->status;
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

CADataType *catype_make_type_symname(typeid_t name, int type, int size) {
  auto dt = new CADataType;
  dt->status = CADT_None;
  dt->formalname = name;
  dt->type = type;
  dt->size = size;
  dt->signature = name;
  dt->struct_layout = nullptr;
  //catype_put_primitive_by_name(name, dt);
  return dt;
}

// type + '*'
// i32 + '*' => *i32, *type + '*' => **type, [type;n] + '*' => *[type;n]
// [[[type1;n1];n2];n3] + '*' => *[[[type1;n1];n2];n3]
// [*i32;n], [**i32;n], *[*i32;n], [*[*i32;n1];n2], *[[*i32;n1];n2]
static typeid_t form_datatype_signature(CADataType *type, int plus, uint64_t len) {
  char buf[1024];
  const char *name = catype_get_type_name(type->signature);
  switch (plus) {
  case '*':
    // pointer
    sprintf(buf, "t:*%s", name);
    break;
  case '[':
    // array
    sprintf(buf, "t:[%s;%lu]", name, len);
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
  typeid_t signature = form_datatype_signature(datatype, '*', 0);
  // TODO: use type also from symbol
  CADataType *type = catype_get_primitive_by_name(signature);
  if (type)
    return type;

  // create new CADataType object here and put it into datatype table
  switch (datatype->type) {
  case POINTER:
    // make pointer's pointer
    type = catype_clone_thin(datatype);
    type->formalname = signature;
    type->signature = signature;

#if 0 // if using compact
    type->pointer_layout->dimension++;
#else
    type->pointer_layout->dimension = 1;
    type->pointer_layout->type = datatype;
#endif
    break;
  case ARRAY:
    // array's pointer
  case STRUCT:
    // structure's pointer
  default:
    // array and struct type can directly append the signature
    type = catype_make_type_symname(signature, POINTER, sizeof(void *));
    type->pointer_layout = new CAPointer;
    type->pointer_layout->dimension = 1;
    type->pointer_layout->type = datatype;
    break;
  }

  type->status = CADT_Expand;
  catype_put_primitive_by_name(signature, type);

  return type;
}

// handle signature for structure, use current symbol table name@address as the
// signature test pointer type parsing, realize array type, struct type:
// [[[i32;2];3];4] <==> [i32;2;3;4], [[*[i32;2];3];4] <==> [*[i32;2];3;4]
// the array representation using the later which is the compact one
CADataType *catype_make_array_type(CADataType *datatype, uint64_t len, bool compact) {
  typeid_t signature = form_datatype_signature(datatype, '[', len);

  // TODO: use type also from symbol
  CADataType *dt = catype_get_primitive_by_name(signature);
  if (dt)
    return dt;

  // create new CADataType object here and put it into datatype table
  switch (datatype->type) {
  case ARRAY:
    dt = catype_clone_thin(datatype);
    dt->size = len * datatype->size;
    dt->formalname = signature;
    dt->signature = signature;

    if (compact) {
      // array and struct type can directly append the signature
      dt->array_layout->dimarray[dt->array_layout->dimension++] = len;
    } else {
      dt->array_layout->dimension = 1;
      dt->array_layout->dimarray[0] = len;
      dt->array_layout->type = datatype;
    }
    break;
  case POINTER:
  case STRUCT:
  default:
    dt = catype_make_type_symname(signature, ARRAY, len * datatype->size);
    dt->array_layout = new CAArray;
    dt->array_layout->dimension = 1;
    dt->array_layout->dimarray[0] = len;
    dt->array_layout->type = datatype;
    break;
  }

  dt->status = CADT_Expand;
  catype_put_primitive_by_name(signature, dt);

  return dt;
}

#if 0
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

    //dt->size += cav->datatype->size;
    dt->struct_layout->fields[i].name = cav->name;
    dt->struct_layout->fields[i].type = cav->datatype;
  }

  return dt;
}
#endif

void put_post_function(typeid_t fnname, void *carrier) {
  g_function_post_map[fnname] = carrier;
}

int exists_post_function(typeid_t fnname) {
  auto itr = g_function_post_map.find(fnname);
  if (itr != g_function_post_map.end())
    return 1;

  return 0;
}

void *get_post_function(typeid_t fnname) {
  auto itr = g_function_post_map.find(fnname);
  if (itr != g_function_post_map.end())
    return itr->second;

  return nullptr;
}

int catype_check_identical(CADataType *type1, CADataType *type2) {
  return type1->signature == type2->signature;
}

int catype_check_identical_in_symtable(SymTable *st1, typeid_t type1, SymTable *st2, typeid_t type2) {
  // notice: when type1 == type2 does means they are equal, because they belong diffrent symbol table
  if (type1 == type2 && st1 == st2)
    return 1;

  CADataType *dt1 = catype_get_by_name(st1, type1);
  CADataType *dt2 = catype_get_by_name(st2, type2);

  if (!dt1 || !dt2)
    return 0;

  // check not only the token type but also check other information when the type is complex type
  if (dt1->type != dt2->type)
    return 0;

  if (dt1->signature != dt2->signature)
    return 0;

  return 1;
}

int catype_check_identical_in_symtable_witherror(SymTable *st1, typeid_t type1, SymTable *st2, typeid_t type2, int exitwhenerror, SLoc *loc) {
  if (catype_check_identical_in_symtable(st1, type1, st2, type2))
    return 1;

  if (exitwhenerror) {
    yyerror("line: %d, col: %d: the type `%s` not identical type `%s`",
	    loc->row, loc->col, symname_get(type1), symname_get(type2));
    exit(-1);
  }

  return 0;
}

const char *catype_get_function_name(typeid_t fnname) {
  return symname_get(fnname) + 2;
}

const char *catype_get_type_name(typeid_t type) {
  return symname_get(type) + 2;
}

// unwind the type into type object, when have object like members
// e.g.
// example 1:
// t:AA; type AA = *[*[*BB;3];4]; type BB = *CC; type CC = [*i32;6];
// after unwind: *[*[**[*i32;6];3];4]
// example 2:
// t:AA; type AA = *[*[*BB;3];4]; struct BB { a: CC }; type CC = *[i32;6];
// after unwind, it becomes: *[*[*{BB;a:*[i32;6]};3];4]
END_EXTERN_C

Value *tidy_value_with_arith(Value *v, int typetok) {
  if (typetok == F32)
    v = ir1.builder().CreateFPExt(v, ir1.float_type<double>());

  return v;
}

Type *gen_llvmtype_from_token(int tok) {
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

static Type *gen_llvmtype_from_catype_inner(CADataType *catype, std::map<CADataType *, Type *> &rcheck) {
  switch (catype->type) {
  case POINTER: {
    // create llvm pointer type
    Type *kerneltype = nullptr;
    auto itr = rcheck.find(catype->pointer_layout->type);
    if (itr != rcheck.end())
      kerneltype = itr->second;
    else
      kerneltype = gen_llvmtype_from_catype_inner(catype->pointer_layout->type, rcheck);

    if (!kerneltype) {
      fprintf(stderr, "create kernel type for pointer failed: %s\n",
	      symname_get(catype->pointer_layout->type->signature));
      return nullptr;
    }

    Type *ptrtype = kerneltype;
    int i = catype->pointer_layout->dimension;
    while(i-- > 0)
      ptrtype = PointerType::get(ptrtype, 0);

    return ptrtype;
  }
  case ARRAY: {
    // create llvm array type
    // it seems here no need to do type checking, because the array kernel type
    // should cannot be recursively defined, or the size is unlimited, the checking
    // should already blocked by the type system analyze
    assert(rcheck.find(catype->pointer_layout->type) == rcheck.end());
    Type *kerneltype = gen_llvmtype_from_catype_inner(catype->array_layout->type, rcheck);

    if (!kerneltype) {
      fprintf(stderr, "create kernel type for array failed: %s\n",
	      symname_get(catype->array_layout->type->signature));
      return nullptr;
    }

    Type *arrtype = kerneltype;
    int i = catype->array_layout->dimension;
    while (--i > -1)
      arrtype = ArrayType::get(arrtype, catype->array_layout->dimarray[i]);

    return arrtype;
  }
  case STRUCT: {
    // create llvm struct type
    size_t fieldnum = catype->struct_layout->fieldnum;
    std::vector<Type *> fields;
    StringRef name = symname_get(catype->formalname);
    bool pack = false;

    StructType *sttype = nullptr;
    auto itr = g_llvmtype_map.find(catype->signature);
    if (itr != g_llvmtype_map.end()) {
      sttype = static_cast<StructType *>(itr->second);
    } else {
      sttype = StructType::create(ir1.ctx(), name);
      g_llvmtype_map.insert(std::make_pair(catype->signature, static_cast<Type *>(sttype)));
    }

    rcheck.insert(std::make_pair(catype, sttype));
    for (int i = 0; i < catype->struct_layout->fieldnum; ++i) {
      Type *fieldtype = gen_llvmtype_from_catype_inner(catype->struct_layout->fields[i].type, rcheck);
      fields.push_back(fieldtype);
    }
    rcheck.erase(catype);
    sttype->setBody(fields, pack);

    // static StructType *create(LLVMContext &Context, StringRef Name);
    // static StructType *create(LLVMContext &Context);

    // static StructType *create(ArrayRef<Type *> Elements, StringRef Name,
    // 			      bool isPacked = false);
    // static StructType *create(ArrayRef<Type *> Elements);
    // static StructType *create(LLVMContext &Context, ArrayRef<Type *> Elements,
    // 			      StringRef Name, bool isPacked = false);
    // static StructType *create(LLVMContext &Context, ArrayRef<Type *> Elements);

    //StructType *sttype = StructType::get(ir1.ctx(), fields, pack);
    return sttype;
  }
  default:
    return gen_llvmtype_from_token(catype->type);
  }
}

Type *gen_llvmtype_from_catype(CADataType *catype) {
  std::map<CADataType *, Type *> rcheck;
  Type *type = gen_llvmtype_from_catype_inner(catype, rcheck);
  return type;
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

static int can_type_binding(CALiteral *lit, tokenid_t typetok) {
  // TODO: use type also from symbol table, when literal also support array or struct, e.g. AA {d,b}
  CADataType *dt = catype_get_primitive_by_name(lit->datatype);
  if (!dt) {
    yyerror("get type failed: type is: `%d`", lit->datatype);
    return 0;
  }

  switch (dt->type) {
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
    yyerror("bad lexical literal type: '%s'", get_type_string(dt->type));
    return 0;
  }
}

Value *gen_primitive_literal_value(CALiteral *value, tokenid_t typetok, SLoc loc) {
  // check if literal value type matches the given typetok, if not match, report error
  if (!value->fixed_type && !can_type_binding(value, typetok)) {
    yyerror("line: %d, col: %d: literal value type '%s' not match the variable type '%s'",
	    loc.row, loc.col, get_type_string(typetok), get_type_string(typetok));
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

Value *gen_zero_literal_value(CADataType *catype) {
  // TODO: implement it
  return nullptr;
}

Value *gen_array_literal_value(CALiteral *lit, CADataType *catype, SLoc loc) {
  // using expand formalized catype object
  if(catype->array_layout->dimension != 1) {
    yyerror("(internal) the array dimension is not normalized into 1");
    return nullptr;
  }
  assert(lit->littypetok == ARRAY);
  int len = catype->array_layout->dimarray[0];
  std::vector<CALiteral> *lits = arraylit_deref(lit->u.arrayvalue);
  assert(len == lits->size());

  std::vector<Constant *> subvalues;
  CADataType *subtype = catype->array_layout->type;
  for (int i = 0; i < len; ++i) {
    CALiteral *sublit = &lits->at(i);
    Value *subvalue = gen_literal_value(sublit, subtype, loc);
    subvalues.push_back((Constant *)subvalue);
  }

  Type *arraytype = gen_llvmtype_from_catype(catype);
  Constant *arrayconst = ConstantArray::get((ArrayType *)arraytype, subvalues);
  //GlobalValue *g = ir1.gen_global_var(arraytype, "constarray", arrayconst, true);

  return arrayconst;

  // AllocaInst::ZExt;
  // ir1.builder().CreateInst
  // ConstantStruct::get();
  //ir1.builder().CreateAlloca();
  // llvm::ConstantArray
  //Constant::getOperand();

  // makeArrayRef();
  // MDStringArray::
  // MDNodeArray::get();
  // MDTupleArray;


  // ConstantDataArray::get();
  // ConstantArray::get(); //(ArrayType *T, ArrayRef<Constant *> V);
  // TODO:
  //ConstantArray::get(ArrayType *T, ArrayRef<Constant *> V);
  //ConstantDataArray;
  //FeatureBitArray;

  yyerror("the array literal not implemented yet");
  return nullptr;
}

Value *gen_literal_value(CALiteral *lit, CADataType *catype, SLoc loc) {
  // TODO: use type also from symbol table, when literal also support array or struct, e.g. AA {d,b}
  Type *type = nullptr;
  Value *llvmvalue = nullptr;
  switch(catype->type) {
  case POINTER:
    if (lit->littypetok == CSTRING) {
      // create string literal
      const char *data = symname_get(lit->u.strvalue.text);
      llvm::StringRef strref(data, lit->u.strvalue.len);
      Constant *llvmconststr = ir1.builder().CreateGlobalStringPtr(strref);
      return llvmconststr;
    }

    type = gen_llvmtype_from_catype(catype);

    if (lit->u.i64value == 0) {
      llvmvalue = ConstantPointerNull::get(static_cast<PointerType *>(type));

      // TODO: other pointer literal value should not supported
      //yyerror("other pointer literal not implemented yet");
      //return nullptr;
    } else {
      llvmvalue = ir1.gen_int<int64_t>(lit->u.i64value);
      //llvmvalue = ir1.builder().CreatePointerCast(llvmvalue, type);
      llvmvalue = ir1.builder().CreateIntToPtr(llvmvalue, type);
    }

    return llvmvalue;
  case ARRAY:
    return gen_array_literal_value(lit, catype, loc);
  case STRUCT:
    //ConstantStruct::get();
    // TODO:
    yyerror("the struct literal not implemented yet");
    return nullptr;
  default:
    return gen_primitive_literal_value(lit, catype->type, loc);
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
  case POINTER:
    return "%p";
  default:
    return "\n";
  }
}

int is_unsigned_type(tokenid_t type) {
  return type == U32 || type == U64 || type == UCHAR;
}

int is_integer_type(tokenid_t type) {
  return type == I32 || type == I64 || type == CHAR || type == U32 || type == U64 || type == UCHAR;
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

// used for `as` value convertion
// VOID I32 I64 U32 U64 F32 F64 BOOL CHAR UCHAR ATOMTYPE_END STRUCT ARRAY POINTER CSTRING
// Trunc ZExt SExt FPToUI FPToSI UIToFP SIToFP FPTrunc FPExt PtrToInt IntToPtr BitCast AddrSpaceCast
// CastOpsBegin stand for no need convert, CastOpsEnd stand for cannot convert
static Instruction::CastOps
llvmtype_cast_table[CSTRING - VOID + 1][CSTRING - VOID + 1] = {
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
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    (ICO)-1,           /* POINTER */
    (ICO)-1,           /* CSTRING */
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
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    ICO::IntToPtr,     /* POINTER */
    (ICO)-1,           /* CSTRING */
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
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    ICO::IntToPtr,     /* POINTER */
    (ICO)-1,           /* CSTRING */
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
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    ICO::IntToPtr,     /* POINTER */
    (ICO)-1,           /* CSTRING */
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
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    ICO::IntToPtr,     /* POINTER */
    (ICO)-1,           /* CSTRING */
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
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    (ICO)-1,           /* POINTER */
    (ICO)-1,           /* CSTRING */
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
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    (ICO)-1,           /* POINTER */
    (ICO)-1,           /* CSTRING */
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
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    (ICO)-1,           /* POINTER */
    (ICO)-1,           /* CSTRING */
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
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    (ICO)-1,           /* POINTER */
    (ICO)-1,           /* CSTRING */
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
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    (ICO)-1,           /* POINTER */
    (ICO)-1,           /* CSTRING */
  },                   // UCHAR ->
  { // Begin ATOMTYPE_END
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
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    (ICO)-1,           /* POINTER */
    (ICO)-1,           /* CSTRING */
  },                   // ATOMTYPE_END -> ?
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
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    (ICO)-1,           /* POINTER */
    (ICO)-1,           /* CSTRING */
  },                   // STRUCT -> ?
  { // Begin ARRAY
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
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    (ICO)0,            /* POINTER */
    (ICO)0,            /* CSTRING */
  },                   // ARRAY -> ?
  { // Begin POINTER
    (ICO)-1,           /* VOID */
    ICO::PtrToInt,     /* I32 */
    ICO::PtrToInt,     /* I64 */
    ICO::PtrToInt,     /* U32 */
    ICO::PtrToInt,     /* U64 */
    (ICO)-1,           /* F32 */
    (ICO)-1,           /* F64 */
    (ICO)-1,           /* BOOL */
    (ICO)-1,           /* CHAR */
    (ICO)-1,           /* UCHAR */
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    ICO::BitCast,      /* POINTER */
    ICO::BitCast,      /* CSTRING */
  },                   // POINTER -> ?
  { // Begin CSTRING
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
    (ICO)-1,           /* ATOMTYPE_END */
    (ICO)-1,           /* STRUCT */
    (ICO)-1,           /* ARRAY */
    ICO::BitCast,      /* POINTER */
    ICO::BitCast,      /* CSTRING */
  },                   // CSTRING -> ?

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

Instruction::CastOps gen_cast_ops(CADataType *fromtype, CADataType *totype) {
  if (fromtype->signature == totype->signature)
    return (Instruction::CastOps)0;

  tokenid_t fromtok = fromtype->type;
  tokenid_t totok = totype->type;
  return llvmtype_cast_table[fromtok-VOID][totok-VOID];
}

// used for literal value convertion
// 1. for the type that before ATOMTYPE_END, it use following regulars:
//   left side is lexical literal value (I64 stand for negative integer value,
//   U64 stand for positive integer value, F64 stand for floating point value)
//   right side is real literal value
// 2. for the type that after ATOMTYPE_END, it use different regulars:
//   they are have no scope checking, because they are non-primitive type
// VOID I32 I64 U32 U64 F32 F64 BOOL CHAR UCHAR ATOMTYPE_END STRUCT ARRAY POINTER CSTRING
static int s_literal_type_convertable_table[ATOMTYPE_END - VOID + 1][CSTRING - ATOMTYPE_END + ATOMTYPE_END - VOID + 1] = {
  {0, }, // VOID -> other-type, means convert from VOID type to other type
  {0, },   // I32 -> other-type
  {0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0 /* ATOMTYPE_END */, 0, 0, 0, 1,}, // I64 ->
  {0, }, // U32 ->
  {0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0 /* ATOMTYPE_END */, 0, 0, 0, 1,}, // U64 ->
  {0, }, // F32 ->
  {0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0 /* ATOMTYPE_END */, 0, 0, 0, 0,}, // F64 ->
  {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 /* ATOMTYPE_END */, 0, 0, 0, 0,}, // BOOL ->
  {0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0 /* ATOMTYPE_END */, 0, 0, 0, 1,}, // CHAR ->
  {0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0 /* ATOMTYPE_END */, 0, 0, 0, 1,}, // UCHAR ->
  {0, }, // ATOMTYPE_END
  // {0, }, // STRUCT, these 4 should just is stub, they should not come here
  // {0, }, // ARRAY ->
  // {0, }, // POINTER ->
  // {0, }, // CSTRING ->
};

int literal_type_convertable(tokenid_t from, tokenid_t to) {
  return s_literal_type_convertable_table[from-VOID][to-VOID];
}

// for rust the `as` cannot convert others to bool and
// int/float to any primitive type, to pointer * but cannot to reference &, bool
// bool can to int type, but cannot float type, char type
// only u8 can cast to char, but char can to int
// it seems use token type is good than use typeid_t type
int as_type_convertable(tokenid_t from, tokenid_t to) {
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
int check_i64_value_scope(int64_t lit, tokenid_t typetok) {
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
  case CSTRING:
  case POINTER:
  case ARRAY:
  case STRUCT:
    return 0;
  default:
    yyerror("i64 lexcial value incompatible with %s", get_type_string(typetok));
    return -1;
  }
}

int check_u64_value_scope(uint64_t lit, tokenid_t typetok) {
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
  case CSTRING:
  case POINTER:
  case ARRAY:
  case STRUCT:
    return 0;
  default:
    yyerror("u64 lexcial value incompatible with %s", get_type_string(typetok));
    return -1;
  }
}

int check_f64_value_scope(double lit, tokenid_t typetok) {
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

int check_char_value_scope(char lit, tokenid_t typetok) {
  if (typetok == UCHAR && lit < 0)
    return 1;

  return 0;
}

int check_uchar_value_scope(uint8_t lit, tokenid_t typetok) {
  if (typetok == CHAR && lit > 127)
    return -1;

  return 0;
}

bool catype_is_float(tokenid_t typetok) {
  return (typetok == F32 || typetok == F64);
}

bool catype_is_complex_type(tokenid_t typetok) {
  switch (typetok) {
  case ARRAY:
  case STRUCT:
    return true;
  case POINTER:
  default:
    return false;
  }
}

static CADataType *catype_make_type(const char *name, int type, int size) {
  auto datatype = new CADataType;
  int formalname = symname_check_insert(name);
  datatype->status = CADT_Orig;
  datatype->formalname = formalname;
  datatype->type = type;
  datatype->size = size;
  datatype->signature = formalname;
  datatype->struct_layout = nullptr;
  catype_put_primitive_by_name(formalname, datatype);
  return datatype;
}
