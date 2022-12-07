#ifndef __codegen_jit1_h__
#define __codegen_jit1_h__

#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/Mangling.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/Orc/SymbolStringPool.h"
#include "llvm/Target/TargetMachine.h"
#include <vector>
#if LLVM_VERSION > 12
#include "llvm/ExecutionEngine/Orc/ExecutorProcessControl.h"
#else
#include "llvm/ExecutionEngine/Orc/TargetProcessControl.h"
#endif
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Support/Error.h"
#include <memory>
#include <utility>

namespace jit_codegen {
using namespace llvm;

class JIT1 {
public:
 JIT1(std::unique_ptr<orc::ExecutionSession> es,
      orc::JITTargetMachineBuilder builder, DataLayout dl)
   : _es(std::move(es)), _dl(std::move(dl)), _mangle(*this->_es, this->_dl),
    _obj_layer(*this->_es, []() { return std::make_unique<SectionMemoryManager>(); }),
    _compile_layer(*this->_es, _obj_layer,
		   std::make_unique<orc::ConcurrentIRCompiler>(std::move(builder))),
    _main_jitdl(this->_es->createBareJITDylib("<main>")) {
    _main_jitdl.addGenerator(cantFail(orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(this->_dl.getGlobalPrefix())));
  }

  ~JIT1() {
    if (auto err = _es->endSession())
      _es->reportError(std::move(err));
  }

  // this function can be called multiple time to register more symbols
  void register_one_imported_symbol(const char *name, void *address) {
    auto abs_symbol = orc::absoluteSymbols({
	{_mangle(name), JITEvaluatedSymbol(pointerToJITTargetAddress(address), JITSymbolFlags::Exported)},
      });

    cantFail(this->_main_jitdl.define(abs_symbol));
  }

  // this function can be called multiple time to register more symbols
  template <typename CAN_STRING>
  void register_imported_symbols(std::vector<std::pair<CAN_STRING, void *>> &name_addresses) {
    orc::SymbolMap symbol_map;
    for (auto na : name_addresses) {
      symbol_map[_mangle(na.first)] = JITEvaluatedSymbol(pointerToJITTargetAddress(na.second), JITSymbolFlags::Exported);
    }

    auto abs_symbol = orc::absoluteSymbols(symbol_map);
    cantFail(this->_main_jitdl.define(abs_symbol));
  }

  static llvm::Expected<std::unique_ptr<JIT1>> create_instance() {
#if LLVM_VERSION > 12
    auto epc = orc::SelfExecutorProcessControl::Create();
    if (!epc)
      return epc.takeError();

    auto es = std::make_unique<orc::ExecutionSession>(std::move(*epc));
    orc::JITTargetMachineBuilder builder(es->getExecutorProcessControl().getTargetTriple());
#else // LLVM12
    auto ssp = std::make_shared<orc::SymbolStringPool>();
    auto tpc = orc::SelfTargetProcessControl::Create(ssp);
    if (!tpc)
      return tpc.takeError();

    auto es = std::make_unique<orc::ExecutionSession>(std::move(ssp));
    orc::JITTargetMachineBuilder builder(tpc.get()->getTargetTriple());
#endif

    auto dl = builder.getDefaultDataLayoutForTarget();
    if (!dl)
      return dl.takeError();

    return std::make_unique<JIT1>(std::move(es), std::move(builder), std::move(*dl));
  }

  const DataLayout &get_datalayout() const { return _dl; }
  orc::JITDylib &get_main_jitdl() { return _main_jitdl; }
  llvm::Error add_module(orc::ThreadSafeModule tsm, orc::ResourceTrackerSP rt = nullptr) {
    if (!rt)
      rt = _main_jitdl.getDefaultResourceTracker();

    return _compile_layer.add(rt, std::move(tsm));
  }

  llvm::Expected<JITEvaluatedSymbol> find(StringRef name) {
    return _es->lookup({&_main_jitdl}, _mangle(name.str()));
  }

private:
  std::unique_ptr<llvm::orc::ExecutionSession> _es;
  DataLayout _dl;
  orc::MangleAndInterner _mangle;
  orc::RTDyldObjectLinkingLayer _obj_layer;
  orc::IRCompileLayer _compile_layer;
  orc::JITDylib &_main_jitdl;
};
}

#endif

