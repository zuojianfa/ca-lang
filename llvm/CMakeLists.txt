cmake_minimum_required(VERSION 3.17.0)

project(ca
  VERSION 0.1
  DESCRIPTION "ca: a rust like compiler with jit and compatible with C ABI powered by llvm"
  HOMEPAGE_URL "https://gitee.com/zuojianfa/ca"
  LANGUAGES CXX C)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER clang++)

execute_process(COMMAND llvm-config "--cxxflags" OUTPUT_VARIABLE CLANGPP_CXXFLAGS)
string(REPLACE "\n" " " CLANGPP_CXXFLAGS ${CLANGPP_CXXFLAGS})
message("cxxflags: " ${CLANGPP_CXXFLAGS})

# -I/usr/lib/llvm/12/include -std=c++14    -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
add_compile_options(${CLANGPP_CXXFLAGS})
add_library(ir1 STATIC dwarf_debug.cpp ir1.cpp)
#target_compile_options(ir1 PUBLIC ${CLANGPP_CXXFLAGS})

target_include_directories(ir1 PUBLIC .)

install(TARGETS ir1 DESTINATION lib)
install(FILES IR_generator.h dwarf_debug.h ir1.h jit1.h DESTINATION include)

