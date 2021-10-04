# the llvm-ir ca compiler with no defined function support
This branch support compile source code into llvm ir aims: include ll file, native object file (.o), native assembly file (.s) and jit running with gcc debug support and optimization.

This branch support functions.

The llvm-ir or rename it into ca contains the llvm code generation, can compiled into many objectives: llvm IR, native `as` assembly language (.s), native object file (.o elf64) and can run with jit with or without optimization.

## Usage
```
./llvm-ir
Usage: ca [-ll] | [-S] | [-native] | [-c] | [-jit] [-O] | [-g] <input> [<output>]
Options:
         -ll:      compile into IR assembly file: .ll (llvm)
         -S:       compile into native (as) assembly file: .s
         -native:  compile into native execute file: ELF file on linux, PE file on windows (default value)
         -c:       compile into native object file: .o
         -jit:     interpret using jit (llvm)
         -O:       do optimization
         -g:       do not do any optimization (default value)
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

## debug & optimization
### -g
Add debug information into the object file, and can use gdb to debug the code

### -O
Add optimization pass for the llvm (optimization) pass

### -g -O
Add debug information into the object file and add llvm optimization pass 

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

# TODO list

## Debugging

- [x] add debugging support for gdb
- [x] add function debug support
- [x] add debug variable (global) type information for viewing variable value in gdb
- [x] add debug variable (local) type information for viewing variable value in gdb
- [x] make non-global variable visiable when debugging, now can only debug but cannot see the variable name in the debugger
- [x] resolve inner variable problem for debugging
- [ ] skip the function name for debugging just like c function
- [ ] generate debug info for label DILabel (DIBuilder::createLabel)
- [ ] support fine location (lineno rowno) info for each symbol
- [x] record program begin location
- [x] record program end location
- [x] record main function begin location
- [x] record main function end location
- [x] handle the main function begin position

## Grammar
- [x] add function functionality

- [ ] support array

- [ ] support layered variable definition

- [ ] add other atomic type

- [ ] add record (struct) type

- [ ] add pointer type

- [ ] support multiple compile unit and link them together

- [ ] support rust grammar
  - [ ] loop
  - [ ] match
  - [ ] type
  - [x] last expression as return value in scope
  - [ ] immutable
  - [ ] mutable
  
- [x] support invoke extern c function

  now directly support the extern c function, the `libc` is automatically added into the linker, so all the `libc` function can be used directly 

- [x] support c variant parameter function

- [x] support comment

## Makefile
- [ ] use cmake to build the system
- [ ] make test .ca file automatically generating not write one by one
## Optimization
- [ ] support optimization for each function not only `main`
- [ ] support global optimization
## System
- [x] jit (just in time running) support
- [x] compile into executable file (linux)
- [x] compile into ll (llvm assembly text file)
- [ ] compile into llvm assembly binary file
- [ ] Passing ld options for -native command
- [ ] Write interactive interpreter like python command line
- [ ] Add option `-nomain` to not emit main function
## self-defined machine
- [ ] add interpret machine
- [ ] add instruction
- [ ] add code generation



is scopeline the real skip function start for debugging? try it

NEXT TODO: 

- [ ] support other atomic type
- [ ] support point type
- [ ] support array type
- [ ] debug support inner field scope, 


# License
See `LICENSE` file in this directory, for license in directory `cruntime` see cruntime/README.md

