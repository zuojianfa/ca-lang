#include "dwarf_debug.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/BinaryFormat/Dwarf.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include <linux/limits.h>
#include <memory>

namespace dwarf_debug {

DWARFDebugInfo::DWARFDebugInfo(llvm::IRBuilder<> &builder, llvm::Module &module,
                               const char *src_path)
  : builder(builder), dibuilder(std::make_unique<llvm::DIBuilder>(module)) {
  char dir[PATH_MAX];
  const char *file;
  const char *pos = strchr(src_path, '/');
  if (pos != NULL) {
    strncpy(dir, src_path, pos - src_path);
    dir[pos - src_path] = '\0';
    file = pos + 1;
  } else {
    dir[0] = '.'; dir[1] = '\0';
    file = src_path;
  }

  module.addModuleFlag(llvm::Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);
  llvm::DIFile *difile = dibuilder->createFile(file, dir);
  dicu = dibuilder->createCompileUnit(llvm::dwarf::DW_LANG_C, difile, "ca compiler", 0, "", 0);
}

void DWARFDebugInfo::emit_location(int row, int col) {
  if (row == -1)
    return builder.SetCurrentDebugLocation(llvm::DebugLoc());

  llvm::DIScope *scope;
  if (lexical_blocks.empty())
    scope = this->dicu;
  else
    scope = lexical_blocks.back();

  auto loc = llvm::DILocation::get(scope->getContext(), row, col, scope);
  builder.SetCurrentDebugLocation(loc);
}

llvm::DIType *DWARFDebugInfo::int_type_di() {
  if (ditype)
    return ditype;

  //ditype = DBuilder->createBasicType("double", 64, llvm::dwarf::DW_ATE_float);
  ditype = dibuilder->createBasicType("int", 32, llvm::dwarf::DW_ATE_signed);
  return ditype;
}

llvm::DISubroutineType *DWARFDebugInfo::fn_type_di(int fargc, llvm::DIFile *unit) {
  llvm::SmallVector<llvm::Metadata *, 8> types;
  llvm::DIType *diintty = int_type_di();

  // add the result type
  types.push_back(diintty);

  // the argument type
  for (int i = 0; i < fargc; ++i) {
    types.push_back(diintty);
  }

  return dibuilder->createSubroutineType(dibuilder->getOrCreateTypeArray(types));
}

llvm::DIFile *DWARFDebugInfo::create_difile() {
  llvm::DIFile *unit = dibuilder->createFile(dicu->getFilename(), dicu->getDirectory());
  return unit;
}

} // namespace dwarf_debug

