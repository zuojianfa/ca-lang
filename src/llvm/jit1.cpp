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

#include "jit1.h"

#include "llvm/ExecutionEngine/Orc/SymbolStringPool.h"

#if LLVM_VERSION > 12
#include "llvm/ExecutionEngine/Orc/ExecutorProcessControl.h"
#else
#include "llvm/ExecutionEngine/Orc/TargetProcessControl.h"
#endif

#include "llvm/ExecutionEngine/SectionMemoryManager.h"

#include <memory>
#include <utility>

namespace jit_codegen {
  using namespace llvm;

  JIT1::JIT1(std::unique_ptr<orc::ExecutionSession> es,
      orc::JITTargetMachineBuilder builder, DataLayout dl)
   : _es(std::move(es)), _dl(std::move(dl)), _mangle(*this->_es, this->_dl),
    _obj_layer(*this->_es, []() { return std::make_unique<SectionMemoryManager>(); }),
    _compile_layer(*this->_es, _obj_layer,
		   std::make_unique<orc::ConcurrentIRCompiler>(std::move(builder))),
    _main_jitdl(this->_es->createBareJITDylib("<main>")) {
    _main_jitdl.addGenerator(cantFail(orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(this->_dl.getGlobalPrefix())));
  }

  JIT1::~JIT1() {
    if (auto err = _es->endSession())
      _es->reportError(std::move(err));
  }

  // this function can be called multiple time to register more symbols
  void JIT1::register_one_imported_symbol(const char *name, void *address) {
    auto abs_symbol = orc::absoluteSymbols({
	{_mangle(name), JITEvaluatedSymbol(pointerToJITTargetAddress(address), JITSymbolFlags::Exported)},
      });

    cantFail(this->_main_jitdl.define(abs_symbol));
  }

  void JIT1::register_imported_symbols(
      std::vector<std::pair<const char *, void *>> &name_addresses) {
    orc::SymbolMap symbol_map;
    for (auto na : name_addresses) {
      symbol_map[_mangle(na.first)] = JITEvaluatedSymbol(
          pointerToJITTargetAddress(na.second), JITSymbolFlags::Exported);
    }

    auto abs_symbol = orc::absoluteSymbols(symbol_map);
    cantFail(this->_main_jitdl.define(abs_symbol));
  }

  llvm::Expected<std::unique_ptr<JIT1>> JIT1::create_instance() {
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

  llvm::Error JIT1::add_module(orc::ThreadSafeModule tsm, orc::ResourceTrackerSP rt) {
    if (!rt)
      rt = _main_jitdl.getDefaultResourceTracker();

    return _compile_layer.add(rt, std::move(tsm));
  }

  llvm::Expected<JITEvaluatedSymbol> JIT1::find(StringRef name) {
    return _es->lookup({&_main_jitdl}, _mangle(name.str()));
  }
}

