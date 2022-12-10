#ifndef __codegen_dwarf_debug_h__
#define __codegen_dwarf_debug_h__

#include "llvm/BinaryFormat/Dwarf.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace dwarf_debug {

class DWARFDebugInfo {
public:
  DWARFDebugInfo(llvm::IRBuilder<> &builder, llvm::Module &module, const char *src_path);

  void initialize_types();
  llvm::DIType *get_ditype(const char *type);
  void put_ditype(const char *type, llvm::DIType *ditype);

  void emit_location(int row = -1, int col = 0, llvm::DIScope *discope = nullptr);
  void set_difile(llvm::DIFile *difile) { this->difile = difile; }
  llvm::DIFile *get_difile() { return difile; }
  llvm::DIFile *create_difile();

  llvm::DIType *int_type_di();
  llvm::DISubroutineType *fn_type_di(int fargc, llvm::DIFile *unit);

public:
  llvm::DICompileUnit *dicu;
  llvm::DIType *ditype;
  std::vector<llvm::DIScope *> lexical_blocks;
  llvm::IRBuilder<> &builder;
  std::unique_ptr<llvm::DIBuilder> dibuilder;
  llvm::DIFile *difile;

private:
  std::unordered_map<std::string, llvm::DIType *> _ditypes;
};

}

#endif
