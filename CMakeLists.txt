cmake_minimum_required(VERSION 3.20.0)

project("ca"
  VERSION 0.1
  DESCRIPTION "ca: a rust like compiler with jit and compatible with C ABI powered by llvm"
  HOMEPAGE_URL "https://gitee.com/zuojianfa/ca"
  LANGUAGES CXX C YACC LEX)

add_subdirectory(llvm)

#  ca.h config.h dotgraph.h symtable.h utils.h llvm/IR_generator.h
add_executable(ca ca_parser.c ca.l ca.y dotgraph.cpp symtable_cpp.cpp utils.c llvm/IR_generator.cpp)

target_include_directories(ca . llvm)


