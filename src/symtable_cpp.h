#ifndef __symtable_cpp_h__
#define __symtable_cpp_h__

#include "ca_types.h"
#include <map>
#include <memory>
#include <set>

struct TraitNodeInfo {
  std::map<typeid_t, struct ASTNode *> fnnodes;
  std::set<typeid_t> ids_with_def; // the function id with default function implementation
  std::set<typeid_t> ids_no_def;   // the function id with no default implementation
};

struct MethodImplInfo {
  int fnname; // function name
  struct STEntry *nameentry; // symbol table entry of function name
  int fnname_manged; // the mangled function name
};

typedef std::map<int, std::unique_ptr<MethodImplInfo>> impl_info_map_t;
typedef std::map<int, struct SymTableAssoc *> impl_assoc_info_map_t;

// The trait implemented function / method is in different map (the trait map)
struct StructImplInfo {
  impl_info_map_t method_in_struct;

  // key: trait id, value method in trait map
  std::map<int, impl_info_map_t> method_in_traits;

  // key: trait id, value method in trait map
  std::map<int, impl_assoc_info_map_t> method_in_traits_copied;
};

#endif

