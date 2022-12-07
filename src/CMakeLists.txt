
find_package(FLEX)
FLEX_TARGET(ca_lexier ca.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)

# using following instead of the above, when using regular bison
find_package(BISON)
BISON_TARGET(ca_parser ca.y ${CMAKE_CURRENT_BINARY_DIR}/ca.tab.c
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/ca.tab.h)

ADD_FLEX_BISON_DEPENDENCY(ca_lexier ca_parser)

add_subdirectory(llvm)

# handle special target for IR_generator.cpp
add_library(irgen OBJECT llvm/IR_generator.cpp type_system_llvm.cpp ca.tab.h)
target_compile_options(irgen PRIVATE ${llvm_cxxflags})
target_include_directories(irgen PRIVATE . ${CMAKE_CURRENT_BINARY_DIR})

#add_dependencies(irgen ca.tab.h)

#  ca.h config.h dotgraph.h symtable.h utils.h llvm/IR_generator.h ca.l ca.y
add_executable(ca ca.cpp ca.tab.c lex.yy.c ca_parser.c dotgraph.cpp symtable_cpp.cpp type_system.cpp utils.c strutil.c ca_runtime.c $<TARGET_OBJECTS:irgen>)
target_link_options(ca PRIVATE ${llvm_ldflags}
  # the option -Xlinker --export-dynamic make the symbol exported as dynamic, for example: for rt_add function
  # when not use following option rt_add will not exported in the dynamic symbol table and
  # and when use the option rt_add will appear in dynamic symbol table
  # and when it appear in dynamic symbol table, the llvm JIT will automatically can get the function when run
  # in JIT, when not appear in dynamic symbol table JIT will cannot find the symbol automatically and will get
  # an error of:
  # JIT session error: Symbols not found: [ rt_add ]
  # Failed to materialize symbols: { (<main>, { main }) }
  # when not using following option and want use the function `rt_add`, you need tell the JIT explicitly like
  # ```
  #    orc::SymbolMap symbol_map;
  #    symbol_map[_mangle("rt_add")] = JITEvaluatedSymbol(pointerToJITTargetAddress(&rt_add), JITSymbolFlags::Exported);
  #    auto abs_symbol = orc::absoluteSymbols(symbol_map);
  #    cantFail(_main_jitdl.define(abs_symbol));
  # ```
  # or
  # ```
  #    orc::SymbolMap symbol_map;
  #    auto abs_symbol = orc::absoluteSymbols({{_mangle("rt_add"), JITEvaluatedSymbol(pointerToJITTargetAddress(&rt_add), JITSymbolFlags::Exported)}});
  #    cantFail(_main_jitdl.define(abs_symbol));
  # ```
  # open following flag without defining upper logic in JIT1 class, but following option will export all symbols even unwanted
  #-Xlinker --export-dynamic
  )

# default to debug version without an postfix, when need it just uncomment following one
#set_target_properties(ca PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
set_target_properties(ca PROPERTIES RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX})
set_target_properties(ca PROPERTIES RELWITHDEBINFO_POSTFIX ${CMAKE_RELWITHDEBINFO_POSTFIX})

target_include_directories(ca PUBLIC . llvm ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(ca PUBLIC ir1 gc)

install(TARGETS ca DESTINATION bin)
install(FILES README.md LICENSE DESTINATION include)

string(TOUPPER ${CMAKE_BUILD_TYPE} current_build_type)
string(COMPARE EQUAL ${current_build_type} "DEBUG" current_build_type_debug)
string(COMPARE EQUAL ${current_build_type} "RELEASE" current_build_type_release)
if (${current_build_type_debug} EQUAL true)
#  set(current_target_name ${}
endif()

add_custom_command(TARGET ca POST_BUILD COMMAND cp ca${OUTPUT_NAME_RELEASE} ${CMAKE_SOURCE_DIR})

