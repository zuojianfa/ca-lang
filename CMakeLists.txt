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

add_subdirectory(src)

# testing work
# enable testing work
enable_testing()

#include("${CMAKE_SOURCE_DIR}/cmake/tests.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/functions.cmake")

# packages
include("${CMAKE_SOURCE_DIR}/cmake/packages.cmake")

add_subdirectory(test)

