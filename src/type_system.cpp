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
 * Type system design:
 * String representation:
 * Internal primitive types: t:i8, t:i32, t:u8, t:u32, t:f32, t:bool, ...,
 * User defined types: t:<typename>, t:abcd
 * Pointer types: t:*void, t:*i8, t:*i32, t:**[*i32; 100], t:*[**[*i32; 100];2], t:*atomic_t, t:*rect_t, t:**abc
 * Array types: t:[u64;12], t:[*i32; 100], t:[*[**[*i32; 100];2];3], t:[atomic_t,3], t:[*rect_t,6], t:[**abc, 8]
 * Struct & type defined types: t:atomic_t, t:rect_t, t:abc
 *
 * typeid_t representation:
 * u8 id: xxx where "t:u8" == symname_get(xxx), ...
 *
 * The string and typeid_t representation is scope related,
 * so it should associate with the symbol table.
 *
 * CADataType object:
 * This is the unique object that stands for a datatype.
 * Primitive types are stored in the catype map.
 * Other types are stored in the symbol table, because they have a scope.
 *
 * About fallback of type:
 * There exist type definitions based on unknown types (undefined yet types),
 * so a fallback list is needed from unknown types, e.g.,
 * unknowntype, *unknowntype, **unknowntype, [unknowntype;3],
 * *[unknowntype;4], etc. These types should form a list, and when
 * the unknown type is determined, all elements in the list should
 * be filled with (unknowntype, CADataType(unknowntype)),
 * ((*unknowntype, CADataType(*unknowntype))), ...
 */

#include "type_system.h"
#include "ca_parser.h"
#include "strutil.h"

#include "ca_types.h"
#include "symtable.h"

#include <algorithm>
#include <alloca.h>
#include <array>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string.h>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <limits>

BEGIN_EXTERN_C
#include "ca.tab.h"
END_EXTERN_C


#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

extern int glineno;
extern int gcolno;
extern SymTable g_root_symtable;

std::vector<CALiteral> *arraylit_deref(CAArrayLit obj);

// name to CADatatype map
std::unordered_map<typeid_t, CADataType *> s_symtable_type_map;
std::unordered_map<typeid_t, CADataType *> s_signature_type_map;
std::unordered_map<typeid_t, CADataType *> s_type_map;

CADataType *g_catype_void_ptr = nullptr;

std::unordered_map<std::string, int> s_token_primitive_map {
  {"void",   VOID},
  {"short",  I16},
  {"i16",    I16},
  {"int",    I32},
  {"i32",    I32},
  {"i64",    I64},
  {"isize",  I64},
  {"ushort", U16},
  {"u16",    U16},
  {"uint",   U32},
  {"u32",    U32},
  {"u64",    U64},
  {"usize",  U64},
  {"float",  F32},
  {"f32",    F32},
  {"double", F64},
  {"f64",    F64},
  {"bool",   BOOL},
  {"i8",     I8},
  {"char",   I8},
  {"u8",     U8},
  {"uchar",  U8},
};

std::unordered_map<std::string, std::string> s_token_primitive_inner_map {
  {"void",   "void",},
  {"short",  "i16", },
  {"i16",    "i16", },
  {"int",    "i32", },
  {"i32",    "i32", },
  {"i64",    "i64", },
  {"isize",  "i64", },
  {"ushort", "u16", },
  {"u16",    "u16", },
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
  {"short",  I16},
  {"i16",    I16},
  {"int",    I32},
  {"i32",    I32},
  {"i64",    I64},
  {"ushort", I16},
  {"u16",    U16},
  {"uint",   U32},
  {"u32",    U32},
  {"u64",    U64},
  {"float",  F32},
  {"f32",    F32},
  {"double", F64},
  {"f64",    F64},
  {"bool",   BOOL},
  {"i8",     I8},
  {"char",   I8},
  {"u8",     U8},
  {"uchar",  U8},
#endif

#if 0
  {">=",     GE},
  {"<=",     LE},
  {"==",     EQ},
  {"!=",     NE},
  {"...",    VARG},
  {"..",     IGNORE},
#endif

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
  {"struct", STRUCT},
  {"type",   TYPE},
  {"as",     AS},
  {"sizeof", SIZEOF},
  {"typeof", TYPEOF},
  {"typeid", TYPEID},
  {"__zero_init__", ZERO_INITIAL},
  {"loop",   LOOP},
  {"for",    FOR},
  {"in",     IN},
  {"break",  BREAK},
  {"continue",  CONTINUE},
  {"match",  MATCH},
  {"use",    USE},
  {"mod",    MOD},
  {"box",    BOX},
  //  {"box_noinit", BOX_NOINIT},
  {"ref",    REF},
  {"drop",   DROP},
  {"impl",   IMPL},
  {"trait",  TRAIT},
  //  {"self",   OSELF},
  //  {"Self",   CSELF},
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
  {"i8",    0, 8, 1, I8, TT_Primitive},
  {"char",  0, 8, 1, I8, TT_Alias},
  {"u8",    0, 9, 1, U8,TT_Primitive},
  {"uchar", 0, 9, 1, U8,TT_Alias},
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
  case I16:
    return "i16";
  case I32:
    return "i32";
  case I64:
    return "i64";
  case U16:
    return "u16";
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
  case I8:
    // TODO: for all the above using the following one
    return forid ? get_inner_type_string_by_str("char") : "char";
  case U8:
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

  yyerror("get primitive type `%s` token failed", name);
  return tokenid_novalue;
}

int catype_init() {
  CADataType *datatype;
  int name;
  datatype = catype_make_type("t:void", VOID, 0); // void
  CADataType *datatype_void_ptr = datatype;

  datatype = catype_make_type("t:i16", I16, 2); // i16
  name = symname_check_insert("t:short");
  catype_put_primitive_by_name(name, datatype); // short

  datatype = catype_make_type("t:i32", I32, 4); // i32
  name = symname_check_insert("t:int");
  catype_put_primitive_by_name(name, datatype); // int

  catype_make_type("t:i64", I64, 8);            // i64

  datatype = catype_make_type("t:u16", U16, 2); // u16
  name = symname_check_insert("t:ushort");
  catype_put_primitive_by_name(name, datatype); // ushort

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
  datatype = catype_make_type("t:i8", I8, 1); // i8
  name = symname_check_insert("t:char");
  catype_put_primitive_by_name(name, datatype); // char

  datatype = catype_make_type("t:u8", U8, 1);// u8
  name = symname_check_insert("t:uchar");
  catype_put_primitive_by_name(name, datatype); // uchar

  g_catype_void_ptr = catype_make_pointer_type(datatype);

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
  case SLICE:
  case STRUCT:
    debug_print_tab(identhead);
    fprintf(out_source, "%c%s@%p\n", datatype->struct_layout->type ? '(' : '{', symname_get(datatype->formalname), datatype);

    accessed.insert(datatype);
    for (int i = 0; i < datatype->struct_layout->fieldnum; ++i) {
      debug_print_tab(ident + 1);
      if (datatype->struct_layout->type != Struct_NamedStruct)
	fprintf(out_source, "%s: ", symname_get(datatype->struct_layout->fields[i].name));

      debug_catype_datatype_aux(datatype->struct_layout->fields[i].type, accessed, 0, ident + 1);
      fprintf(out_source, ",\n");
    }

    debug_print_tab(ident);
    fprintf(out_source, datatype->struct_layout->type ? ")" : "}");
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

/*
 * Type signature example:
 * Primitive: i8, i32, u32, f64, bool, void, ...
 * Pointer: *<anytype>, *i8, *u32, *f64, ...
 * Reference: &<anytype>, &i8, &u32, &f64, ...
 * Array: [<anytype>; N]
 * Struct: {Name; <name1>:<anytype>, <name2>:<anytype>, <name3>:<anytype>,}
 * Unnamed tuple: (<anytype>, <anytype>, <anytype>[,])
 * Named tuple: (Name; <anytype>, <anytype>, <anytype>[,])
 *              # <anytype> can also include Name
 * Union: <Name; <name1>:<anytype>, <name2>:<anytype>, <name3>:<anytype>,>
 * Enum: #name, name=value, name#
 * How to represent the recursive type?
 * Struct AA { a: *AA, b: {} }
 */

/*
 * TODO: Consider replacing the string-style signature with a struct-based
 * approach. This means converting the string signature type into a
 * struct signature. However, it is currently unknown if this will
 * yield higher performance.
 *
 * The signature is only used for indexing the type object; it cannot be
 * used for operations such as `->`. This is not only due to performance
 * issues but also because it lacks the ability to accurately identify
 * the exact type in recursive contexts when using the signature.
 */
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
				     CADataType **retdt = nullptr, CAStructType struct_type = Struct_NamedStruct);

