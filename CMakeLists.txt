cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER clang++)
#set(CMAKE_YACC_COMPILER bison)
#set(CMAKE_LEX_COMPILER flex)

set(CMAKE_RELEASE_POSTFIX r)
set(CMAKE_DEBUG_POSTFIX d)

#find_package(BISON)
#BISON_TARGET(caparser ca.y ${CMAKE_CURRENT_SOURCE_AADIR}/ca.tab.c COMPILE_FLAGS "-d")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ca.tab.h ${CMAKE_CURRENT_BINARY_DIR}/ca.tab.c ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c
  COMMAND bison -d ${CMAKE_CURRENT_SOURCE_DIR}/ca.y
  COMMAND flex ${CMAKE_CURRENT_SOURCE_DIR}/ca.l
  )

project(ca
  VERSION 0.1
  DESCRIPTION "ca: a rust like compiler with jit and compatible with C ABI powered by llvm"
  HOMEPAGE_URL "https://gitee.com/zuojianfa/ca"
  LANGUAGES CXX C)

#add_executable(llvm-config IMPORTED)
# get CFLAGS using llvm-config
execute_process(COMMAND llvm-config "--cxxflags"
  OUTPUT_VARIABLE CLANGPP_CXXFLAGS)

# remove the last \n character of llvm-config command
string(REPLACE "\n" " " CLANGPP_CXXFLAGS ${CLANGPP_CXXFLAGS})

# get LDFLAGS using llvm-config
execute_process(COMMAND llvm-config --ldflags --system-libs --libs core orcjit native
  OUTPUT_VARIABLE CLANGPP_LDFLAGS)

# remove the last \n character of llvm-config command
string(REPLACE "\n" " " CLANGPP_LDFLAGS ${CLANGPP_LDFLAGS})


set(CMAKE_CXX_FLAGS "${CLANGPP_CXXFLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CLANGPP_CXXFLAGS} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CLANGPP_CXXFLAGS} -O2")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CLANGPP_CXXFLAGS} -O2 -g")

set(CMAKE_EXE_LINKER_FLAGS "${CLANGPP_LDFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CLANGPP_LDFLAGS} -g")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CLANGPP_LDFLAGS} -O2")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CLANGPP_LDFLAGS} -O2 -g")

add_subdirectory(llvm)

#  ca.h config.h dotgraph.h symtable.h utils.h llvm/IR_generator.h ca.l ca.y
add_executable(ca ca.tab.c lex.yy.c ca_parser.c dotgraph.cpp symtable_cpp.cpp utils.c llvm/IR_generator.cpp)
set_target_properties(ca PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
set_target_properties(ca PROPERTIES RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX})

target_include_directories(ca PUBLIC . llvm ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(ca PUBLIC ir1)

