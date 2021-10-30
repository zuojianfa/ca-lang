add_library(ir1 STATIC dwarf_debug.cpp ir1.cpp)

set(ir1_cxxflags -I/usr/lib/llvm/12/include -std=c++14    -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS)
message("llvm: " ${llvm_cxxflags})
target_compile_options(ir1 PRIVATE ${ir1_cxxflags})

target_include_directories(ir1 PRIVATE .)

install(TARGETS ir1 DESTINATION lib)
install(FILES IR_generator.h dwarf_debug.h ir1.h jit1.h DESTINATION include)