static int str_assign_while(char *sigbuf, const char *&pch, int ch) {
  int i = 0;
  while(*pch == ch) {
    sigbuf[i++] = *pch++;
  }

  return i;
}

/**
 * @brief internal function: unwind type signature.
 * @param buflen: Input/output parameter; input for sigbuf size, output for used size.
 * @param typesize: When `typesize` is not NULL, it will store the typesize.
 *           Returns -1 when the type is unlimited.
 * @param prenamemap: For checking if the name has already appeared in the upper tree;
 *             it always inherits values from the parent.
 * @param rcheckset: For checking if recursion exists; when encountering '*' or '&',
 *            it will discard (not use) the values from the parent.
 * @param retdt: When `retdt` is not NULL, it returns the CADataType object
 *        associated with the signature. Note: The returned object `retdt`
 *        needs to be normalized to become compacted. However, when the
 *        object is created directly from an unwinded string, it does not
 *        need normalization, as the unwinded string is already normalized.
 *
 * @return:
 * - -1 for error
 * - 0 or positive value for the consumption index.
 */
static int catype_unwind_type_signature_inner(SymTable *symtable, const char *caname,
					      const std::map<std::string, CADataType *> &prenamemap,
					      const std::set<std::string> &rcheckset,
					      char *sigbuf, int &buflen, int *typesize,
					      CADataType **retdt = nullptr)
{
  /*
   * the unwinding algorithm need realize following functionality
   * t:i32 => return
   * t:*i32 t:**i32 =>
   * t:[i32;3]
   * type A = **B; type B = **[*i32;3] t:A => t:**B; t:
   */

  /*
  const char *tname = symname_get(name);
  if (tname[0] == 'T' && tname[1] == ':') {
    ASTNode *expr = astnode_unwind_from_addr(tname+2);
    name = inference_expr_type(expr);
  }
  */

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
	addrdt->pointer_layout->allocpos = CAPointerAllocPos::PP_Stack;
      }
      break;
    case '&':
      /*
       * handling reference: &AA
       * TODO: When implementing reference type, it is also suitable for CADataType,
       * note that '&' can denote either a reference type or the address-of operator.
       * However, in type names, it always represents a reference type.
       */
      sigi += str_assign_while(sigbuf, pch, '&');
      *typesize = sizeof(void *);
      sizefin = 1;
      break;
    default:
      tbuflen = buflen - sigi;
      if (sizefin) {
	// when the type is already determined (* & case), just use new check set
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
	// handling - named tuple (Name;<type1>,<type2>,<type3>[,]) and
	// general unnamed tuple: (;<type1>,<type2>,<type3>[,])
	ret = catype_unwind_type_struct(symtable, pch, prenamemap, *prcheckset, sigbuf + sigi, tbuflen, &tmptypesize, outdt, Struct_NamedTuple);
	break;
      case '<':
	// TODO: handling union: <Name; <name1>:<anytype>, <name2>:<anytype>, <name3>:<anytype>, >
	ret = catype_unwind_type_struct(symtable, pch, prenamemap, *prcheckset, sigbuf + sigi, tbuflen, &tmptypesize, outdt, Struct_General);
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

      // if size is already determined (for pointer or reference type) or size calculation failed
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

static int signature_extract_name(char *namebuf, char *sigbuf, int &sigi, const char *&pch) {
  int i = 0;
  while(isalnum(*pch) || *pch == '_') {
    sigbuf[sigi++] = *pch;
    namebuf[i++] = *pch++;
  }
  namebuf[i] = '\0';

  return i;
}

static int extract_general_struct_name(CAStructType &struct_type, char *namebuf,
                                       int elen, char *sigbuf, int &sigi,
                                       char *pch) { return -1; }

static bool is_general_struct_type(CAStructType struct_type) {
  return struct_type == Struct_Slice || struct_type == Struct_Enum || struct_type == Struct_Union;
}

static bool is_tuple_type(CAStructType struct_type) {
  return struct_type == Struct_NamedTuple || struct_type == Struct_GeneralTuple;
}

static bool is_named_field_struct_type(CAStructType struct_type) {
  return struct_type == Struct_NamedStruct || struct_type == Struct_Union;
}

static bool is_named_struct_type(CAStructType struct_type) {
  return
    struct_type == Struct_NamedStruct ||
    struct_type == Struct_NamedTuple ||
    struct_type == Struct_Union ||
    struct_type == Struct_Enum;
}

static int catype_unwind_type_struct(SymTable *symtable, const char *pchbegin,
				     const std::map<std::string, CADataType *> &prenamemap,
				     const std::set<std::string> &rcheckset,
				     char *sigbuf, int &buflen, int *typesize,
				     CADataType **retdt, CAStructType struct_type) {
  const char *pch = pchbegin;
  int sigi = 0;
  sigbuf[sigi++] = *pch++; // = '{'; or '(' when tuple; or '<' when struct form types (union, enum, slice, ...)
  int tmptypesize = 0;
  int sizeerror = 0;
  int calcing = 0;
  *typesize = -1;

  char namebuf[128];
  int elen = signature_extract_name(namebuf, sigbuf, sigi, pch);

  if (struct_type == Struct_General) {
    // <slice;;*i32,i64>
    if (!strcmp(namebuf, "slice")) {
      struct_type = Struct_Slice;
    } else if (!strcmp(namebuf, "union")) {
      // <union;UU;f1:i32,f2:i64,f3:*u8> or <union;UU> # union can be empty
      struct_type = Struct_Union;
      yyerror("not implemented for the `union` unwinding");
    } else if (!strcmp(namebuf, "enum")) {
      // <enum;EE;A,B=3,C> or even <enum;EE;{A;i32}, {B;f1:i32,f2:i64}, D=5> or
      // <enum;EE;A> #enum cannot be empty
      struct_type = Struct_Enum;
      yyerror("not implemented for the `enum` unwinding");
    } else {
      if (elen)
	yyerror("not implemented for the unknown `%s` unwinding", namebuf);
      else {
	yyerror("(internal) general struct have no name `%s`", pch);
	return -1;
      }
    }

    if (*pch != ';') {
      yyerror("(internal) bad format of `%s` style `%s` != ';'", namebuf, pch);
      return -1;
    }

    sigbuf[sigi++] = *pch++; // = ';';
    elen = signature_extract_name(namebuf, sigbuf, sigi, pch);
  }

  if (elen == 0) {
    // general tuple and slice can have no name
    if (struct_type == Struct_NamedTuple) {
      struct_type = Struct_GeneralTuple;
    } else if (struct_type == Struct_Slice) {
      // slice type is also anonymous
    } else {
      yyerror("(internal) this struct type `%s` have no name `%s`", struct_type, pch);
      return -1;
    }
  }
  // slice should have no name?

  if ((struct_type == Struct_NamedStruct && *pch != '}') ||
      (is_tuple_type(struct_type) && *pch != ')')) {
    if (*pch != ';') {
      if (struct_type)
	yyerror("(internal) bad format of struct style `%s` != ';' or ')'", pch);
      else
	yyerror("(internal) bad format of struct `%s` != ';' or '}'", pch);

      return -1;
    }

    sigbuf[sigi++] = *pch++; // = ';';
  } else if (is_general_struct_type(struct_type) && *pch != '>' ) { // struct_type == slice, union, enum
    if (*pch != ';') {
      yyerror("(internal) bad format of struct `%d` style `%s` != ';' or '>'", struct_type, pch);
      return -1;
    }

    sigbuf[sigi++] = *pch++; // = ';';
  }

  std::map<std::string, CADataType *> namemap = prenamemap;
  std::set<std::string> checkset = rcheckset;
  if (is_named_struct_type(struct_type))
    checkset.insert(namebuf);

  CADataType *dt = nullptr;
  CADataType *addrdt = nullptr;
  if (retdt) {
    int nameid = !is_named_struct_type(struct_type) ? -1 : symname_check_insert(namebuf);
    addrdt = catype_make_struct_type(nameid, *typesize, struct_type, 10);
    *retdt = addrdt;
  }

  if (is_named_struct_type(struct_type))
    namemap.insert(std::make_pair(namebuf, addrdt));

  while ((struct_type == Struct_NamedStruct && *pch != '}') ||
	 (is_tuple_type(struct_type) && *pch != ')') ||
	 (is_general_struct_type(struct_type) && *pch != '>')) {
    if (is_named_field_struct_type(struct_type)) {
      // extract field name
      int elen = signature_extract_name(namebuf, sigbuf, sigi, pch);
      if (elen == 0) {
	yyerror("(internal) this struct type `%d` member have no name `%s`", struct_type, pch);
	return -1;
      }

      if (*pch != ':') {
	yyerror("(internal) bad format of struct `%s` != ':' with type `%d`", pch, struct_type);
	return -1;
      }

      sigbuf[sigi++] = *pch++; // = ':';
    }

    int tbuflen = buflen - sigi;

    int tsize = 0;
    CADataType **outdt = retdt ? &dt : nullptr;
    int ret = catype_unwind_type_signature_inner(symtable, pch, namemap, checkset, sigbuf+sigi, tbuflen, &tsize, outdt);
    if (ret == -1) {
      yyerror("(internal) unwind type `%s` failed");
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

    if (retdt) {
      castruct_add_member(addrdt->struct_layout,
			  !is_named_field_struct_type(struct_type)
                              ? -1
                              : symname_check_insert(namebuf),
                          *outdt, 0);
    }
  }

  sigbuf[sigi++] = *pch++; // = '}'; or ')' when tuple, '>' when general structure
  buflen = sigi;

  if (sizeerror)
    *typesize = -1;
  else if (calcing)
    *typesize = -2;
  else
    *typesize = tmptypesize;

  // TODO: fillback typeid signature when possible or when it is needed to use these information
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
    yyerror("(internal) unwind type `%s` failed");
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

  // TODO: the number can be used when forming CADataType
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

/**
 * @brief Unwind type name
 * When the struct type has already been unwinded, simply append the name.
 * In structs with different members, if a later member already exists in a
previous
 * one and is used, this strategy can only compare in strict mode.
 * In non-strict mode, types must be expanded in all struct members,
 * even if a previous member has already expanded the same type.
 *
 * Example:
 * struct T1 { a: A, b: B, c: A }
 * struct T2 { a: A, b: B, c: B }
 *
 * If types A and B have been unwinded but the last A and B have not,
 * comparison cannot occur in non-strict mode because it��s unclear
 * if type A equals type B. Therefore, types must also be unwinded
 * when not in the recursive scope, requiring a check to see if
 * one type is within the recursive scope.
 *
 * @return
 * - -1 if there is an error
 * - 0 or a positive value indicates the consumption index.
 *
 * @note
 * non-strict comparing will ignore the name of structure, the enum etc.
 */
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
    /* discovered a recursively defined type, it's type size is recursive,
       so it may have unlimited size, like this:
       struct A { a: A, b: i32}
    */
    // TODO: return error number and the upper logic can print different error message
    fprintf(stderr, "type name `%s` is recursively defined: ", namebuf);
    return -1;
  }

  // when the name already processed, then the name here should be the recursive name
  auto itr = prenamemap.find(namebuf);
  if (itr != prenamemap.end()) {
    strcpy(sigbuf, namebuf);
    buflen = i;
    /*
     * When the type already appears in a previous context, -2 means the type size
     * is being calculated from above, and the type size of this layer depends
     * on the upper case.
     */
    *typesize = -2;

    if (!itr->second) {
      caerror(&entry->sloc, NULL, "type `%s` should recursive defining", namebuf);
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

  // when type is not a direct structure type
  if (!entry->u.datatype.members) {
    int tbuflen = buflen;

    // when ever retdt is null, it still use this
    namemap.insert(std::make_pair(namebuf, nullptr));
    int ret = catype_unwind_type_signature_inner(entry->u.datatype.idtable, caname, namemap, checkset, sigbuf, tbuflen, typesize, retdt);
    if (ret == -1) {
      yyerror("(internal) unwind type `%s` failed", caname);
      return -1;
    }

    if (*(caname+ret)) {
      yyerror("(internal) type contains extra text when unwind type `%s`", caname);
      return -1;
    }

    buflen = tbuflen;
    return i; // + ret;
  }

  /*
   * If it is a struct type definition, then the `namebuf` should be identical to the `caname`.
   * This way, when checking with the new set (checkset and namemap), it will always be found
   * because the upper layer just added it. When checking with the old set (rcheckset and
   * prenamemap), it will not be found, as it was previously located once. Thus, the struct
   * case is special, and no further checking is needed.
   */
  CADataType *addrdt = nullptr;
  if (retdt) {
    int nameid = symname_check_insert(namebuf);
    addrdt = catype_make_struct_type(nameid, *typesize, entry->u.datatype.tuple, 10);
    *retdt = addrdt;
  }

  namemap.insert(std::make_pair(namebuf, addrdt));

  int tmptypesize = 0;
  int sizeerror = 0;
  int calcing = 0;

  int sigi = 0;
  if (entry->u.datatype.tuple)
    sigi = sprintf(sigbuf, "(%s;", caname);
  else
    sigi = sprintf(sigbuf, "{%s;", caname);

  ST_ArgList *members = entry->u.datatype.members;
  for (int j = 0; j < members->argc; ++j) {
    typeid_t membertype = typeid_novalue;
    if (entry->u.datatype.tuple) {
      membertype = members->types[j];
    } else {
      const char *argname = symname_get(members->argnames[j]);
      STEntry *nameentry = sym_getsym(members->symtable, members->argnames[j], 0);
      if (nameentry->sym_type != Sym_Member) {
	yyerror("(internal) symbol type is not struct member for argument: `%s`", argname);
	return -1;
      }

      sigi += sprintf(sigbuf + sigi, "%s:", argname);
      membertype = nameentry->u.varshielding.current->datatype;
    }

    const char *mtypename = catype_get_type_name(membertype);
    int tbuflen = buflen - sigi;

    int tsize = 0;
    CADataType **outdt = retdt ? &dt : nullptr;
    int ret = catype_unwind_type_signature_inner(members->symtable, mtypename,
						 namemap, checkset, sigbuf + sigi, tbuflen, &tsize, outdt);
    if (ret == -1) {
      yyerror("(internal) unwind type `%s` failed");
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
      castruct_add_member(addrdt->struct_layout, members->argnames[j], *outdt, 0);
  }

  sigbuf[sigi-1] = entry->u.datatype.tuple ? ')' : '}'; // remove last ',' or ';' when it is empty struct
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
    yyerror("(internal) unwind type signature `%s` failed", caname);
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

/// skeleton, member-name, type-name
enum DataTypeCmpLevel {
  /**
   * (0 0) The type skeleton must be the same; other factors can
   * be different. You can compare the members' layout of the two
   * types, and if they are the same, it means the layout is identical.
   */
  DTCL_Skeleton,

  /**
   * (0 1) The type skeleton and type name must be the same; struct
   * member names can be different. Using different names cannot
   * ensure the program functions correctly.
   */
  DTCL_TypeName,

  /**
   * (1 0) The type skeleton and struct member names must be the same;
   * type names can be different. This model allows the program to function
   * correctly, so it should be used.
   */
  DTCL_Member,

  /**
   * (1 1) The type skeleton, type name, and struct member names must be
   * the same, similar to C.
   */
  DTCL_Strict,
};

/// @return true when strictly compare type name
static int catypeaux_strict_typename(DataTypeCmpLevel level) {
  return level == DTCL_TypeName || level == DTCL_Strict;
}

/// @return true when strictly compare member name
static int catypeaux_strict_membername(DataTypeCmpLevel level) {
  return level == DTCL_Member || level == DTCL_Strict;
}

/**
 * @brief Compare two types' signatures.
 *
 * @return
 *  1 if the signatures are the same,
 *  0 if they are not the same,
 * -1 if there is an error (e.g., format issue).
 *
 * @example
 * {A24;v1:{A10;b3:{A3;a:[f32;64]}},v2:{A21;c3:[{A18;c3:[{A12;b3:[{A3;a:[f32;64]};32]};32]};32]}}
 *
 * @note
 * Using namemap for comparing the data type skeleton can only handle
 * cases where the type name order is exchanged. It cannot compare
 * if the type names genuinely have the same member layout. This can be used
 * in a cache, such as a data layout cache, and when the cache does not hit,
 * the cache can be reconstructed.
 */
static int catype_compare_type_by_signature_str(std::map<std::string, std::string> &prenamemap,
						const char *caname1, const char *caname2,
						DataTypeCmpLevel cmplevel)
{
  if (cmplevel == DTCL_Strict)
    return !strcmp(caname1, caname2);

  // non-strict compare will ignoring the name of structure, the enum etc.
  std::map<std::string, std::string> namemap;

  /*
   * When ignoring type names, only consider the name after '{',
   * which is the struct name. For primitive type names, a strict comparison
   * should be performed.
   *
   * Although Rust allows the redefinition of primitive types,
   * but CA does not support that.
   */

  const char *pch1 = caname1;
  const char *pch2 = caname2;

  char namebuf1[1024];
  char namebuf2[1024];
  int i = 0, j = 0;

  int stricttype = catypeaux_strict_typename(cmplevel);
  int strictmember = catypeaux_strict_membername(cmplevel);

  /*
   * Does not parse the signature structure; instead, it uses a
   * simple algorithm to perform the task without type validation checking,
   * assuming the input is verified with different prefix characters.
   *
   * Different actions are token, the core idea is coping with the ID names and
   * identifying whether it is a type name or a struct member name:
   *
   * - '{': The next item is a type name.
   * - ';': The next item is a struct member name (when it is not numeric).
   * - ',': The next item is a struct member name.
   *
   * When encountering an alphabetic character, look back at the previous character
   * to determine actions:
   * - '{': Indicates a new name not in the name map; compare the type name.
   * - ';' or ',': Compare the struct member name.
   * - Otherwise: The following item is either a primitive type name
   *   or a previously accessed struct name.
   */
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

    // If a structure start is not encountered, the types must not be identical
    switch (*(pch1-1)) {
    case '{':
      // Compare sub-datatype; this is the first encounter
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

/**
 * @brief Compare two types' signatures.
 *
 * @note the strict indicates whether to perform a strict comparison.
 */
static int catype_compare_type_by_signature(std::map<std::string, std::string> &prenamemap,
					    typeid_t name1, typeid_t name2,
					    DataTypeCmpLevel cmplevel)
{
  const char *caname1 = catype_get_type_name(name1);
  const char *caname2 = catype_get_type_name(name2);
  return catype_compare_type_by_signature_str(prenamemap, caname1, caname2, cmplevel);
}

/**
 * @brief Calculate the type line's closure, which involves all
 * CADataType objects related to the type and inserting them into
 * the table with their signatures.
 *
 * Example:
 * struct A { b: B, c: C }
 * struct B { c: C }
 * struct C { a: *A }
 * type AA = A;
 * type BB = AA;
 *
 * - typeid(A) == {A; b: {B; c: {C; a: *A}}, c: {C; a: *A}}
 * - typeid(B) == {B; c: {C; a: *{A; b: B, c: C}}}
 * - typeid(C) == {C; a: *{A; b: {B; c: C}, c: C}}
 * - typeid(AA) == typeid(A)
 * - typeid(BB) == typeid(A)
 *
 * When input is 'struct A', it relates to types B and C. Therefore,
 * this function calculates the type IDs of A, B, and C, and finally
 * inserts them into s_type_map.
 */
void catype_make_type_closure(SymTable *symtable, typeid_t id) {
  // TODO:
  //catype_get_by_name();

}

/**
 * @brief Compact CADataType objects of pointers (*) or arrays ([]) into a single object.
 */
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
    yyerror("(internal) Assertion `name[0] == '*'' failed");
    return typeid_novalue;
  }
  typeid_t id = sym_form_type_id_by_str(name+1);
  return id;
}

/**
 * @brief Transform type t:*type => t:**type.
 */
static typeid_t typeid_increase_pointer(typeid_t type) {
  char buf[1024];
  const char *name = catype_get_type_name(type);
  buf[0] = '*';
  strcpy(buf+1, name);
  typeid_t id = sym_form_type_id_by_str(buf);
  return id;
}

/**
 * @brief Transform type t:[[type;num1];num2] => t:[type;num1].
 */
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

/**
 * @brief Transform type t:[type;num1] => t:[[type;num1];num2]
 */
static typeid_t typeid_increase_array(typeid_t type, int num) {
  const char *name = catype_get_type_name(type);
  char buf[1024];
  sprintf(buf, "[%s;%d]", name, num);
  typeid_t id = sym_form_type_id_by_str(buf);
  return id;
}

/// only need get the top level signature
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
  CAStructType struct_type = Struct_NamedStruct;

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

    struct_type = catype->struct_layout->type;

    rcheck.insert(catype);

    buf[0] = struct_type ? '(' : '{';
    structname = struct_type == Struct_GeneralTuple ? "" : symname_get(catype->struct_layout->name);
    len = strlen(structname) + 1;
    strcpy(buf+1, structname);
    buf[len++] = ';';
    for (int i = 0; i < catype->struct_layout->fieldnum; ++i) {
      auto *&type = catype->struct_layout->fields[i].type;

      //rcheck.insert(catype);
      subid = typeid_get_top_down(type, rcheck);
      //rcheck.erase(catype);

      subname = catype_get_type_name(subid);
      if (struct_type) {
	len += sprintf(buf+len, "%s,", subname);
      } else {
	varname = symname_get(catype->struct_layout->fields[i].name);
	len += sprintf(buf+len, "%s:%s,", varname, subname);
      }
    }

    rcheck.erase(catype);

    buf[len-1] = struct_type ? ')' : '}';
    buf[len] = '\0';
    catype->signature = sym_form_type_id_by_str(buf);
    catype->status = CADT_Expand;
    return catype->signature;
  case VOID:
  case I16:
  case I32:
  case I64:
  case U16:
  case U32:
  case U64:
  case F32:
  case F64:
  case BOOL:
  case I8:
  case U8:
  default:
    return catype->signature;
  }
}

static typeid_t typeid_from_catype(CADataType *catype) {
  // Used to check for circular references when containing struct types
  std::set<CADataType *> rcheck;
  return typeid_get_top_down(catype, rcheck);
}

/**
 * @brief Expand compacted pointers (*) or arrays ([]) into separate
 * CADataType objects.
 */
static CADataType *catype_formalize_type_expand(CADataType *datatype, std::set<CADataType *> &rcheck) {
  CADataType *nextdt = nullptr;
  CADataType *currdt = datatype;
  int dim = 0;
  int *parray = nullptr;

  while (true) {
    switch (currdt->type) {
    case POINTER:
      // TODO: Make the expanded signature (using clone) formal name the correct one.
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
      // TODO: make the expanded (use clone following) signature formalname to correct one.
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

static int catype_get_field_align(CADataType *type) {
  switch(type->type) {
  case POINTER:
    return sizeof(void *);
  case ARRAY:
    return catype_get_field_align(type->array_layout->type);
  case SLICE:
  case STRUCT:
    return type->struct_layout->fieldmaxalign;
  default:
    return type->size;
  }
}

static void catype_formalize_type_layout(CADataType *datatype, std::set<CADataType *> &rcheck) {
  switch (datatype->type) {
  case POINTER:
    catype_formalize_type_layout(datatype->pointer_layout->type, rcheck);
    break;
  case ARRAY:
    catype_formalize_type_layout(datatype->array_layout->type, rcheck);
    datatype->size = datatype->array_layout->dimarray[0] * datatype->array_layout->type->size;
    break;
  case STRUCT: {
    size_t offset = 0;
    int maxalign = 1;
    CAStruct *layout = datatype->struct_layout;
    for (int i = 0; i < layout->fieldnum; ++i) {
      CAStructField &field = layout->fields[i];
      if (rcheck.find(field.type) != rcheck.end())
	  continue;

      rcheck.insert(field.type);
      catype_formalize_type_layout(field.type, rcheck);
      rcheck.erase(field.type);

      int align = catype_get_field_align(field.type);
      if (offset % align != 0)
	offset += align - offset % align;

      field.offset = offset;
      offset += field.type->size;

      maxalign = std::max(maxalign, align);
    }

    layout->fieldmaxalign = maxalign;

    if (offset % maxalign != 0)
      offset += maxalign - offset % maxalign;

    datatype->size = offset;
    break;
  }
  default:
    break;
  }
}

static CADataType *catype_formalize_type(CADataType *datatype, int compact) {
  if (compact)
    return catype_formalize_type_compact(datatype);
  else {
    std::set<CADataType *> rcheck;
    rcheck.insert(datatype);
    datatype = catype_formalize_type_expand(datatype, rcheck);

    rcheck.clear();
    rcheck.insert(datatype);
    catype_formalize_type_layout(datatype, rcheck);
    return datatype;
  }
}

CADataType *catype_create_type_from_unwind(int unwind) {
  // TODO:
  const char *unwindstr = symname_get(unwind);

  // TODO:
  yyerror("(internal) unwinding...");
  return nullptr;
}

/*
 * 1. How to check for circular types?
 * The checking algorithm can be integrated into the calculation of type size.
 * When type A depends on type B, the size of type B must be calculated.
 * For example, A => B, B => C, C => A indicates a circle. When type X is
 * a composite type (e.g., struct), it depends on all its members,
 * thus requiring size calculations for its members.
 *
 * 2. How to represent a type?
 * A type may be recursively defined (e.g., struct A { a: *A }) without
 * forming a circle. Thus, it's not straightforward to represent it
 * structurally. Instead, an unwinded string can represent the type in the
 * CADataType object. This representation also simplifies operations
 * on complex types, as the * operator can return a step in types.
 *
 * 3. How to perform complex type operations, including *, & , etc.?
 *
 * 4. How to unwind a recursively defined type?
 * When encountering a struct name, check if it has appeared previously
 * in the definition. If so, this marks the end for this name; otherwise,
 * register the newly encountered name for future checks.
 *
 * 5. How to compare two signatures?
 * There are two methods to compare them:
 * - Strict: Compare the type structure and the names when containing
 *   structs.
 * - Unstrict: Only compare the type structure, ignoring the names of
 *   contained structs or fields.
 *
 * Q: How to perform unstrict comparisons?
 * A: Use equivalent exchanges. For instance, when encountering names
 * nameA-1 and nameB-1 in both signatures, create a map:
 * nameA-1 => nameB-1 and nameB-1 => nameA-1. When encountering
 * another pair of names, check the map to see if they exist and if they
 * are the same. If so, the names are identical; otherwise, they are not.
 */
CADataType *catype_get_by_name(SymTable *symtable, typeid_t name) {
  /*
   * Combine the primitive type table with the signature table.
   *
   * The process will follow these steps:
   * 1) Look for the type object in the `s_symtable_type_map` for faster access.
   * 2) If not found, unwind the type ID of the type.
   * 3) Search in the primitive (global datatype) table; if found, return the type object.
   * 4) If not found, use the unwound type ID to create the type object.
   * 5) Insert the object into the global table and the `s_symtable_type_map` table.
   */
  symtable = symtable_get_with_assoc(symtable, name);

  // step 1: find type from symtable type table
  typeid_t windst = sym_form_symtable_type_id(symtable, name);
  auto itr = s_symtable_type_map.find(windst);
  if (itr != s_symtable_type_map.end())
    return itr->second;

  // find type from global (signature) table
  auto itr2 = s_type_map.find(name);
  if (itr2 != s_type_map.end()) {
    s_symtable_type_map.insert(std::make_pair(windst, itr2->second));
    return itr2->second;
  }

  // step 2: get unwind id
  int typesize = 0;
  CADataType *dt = nullptr;
  typeid_t unwind = catype_unwind_type_signature(symtable, name, &typesize, &dt);
  if (unwind == typeid_novalue) {
    return nullptr;
  }

  // step 3: find type from global (signature) table
  itr2 = s_type_map.find(unwind);
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

CADataType *catype_from_capattern(CAPattern *cap, SymTable *symtable) {
  switch (cap->type) {
  case PT_Array:
  case PT_GenTuple: {
    /*
     * NEXT TODO: Handle array and general tuple definitions with types such as
     * `let (a, b): (i32, f64) = (1, 2.4)` or
     * `let [a, b]: [u8; 2] = [1, 2]`.
     */
    typeid_t *types = new typeid_t[cap->items->size];
    for (int i = 0; i < cap->items->size; ++i) {
      CADataType *dt = catype_from_capattern(cap->items->patterns[i], symtable);
      if (!dt)
	return nullptr;

      types[i] = dt->signature;
    }

    typeid_t type = typeid_novalue;
    if (cap->type == PT_Array) {
      for (int i = 1; i < cap->items->size; ++i) {
	if (types[i-1] != types[i]) {
	  caerror(&cap->loc, NULL, "Array pattern expected `%s` on `%d`, but found `%s`",
		  catype_get_type_name(types[i-1]), i,
		  catype_get_type_name(types[i]));
	  return nullptr;
	}
      }
      
      type = sym_form_array_id(types[0], cap->items->size);
    } else {
      type = sym_form_tuple_id(types, cap->items->size);
    }

    CADataType *catype = catype_get_by_name(symtable, type);
    if (!catype) {
      caerror(&cap->loc, nullptr, "get type `%s` failed", catype_get_type_name(type));
      return nullptr;
    }

    return catype;
  }
  case PT_Tuple:
  case PT_Struct: {
    typeid_t type = sym_form_type_id(cap->name);
    CADataType *catype = catype_get_by_name(symtable, type);
    if (!catype) {
      caerror(&cap->loc, nullptr, "get type `%s` by pattern failed", catype_get_type_name(type));
      return nullptr;
    }

    return catype;
  }
  case PT_Var: {
    STEntry *entry = sym_getsym(symtable, cap->name, 0);
    if (!entry)
      return nullptr;

    if (entry->sym_type != SymType::Sym_Variable) {
      caerror(&cap->loc, nullptr, "bad entry type `%d`, should be `variable`", entry->sym_type);
      return nullptr;
    }

    if (entry->u.varshielding.current->datatype == typeid_novalue)
      return nullptr;

    CADataType *catype = catype_get_by_name(symtable, entry->u.varshielding.current->datatype);
    return catype;
  }
  case PT_IgnoreOne:
  case PT_IgnoreRange:
    return nullptr;
  default:
    caerror(&(cap->loc), nullptr, "Unknown pattern type `%d` when create catype", cap->type);
    return nullptr;
  }
}

CADataType *catype_make_tuple_type(SymTable *symtable, CADataType **catypes, int len) {
  typeid_t *args = (typeid_t *)alloca(len * sizeof(typeid_t));
  for (int i = 0; i < len; ++i)
    args[i] = catypes[i]->signature;

  typeid_t type = sym_form_tuple_id(args, len);

  typeid_t windst = sym_form_symtable_type_id(symtable, type);
  auto itr = s_symtable_type_map.find(windst);
  if (itr != s_symtable_type_map.end())
    return itr->second;

  auto itr2 = s_type_map.find(type);
  if (itr2 != s_type_map.end()) {
    s_symtable_type_map.insert(std::make_pair(type, itr2->second));
    return itr2->second;
  }

  CADataType *dt = catype_make_struct_type(type, 0, Struct_GeneralTuple, len);
  dt->struct_layout->name = 0;

  size_t size = 0;
  for (int i = 0; i < len; ++i) {
    castruct_add_member(dt->struct_layout, 0, catypes[i], 0);
    size += catypes[i]->size;
  }
  
  //range = catype_get_by_name(symtable, type);

  dt->size = size;

  s_symtable_type_map.insert(std::make_pair(windst, dt));
  s_type_map.insert(std::make_pair(type, dt));

  return dt;
}

CADataType *catype_from_range(ASTNode *node, GeneralRangeType type, int inclusive, CADataType *startdt, CADataType *enddt) {
  SymTable *symtable = node->symtable;
  char sigbuf[4096] = {0, };
  CADataType *range = nullptr;
  if (startdt && enddt) {
    if (startdt->signature != enddt->signature) {
      caerror(&node->begloc, &node->endloc, "expect type `%s`, but found `%s`, both side of range need to be same type",
	      catype_get_type_name(startdt->signature), catype_get_type_name(enddt->signature));
      return nullptr;
    }

    CADataType *catypes[2] = {startdt, enddt};
    range = catype_make_tuple_type(symtable, catypes, 2);
    sprintf(sigbuf, "t:%s%s%s",
	    catype_get_type_name(startdt->signature),
	    inclusive ? "..=" : "..",
	    catype_get_type_name(enddt->signature));
  } else if (startdt) {
    range = startdt;
    sprintf(sigbuf, "t:%s..", catype_get_type_name(range->signature));
  } else if (enddt) {
    range = enddt;
    sprintf(sigbuf, "t:%s%s",
	    inclusive ? "..=" : "..",
	    catype_get_type_name(range->signature));
  } else {
    strcpy(sigbuf, "t:..");
  }

  auto datatype = new CADataType;
  datatype->range_layout = new CARange;
  datatype->range_layout->inclusive = inclusive;
  datatype->range_layout->start = startdt;
  datatype->range_layout->end = enddt;
  datatype->range_layout->type = type;
  datatype->range_layout->range = range;
  
  int formalname = symname_check_insert(sigbuf);
  datatype->status = CADT_Expand;
  datatype->formalname = formalname;
  datatype->type = RANGE;
  datatype->size = range ? range->size : 0;
  datatype->signature = formalname;

  typeid_t windst = sym_form_symtable_type_id(symtable, formalname);
  auto itr = s_symtable_type_map.find(windst);
  if (itr == s_symtable_type_map.end())
    s_symtable_type_map.insert(std::make_pair(windst, datatype));

  auto itr2 = s_type_map.find(formalname);
  if (itr2 == s_type_map.end())
    s_type_map.insert(std::make_pair(formalname, datatype));
  
  return datatype;
}

static int64_t parse_binary_number(const char *text, int len) {
  int i = 0;
  for (i = 0; i < len; ++i) {
    if (text[i] == '1')
      break;
  }
  len -= i;
  text += i;

  if (len > 64) {
    yyerror("the length `%d` of binary number `%s` out of range of 64", len, text);
    return 0;
  }

  // 1011
  int64_t v = 0;
  for (int i = 0; i < len; ++i) {
    v <<= 1;
    v += text[i] - '0';
  }

  return v;
}

static int literal_parse_integer(const char *text, tokenid_t littypetok, tokenid_t typetok, int64_t *i64value) {
  int len = strlen(text);
  int cmplen = littypetok == I64 ? 3 : 2;
  const char *cmpformat = littypetok == I64 ? "-0x" : "0x";
  const char *format = nullptr;
  bool neg = false;
  bool isbinary = false;

  if (len > cmplen && !strncmp(cmpformat, text, cmplen)) {
    format = "%p";
  } else {
    cmpformat = littypetok == I64 ? "-0o" : "0o";
    if (len > cmplen && !strncmp(cmpformat, text, cmplen)) {
      format = "%o";
      text += cmplen;
      if (cmplen == 3)
	neg = true;
    } else {
      cmpformat = littypetok == I64 ? "-0b" : "0b";
      if (len > cmplen && !strncmp(cmpformat, text, cmplen)) {
        isbinary = true;
	text += cmplen;
	if (cmplen == 3)
	  neg = true;
      } else {
	format = "%lu";
      }
    }
  }

  if (isbinary)
    *i64value = parse_binary_number(text, len - cmplen);
  else
    sscanf(text, format, i64value);

  if (neg)
    *i64value = -*i64value;

  int badscope = littypetok == I64 ?
    check_i64_value_scope(*i64value, typetok) :
    check_u64_value_scope((uint64_t)*i64value, typetok);

  return badscope;
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
    intentdeftype = I32;
    badscope = literal_parse_integer(text, lit->littypetok, I32, &lit->u.i64value);
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
  case I8:
    intentdeftype = I8;
    lit->u.i64value = (char)parse_lexical_char(text);
    badscope = check_char_value_scope(lit->u.i64value, I8);
    break;
  case U8:
    intentdeftype = I8;
    lit->u.i64value = (uint8_t)parse_lexical_char(text);
    badscope = check_uchar_value_scope(lit->u.i64value, U8);
    break;
  default:
    caerror(&lit->begloc, &lit->endloc, "void type have no literal value");
    return typeid_novalue;
  }

  if (badscope) {
    caerror(&lit->begloc, &lit->endloc, "bad literal value definition: %s cannot be %s",
	    get_type_string(lit->littypetok), get_type_string(intentdeftype));
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

    /*
     * For type inference, extract the first element to determine its type.
     * When other elements have different types, later usage will check for those differences.
     */
    typeid_t subid = inference_literal_type(sublit);
    if (!sublit->fixed_type) {
      yyerror("(internal) after inference it still cannot determine the literal type");
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

/**
 * @brief Infer and set the literal type for the literal when it does not
 * have a determined type. This differs from `determine_literal_type`,
 * which is used when passing a defined type.
 */
typeid_t inference_literal_type(CALiteral *lit) {
  if (lit->fixed_type) {
    // No inference needed; an error should be reported instead
    return lit->datatype;
  }

  switch(lit->littypetok) {
  case ARRAY:
    return inference_array_literal(lit);
  case STRUCT: {
    // not implemented
    caerror(&lit->begloc, &lit->endloc, "not implemented the literal for struct type.");
    return typeid_novalue;
  }
  case POINTER: {
    // should never come here
    caerror(&lit->begloc, &lit->endloc, "not implemented the literal for pointer type, should can never come here");
    return typeid_novalue;
  }
  default:
    return inference_primitive_literal_type(lit);
  }
}

static bool is_literal_zero_value(CALiteral *lit) {
  const char *v = symname_get(lit->textid);
  return lit->littypetok == U64 && !strcmp(v, "0");
}

/**
 * @brief Determine and set the literal type for the literal based on a
 * specified type. This differs from `inference_literal_type`, which does
 * not have a defined type parameter.
 */
void determine_primitive_literal_type(CALiteral *lit, CADataType *catype) {
  tokenid_t typetok = catype->type;
  if (typetok == tokenid_novalue || typetok == VOID)
    return;

  tokenid_t littypetok = lit->littypetok;

  // check convertable
  if (!is_literal_zero_value(lit) &&
      !literal_type_convertable(littypetok, typetok)) {
    caerror(&lit->begloc, &lit->endloc, "bad literal value definition: %s cannot be %s",
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
    badscope = literal_parse_integer(text, littypetok, typetok, &lit->u.i64value);
    break;
  case F64:
    lit->u.f64value = atof(text);
    badscope = check_f64_value_scope(lit->u.f64value, typetok);
    break;
  case BOOL:
    lit->u.i64value = atoll(text) ? 1 : 0;
    badscope = (typetok != BOOL);
    break;
  case I8:
    lit->u.i64value = (char)parse_lexical_char(text);
    badscope = check_char_value_scope(lit->u.i64value, typetok);
    break;
  case U8:
    lit->u.i64value = (uint8_t)parse_lexical_char(text);
    badscope = check_uchar_value_scope(lit->u.i64value, typetok);
    break;
  default:
    caerror(&lit->begloc, &lit->endloc, "%s type have no lexical value", get_type_string(littypetok));
    break;
  }

  if (badscope) {
    caerror(&lit->begloc, &lit->endloc, "bad literal value definition: %s cannot be %s",
	    get_type_string(littypetok), get_type_string(typetok));
    return;
  }

  lit->fixed_type = 1;
}

void determine_array_literal(CALiteral *lit, CADataType *catype) {
  if (lit->littypetok != ARRAY && catype->type != ARRAY) {
    yyerror("array type not identical: %d != %d\n", lit->littypetok, catype->type);
    return;
  }

  // assuming the catype object is expanded and normalized, so dimension must be 1
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

  // TODO: check if it can convert
  switch (littypetok) {
  case ARRAY:
    determine_array_literal(lit, catype);
    break;
  case CSTRING:
  case POINTER:
  case STRUCT:
    // TODO: implementing these kind of type conversion when encountered
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
    dt->pointer_layout->allocpos = CAPointerAllocPos::PP_Stack;
    break;
  case SLICE:
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

/**
 * @brief Form the data type signature based on the type and pointer notation.
 *
 * Rules:
 * - `type + '*'`
 * - `i32 + '*'` => `*i32`
 * - `*type + '*'` => `**type`
 * - `[type;n] + '*'` => `*[type;n]`
 * - `[[[type1;n1];n2];n3] + '*'` => `*[[[type1;n1];n2];n3]`
 *
 * Examples:
 * - `[*i32;n]`
 * - `[**i32;n]`
 * - `*[*i32;n]`
 * - `[**[*i32;n1];n2]`
 * - `*[[*i32;n1];n2]`
 */
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
  case 'c':
    // t:(;*i32,i64)
    sprintf(buf, "t:<slice;;%s,i64>", catype_get_type_name(type->signature));
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
  // TODO: use the type also from symbol
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
    // array and struct types can directly append the signature
    type = catype_make_type_symname(signature, POINTER, sizeof(void *));
    type->pointer_layout = new CAPointer;
    type->pointer_layout->dimension = 1;
    type->pointer_layout->type = datatype;
    type->pointer_layout->allocpos = CAPointerAllocPos::PP_Stack;
    break;
  }

  type->status = CADT_Expand;
  catype_put_primitive_by_name(signature, type);

  return type;
}

/**
 * @brief Handle the signature for structures, using the current symbol table
 * `name@address` as the signature test pointer type for parsing.
 *
 * This implementation realizes array and struct types:
 * - `[[[i32;2];3];4]` <==> `[i32;2;3;4]`
 * - `[[*[i32;2];3];4]` <==> `[*[i32;2];3;4]`
 *
 * The array representation uses the latter, which is the compact form.
 */
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

CADataType *catype_make_struct_type(int nameid, int typesize, CAStructType struct_type, int init_capacity) {
  CAStruct *castruct = new CAStruct;
  castruct->type = struct_type;
  castruct->name = nameid;
  castruct->fieldnum = 0;
  castruct->capacity = init_capacity;
  castruct->packed = 0;
  castruct->fieldmaxalign = 1;
  castruct->fields = new CAStructField[castruct->capacity];

  CADataType *struct_catype = catype_make_type_symname(nameid, STRUCT, typesize);
  struct_catype->struct_layout = castruct;

  return struct_catype;
}

int catype_check_identical(CADataType *type1, CADataType *type2) {
  return type1->signature == type2->signature;
}

int catype_check_identical_in_symtable(SymTable *st1, typeid_t type1, SymTable *st2, typeid_t type2) {
  /**
   * Note: When `type1 == type2`, it does not mean they are equal,
   * as they belong to different symbol tables.
   */
  if (type1 == type2 && st1 == st2)
    return 1;

  CADataType *dt1 = catype_get_by_name(st1, type1);
  CADataType *dt2 = catype_get_by_name(st2, type2);

  if (!dt1 || !dt2)
    return 0;

  // check not only the token type but also additional information when dealing with complex type
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
    caerror(loc, NULL, "the type `%s` not identical type `%s`",
	    symname_get(type1), symname_get(type2));
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

const char *catype_remove_impl_prefix(const char *name) {
  const char *pos = strrchr(name, ':');
  if (pos)
    return pos + 1;
  else
    return name;
}

const char *catype_struct_impl_id_to_function_name_str(typeid_t fnname) {
  const char *local_name_impl = catype_get_function_name(fnname);
  const char *local_name = catype_remove_impl_prefix(local_name_impl);
  return local_name;
}

typeid_t catype_struct_impl_id_to_function_name(typeid_t fnname) {
  const char *local_name = catype_struct_impl_id_to_function_name_str(fnname);
  return symname_check_insert(local_name);
}

/**
 * @brief Return values:
 * - 0: Function
 * - 1: Tuple
 * - -1: Error
 */
int extract_function_or_tuple(SymTable *symtable, int name, STEntry **entry, const char **fnname) {
  const char *fnname_ = catype_get_type_name(name);
  if (fnname)
    *fnname = fnname_;

  *entry = sym_getsym(symtable, name, 1);
  if (*entry) {  // if it really a function
    if ((*entry)->sym_type == Sym_FnDecl || (*entry)->sym_type == Sym_FnDef) {
      return 0;
    }

    return -1;
  }

  int tupleid = symname_check(fnname_);
  if (tupleid == -1)
    return -1;

  tupleid = sym_form_type_id(tupleid);
  *entry = sym_getsym(symtable, tupleid, 1);
  if (*entry && (*entry)->sym_type == Sym_DataType)
    return 1;

  return -1;
}

typeid_t catype_trait_self_id() {
  typeid_t self = sym_form_type_id_by_str(CSELF);
  return self;
}

typeid_t catype_trait_self_ptr_id() {
  typeid_t self = catype_trait_self_id();
  return sym_form_pointer_id(self);
}

/**
 * @brief Unwind the type into a type object when it has object-like members.
 *
 * Examples:
 *
 * Example 1:
 * - Input: `t:AA; type AA = *[*[*BB;3];4]; type BB = *CC; type CC = [*i32;6];`
 * - After unwind: `*[*[**[*i32;6];3];4]`
 *
 * Example 2:
 * - Input: `t:AA; type AA = *[*[*BB;3];4]; struct BB { a: CC }; type CC = *[i32;6];`
 * - After unwind: `*[*[*{BB;a:*[i32;6]};3];4]`
 */
END_EXTERN_C

int64_t parse_to_int64(CALiteral *value) {
  if (catype_is_float(value->littypetok))
    return (int64_t)value->u.f64value;
  else
    return value->u.i64value;
}

double parse_to_double(CALiteral *value) {
  if (catype_is_float(value->littypetok))
    return value->u.f64value;
  else
    return (double)value->u.i64value;
}

int can_type_binding(CALiteral *lit, tokenid_t typetok) {
  // TODO: Use type from the symbol table. When support literals for arrays or structs, e.g., `AA {d, b}`.
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
  case I8:
    return !check_char_value_scope(lit->u.i64value, typetok);
  case U8:
    return !check_uchar_value_scope(lit->u.i64value, typetok);
  default:
    yyerror("bad lexical literal type: '%s'", get_type_string(dt->type));
    return 0;
  }
}

const char *get_printf_format(int type) {
  switch (type) {
  case VOID:
    yyerror("(internal) void type have no format value");
    return "\n";
  case I16:
    return "%d";
  case I32:
    return "%d";
  case I64:
    return "%ld";
  case U16:
    return "%u";
  case U32:
    return "%u";
  case U64:
    return "%lu";
  case F32:
    return "%f";
  case F64:
    return "%lf";
  case I8:
    return "%c";
  case U8:
    return "%c";
  case BOOL:
    return "%1d";
  case POINTER:
    return "%p";
  default:
    return "\n";
  }
}

/**
 * @brief The table is used for literal value conversion.
 *
 * 1. For types before `ATOMTYPE_END`, the following rules apply:
 *    - The left side is a lexical literal value
 *      (e.g., `I64` for negative integers,
 *       `U64` for positive integers,
 *       `F64` for floating point values).
 *    - The right side is the real literal value.
 *
 * 2. For types after `ATOMTYPE_END`, different rules apply:
 *    - They do not require scope checking, as they are non-primitive types.
 *
 * Supported types:
 * - VOID
 * - I16
 * - I32
 * - I64
 * - U16
 * - U32
 * - U64
 * - F32
 * - F64
 * - BOOL
 * - I8
 * - U8
 * - ATOMTYPE_END
 * - STRUCT
 * - ARRAY
 * - POINTER
 * - CSTRING
 */
static int s_literal_type_convertable_table[ATOMTYPE_END - VOID + 1][CSTRING - ATOMTYPE_END + ATOMTYPE_END - VOID + 1] = {
  {0, }, // VOID -> other-type, means convert from VOID type to other type
  {0, },   // I16 -> other-type
  {0, },   // I32 -> other-type
  {0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0 /* ATOMTYPE_END */, 0, 0, 0, 1,}, // I64 ->
  {0, }, // U16 ->
  {0, }, // U32 ->
  {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0 /* ATOMTYPE_END */, 0, 0, 0, 1,}, // U64 ->
  {0, }, // F32 ->
  {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0 /* ATOMTYPE_END */, 0, 0, 0, 0,}, // F64 ->
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 /* ATOMTYPE_END */, 0, 0, 0, 0,}, // BOOL ->
  {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0 /* ATOMTYPE_END */, 0, 0, 0, 1,}, // I8 ->
  {0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0 /* ATOMTYPE_END */, 0, 0, 0, 1,}, // U8 ->
  {0, }, // ATOMTYPE_END
  // {0, }, // STRUCT, these 4 should just is stub, they should not come here
  // {0, }, // ARRAY ->
  // {0, }, // POINTER ->
  // {0, }, // CSTRING ->
};

int literal_type_convertable(tokenid_t from, tokenid_t to) {
  return s_literal_type_convertable_table[from-VOID][to-VOID];
}

/**
 * @brief Type convertibility using `as` in Rust.
 *
 * The `as` operator cannot:
 * - Convert other types to `bool`.
 * - Convert `int`/`float` to any primitive type, except to pointer (`*`),
 *   but not to reference (`&`).
 *
 * It can:
 * - Convert `bool` to `int`, but not to `float` or `char`.
 * - Only `u8` can be cast to `char`, but `char` can be converted to `int`.
 *
 * It seems that using token types is preferable to using `typeid_t` types.
 */
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

/**
 * @brief Check if the specified type (`typetok`) can accept a literal value.
 */
int check_i64_value_scope(int64_t lit, tokenid_t typetok) {
  // the match table should match the corrsponding line in table s_literal_type_convertable_table
  switch(typetok) {
  case I16:
    if (lit < std::numeric_limits<int16_t>::min())
      return 1;
    return 0;
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
  // the match table should match the corrsponding line in table s_literal_type_convertable_table
  switch(typetok) {
  case I16:
    if (lit > std::numeric_limits<int16_t>::max())
      return -1;
    return 0;
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
  case U16:
    if (lit > std::numeric_limits<uint16_t>::max())
      return -1;
    return 0;
  case U32:
    if (lit > std::numeric_limits<uint32_t>::max())
      return -1;
    return 0;
  case I8:
    if (lit > std::numeric_limits<char>::max())
      return -1;
    return 0;
  case U8:
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
  // the match table should match the corrsponding line in table s_literal_type_convertable_table
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
  if (typetok == U8 && lit < 0)
    return 1;

  return 0;
}

int check_uchar_value_scope(uint8_t lit, tokenid_t typetok) {
  if (typetok == I8 && lit > 127)
    return -1;

  return 0;
}

bool catype_is_float(tokenid_t typetok) {
  return (typetok == F32 || typetok == F64);
}

bool catype_is_complex_type(CADataType *catype) {
  if (catype->type == RANGE && catype->range_layout->range)
    return catype_is_complex_type(catype->range_layout->range);

  switch (catype->type) {
  case ARRAY:
  case SLICE:
  case STRUCT:
    return true;
  case RANGE:
    switch (catype->range_layout->type) {
    case FullRange:
      // should not come here
      //yyerror("should not come here for full range");
      return true;
    case InclusiveRange:
    case RightExclusiveRange:
    case InclusiveRangeTo:
    case RightExclusiveRangeTo:
    case RangeFrom:
      //return catype_is_complex_type(catype->range_layout->range);
      return true;
    default:
      yyerror("bad range type: %d", catype->range_layout->type);
      return false;
    }
  case POINTER:
  default:
    return false;
  }
}

bool catype_is_signed(tokenid_t type) {
  return type == I8 || type == I16 || type == I32 || type == I64;
}

bool catype_is_unsigned(tokenid_t type) {
  return type == U8 || type == U16 || type == U32 || type == U64;
}

bool catype_is_integer(tokenid_t type) {
  switch (type) {
  case I8:
  case I16:
  case I32:
  case I64:
  case U8:
  case U16:
  case U32:
  case U64:
    return true;
  default:
    return false;
  }
}

bool catype_is_integer_range(CADataType *catype) {
  if (catype->type != RANGE)
    return false;

  switch (catype->range_layout->type) {
  case FullRange:
    return true;
  case RangeFrom:
  case InclusiveRange:
  case RightExclusiveRange:
    return catype_is_integer(catype->range_layout->start->type);
  case InclusiveRangeTo:
  case RightExclusiveRangeTo:
    return catype_is_integer(catype->range_layout->end->type);
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

void castruct_add_member(CAStruct *castruct, int name, CADataType *dt, size_t offset) {
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
  castruct->fields[castruct->fieldnum].offset = offset;
  castruct->fieldnum++;
}

CADataType *slice_create_catype(CADataType *item_catype) {
  CADataType *catype = catype_make_struct_type(0, 2 * sizeof(void *), Struct_GeneralTuple, 2);
  catype->status = CADT_Expand;

  CADataType *itemptr_catype = catype_make_pointer_type(item_catype);
  castruct_add_member(catype->struct_layout, 0, itemptr_catype, 0);

  CADataType *size_catype = catype_get_primitive_by_token(I64);
  castruct_add_member(catype->struct_layout, 0, size_catype, sizeof(void *));

  catype->type = SLICE;

  typeid_t signature = form_datatype_signature(itemptr_catype, 'c', 0);

  // t:(;*i32,i64)
  catype->signature = signature;
  return catype;
}


