#ifndef __symtable_cpp_h__
#define __symtable_cpp_h__

#include "ca_types.h"
#include <map>
#include <set>

struct TraitNodeInfo {
  std::map<typeid_t, struct ASTNode *> fnnodes;
  std::set<typeid_t> ids_with_def; // the function id with default function definition
  std::set<typeid_t> ids_no_def;
};

#endif

