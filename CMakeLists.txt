cmake_minimum_required(VERSION 3.16.0)

project(ca
  VERSION 0.1
  DESCRIPTION "ca: a rust like compiler with jit and compatible with C ABI powered by llvm"
  HOMEPAGE_URL "https://gitee.com/zuojianfa/ca"
  LANGUAGES CXX C)

message("OS: " ${CMAKE_SYSTEM})
message("OS name: " ${CMAKE_SYSTEM_NAME})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(LINUX 1)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set(WINDOWS 1)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "AIX")
  set(AIX 1)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
#set(CMAKE_YACC_COMPILER bison)
#set(CMAKE_LEX_COMPILER flex)

set(CMAKE_RELEASE_POSTFIX _r)
#set(CMAKE_DEBUG_POSTFIX _d)
set(CMAKE_RELWITHDEBINFO_POSTFIX _rd)

# how to add dependency to the customer command, such as when using customer yacc or lex command
# answer: can use MAIN_DEPENDENCY or DEPENDS property to do the job
# add_custom_command(
#    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ca.tab.h ${CMAKE_CURRENT_BINARY_DIR}/ca.tab.c ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c
#    COMMAND bison -d ${CMAKE_CURRENT_SOURCE_DIR}/ca.y
#    COMMAND flex ${CMAKE_CURRENT_SOURCE_DIR}/ca.l
#    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ca.y ${CMAKE_CURRENT_SOURCE_DIR}/ca.l
#    )
# add_dependency()

find_package(FLEX)
FLEX_TARGET(ca_lexier ca.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)

# using following instead of the above, when using regular bison
find_package(BISON)
BISON_TARGET(ca_parser ca.y ${CMAKE_CURRENT_BINARY_DIR}/ca.tab.c
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/ca.tab.h)

ADD_FLEX_BISON_DEPENDENCY(ca_lexier ca_parser)

#add_executable(llvm-config IMPORTED)
# get CFLAGS using llvm-config
execute_process(COMMAND llvm-config "--cxxflags"
  OUTPUT_VARIABLE CLANGPP_CXXFLAGS)

# remove the last \n character of llvm-config command
string(REPLACE "\n" " " CLANGPP_CXXFLAGS ${CLANGPP_CXXFLAGS})
string(REPLACE " " ";" CLANGPP_CXXFLAGS ${CLANGPP_CXXFLAGS})

# get LDFLAGS using llvm-config
execute_process(COMMAND llvm-config --ldflags --system-libs --libs core orcjit native
  OUTPUT_VARIABLE CLANGPP_LDFLAGS)

# remove the last \n character of llvm-config command
string(REPLACE "\n" " " CLANGPP_LDFLAGS ${CLANGPP_LDFLAGS})
string(REPLACE " " ";" CLANGPP_LDFLAGS ${CLANGPP_LDFLAGS})

# get llvm version
execute_process(COMMAND llvm-config --version OUTPUT_VARIABLE LLVM_VERSION)
string(REPLACE "\n" "" LLVM_VERSION ${LLVM_VERSION})
string(SUBSTRING ${LLVM_VERSION} 0 2 LLVM_VERSION_MAJOR)
set(llvm_version_flag -DLLVM_VERSION=${LLVM_VERSION_MAJOR})
message("llvm2: " ${llvm_version_flag})

# four standard configurations: Debug Release RelWithDebInfo MinSizeRel
#set(CMAKE_CXX_FLAGS "")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

#set(CMAKE_EXE_LINKER_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-g")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-O2")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "-O2 -g")

#set(llvm_cxxflags -I/usr/lib/llvm/12/include -std=c++14    -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS)
#set(llvm_ldflags -L/usr/lib/llvm/12/lib64  -lLLVM-12)
set(llvm_cxxflags ${CLANGPP_CXXFLAGS} ${llvm_version_flag})
set(llvm_ldflags ${CLANGPP_LDFLAGS})

add_subdirectory(llvm)

# handle special target for IR_generator.cpp
add_library(irgen OBJECT llvm/IR_generator.cpp type_system.cpp type_system_llvm.h ca.tab.h)
target_compile_options(irgen PRIVATE ${llvm_cxxflags})
target_include_directories(irgen PRIVATE . ${CMAKE_CURRENT_BINARY_DIR})

#add_dependencies(irgen ca.tab.h)

#  ca.h config.h dotgraph.h symtable.h utils.h llvm/IR_generator.h ca.l ca.y
add_executable(ca ca.y ca.l ca.tab.c lex.yy.c ca_parser.c dotgraph.cpp symtable_cpp.cpp utils.c strutil.c $<TARGET_OBJECTS:irgen>)
target_link_options(ca PRIVATE ${llvm_ldflags})

# default to debug version without an postfix, when need it just uncomment following one
#set_target_properties(ca PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
set_target_properties(ca PROPERTIES RELEASE_POSTFIX ${CMAKE_RELEASE_POSTFIX})
set_target_properties(ca PROPERTIES RELWITHDEBINFO_POSTFIX ${CMAKE_RELWITHDEBINFO_POSTFIX})

target_include_directories(ca PUBLIC . llvm ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(ca PUBLIC ir1)


install(TARGETS ca DESTINATION bin)
install(FILES README.md LICENSE DESTINATION include)

string(TOUPPER ${CMAKE_BUILD_TYPE} current_build_type)
string(COMPARE EQUAL ${current_build_type} "DEBUG" current_build_type_debug)
string(COMPARE EQUAL ${current_build_type} "RELEASE" current_build_type_release)
if (${current_build_type_debug} EQUAL true)
#  set(current_target_name ${}
endif()

add_custom_command(TARGET ca POST_BUILD COMMAND cp ca${OUTPUT_NAME_RELEASE} ${CMAKE_SOURCE_DIR})

# testing work
# enable testing work
enable_testing()

#include("${CMAKE_SOURCE_DIR}/cmake/tests.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/functions.cmake")

# packages
include("${CMAKE_SOURCE_DIR}/cmake/packages.cmake")

add_subdirectory(test)

