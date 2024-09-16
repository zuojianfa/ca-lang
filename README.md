# CA compiler
*By Rusheng Xia (xrsh_2004@163.com)*

CA is a programming language which has similar grammars as rust language, similar to C language in many features, compatible with C ABI, support calling C functions directly without any matching and transformation. It uses Mark-and-Sweep style garbage collections for it's memory management.

CA compiler uses LLVM as its backend, and uses JIT or AOT technology in LLVM to run program directly without compile.

Currently, CA compiler is under development, it has already realized most of grammars, but still have a lot of works to do. 

For language details see the book: [CA book](book/ca-book.md).

## Features
- CA is static type language
- Support compile CA source file into llvm ir representation
- Support JIT functionality that run from source file directly
- Support compile CA source file into native executable file: ELF file on linux
- Support compile CA source file into readable IR assembly file `.ll`
- Support compile CA source file into native (as) assembly file: `.s`
- Support debug with gdb debugger
- The object file uses C ABI
- Internal Suppport C library functions
- Support garbage collection feature using gc
- Support generate default main function when source code not defined
- Support generate `.dot` file to show the graph of grammar tree
- Support llvm12 or llvm13 library
- It include the whole test cases in the source code


## Supportted Grammers
- datatype can be defined in any scope
- Support pointer like in C language
- Support multi-dimension array and their initialization
- Support array convert to pointer
- Support char array c onvert into string type
- Support inner debug `print` statement, which can print any value
- Support `typeof` statement to get the type of one variable dynamically, and then use the type to define another variable
- Not like rust the variable is mutable by default (and only can be mutable)
- Support binary number literal
- 

This branch support functions.

The llvm-ir or rename it into ca contains the llvm code generation, can compiled into many objectives: llvm IR, native `as` assembly language (.s), native object file (.o elf64) and can run with jit with or without optimization.

## Environment Preparing
When using rpm package based linux system like fedora install following packages
- llvm13 llvm13-devel
- flex
- bison
- gc-devel


## Usage
```
./ca
Usage: ca [options] <input> [<output>]
Options:
         -ll:      compile into IR assembly file: .ll (llvm)
         -S:       compile into native (as) assembly file: .s
         -native:  compile into native execute file: ELF file on linux, PE file on windows (default value)
         -c:       compile into native object file: .o
         -jit:     interpret using jit (llvm)
         -O[123]:  do optimization of level 1 2 3, default is level 2
         -g:       do not do any optimization (default value)
         -main:    do generate the default main function
         -dot <dotfile>:  generate the do not generate the default main function
```

## Compile
- make all executable
`make`

- make llvm-ir
`make llvm-ir`

- make only object file (.o) for test program (.ca) with `-ll` option and invoke `llc`
`make llvm-test-to-o`

- make executable file for test program (.ca) with `llvm-test-to-o` aim and `clang`
`make llvm-test-clang`

- make native assembly file (.s) for test program (.ca) with `-ll` option and invoke `llc`
`make llvm-test-to-s`

- make executable file for test program (.ca) with `llvm-test-to-s` aim and `as` `ld` with `c runtime` object file (`crt1.o, crti.o, crtn.o, crtbegin.o crtend.o`)
`make llvm-test-as-ld`

- make executable file for test program (.ca) with `llvm-test-to-s` aim and `as` `ld` with direct assembly entry without `c runtime` object file
`make llvm-test-as-ld2`

- make executable file for test program (.ca) with `-c` option to create object file (.o) and `ld` with `c runtime` object file (`crt1.o, crti.o, crtn.o, crtbegin.o crtend.o`)
`make llvm-test-o`

- clean code
`make clean`

## Compile with cmake
### Debug Version
```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j3
```

### Release Version
```
mkdir rbuild && cd rbuild
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j3
```

### Release Version with Debug
```
mkdir rbuild && cd rbuild
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j3
or 
cmake --build .
```

## debug & optimization
### -g
Add debug information into the object file, and can use gdb to debug the code

### -O
Add optimization pass for the llvm (optimization) pass

### -g -O
Add debug information into the object file and add llvm optimization pass 

## Install
```
cmake --install . --prefix "/home/to/dir"
```

## make package / installer
```
cpack
# or
cpack -G ZIP -C Debug
```

### make source code distribution
```
cpack --config CPackSourceConfig.cmake
# or
cpack --config CPackSourceConfig.cmake
```

## An code example
```
x = 0;
print x;
y = 0;
while (x < 10) {
	print x;
	x = x * 2 + 4 / y;
	y = y + 1;
	z = z + 1;
	i = i + 1;
	j = i + 1;
}

print y;
```

# Features
## link multiple object
```
ca -c -g test/extern_call2.ca extern_call2.o
ca -c -g -nomain test/extern_call2_assist.ca extern_call2_assist.o
clang test/extern_call2.ca test/extern_call2_assist.ca -o call2
# or
ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o call2 extern_call2.o extern_call2_assist.o -lc
```

