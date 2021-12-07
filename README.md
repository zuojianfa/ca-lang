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
- [x] add type convertion `as` keyword
- [ ] support multiple compile unit and link them together
- [ ] support rust grammar
  - [ ] loop
  - [ ] match
  - [ ] type
  - [x] last expression as return value in scope
  - [ ] immutable
  - [ ] mutable
  - [ ] support attribute grammar like `#[derive(Clone)]`
- [ ] support never return type `!` like rust
- [x] support invoke extern c function
  now directly support the extern c function, the `libc` is automatically added into the linker, so all the `libc` function can be used directly 
- [x] support c variant parameter function
- [x] support comment
- [ ] support `0xxxxxxxx` literal
- [x] when `-main` provided the global variable should use `#[scope(global)]` grammar to specifiy if it is a global variable
  - [x] try default global variable as local variable in generated `main` function
  - [x] add `#[scope(global)]` grammar

  after adding global grammars the semantic will be:
  - `#[scope(global)]` can only used to describe the declare variables in non-function
  - when compiling with `-main`
	- the variable declare in non-function will be local variable in generated main function
	- use `#[scope(global)]` ahead of global variable declare will be the global variable
  - when no `-main` option
	- the variable declare in non-function will be global variable
	- `#[scope(global)]` have the same effect as with `-main` option, so it's no affect to non-function variables
- [ ] how to cope with: when use `-main` but the source file already defined a main function?
  detect the conflicting

## Makefile
- [ ] use cmake to build the system
- [ ] make test .ca file automatically generating not write one by one

## Optimization
- [ ] support optimization for each function not only `main`
- [ ] support global optimization
- [ ] add optimization for all functions, in `do_optimize_pass` function

## System
- [x] jit (just in time running) support
- [x] compile into executable file (linux)
- [x] compile into ll (llvm assembly text file)
- [ ] compile into llvm assembly binary file
- [ ] Passing ld options for -native command
- [ ] Write interactive interpreter like python command line
- [x] Add option `-main` to emit main function, default not generate main function
- [ ] support multi module compile, reference the functions / global variables in other ca module
  it should be the same as C language
- [ ] support smoothly invoke c libraries in order to make use of all kinds of C libraries
  - [ ] write a tool to converting c function declare into ca's declare
  - [ ] write a tool to convert c header file declear into ca's declare file
- [ ] create self memory management runtime system, can invoke malloc directly
  
## Testing
- [ ] convert (by hand) a real whole program project into ca language project
  - [ ] simple one like find grep
  - [ ] complex pure c redis

## self-defined machine
- [ ] add interpret machine
- [ ] add instruction
- [ ] add code generation

## Misc
- [x] llvm require only one `ret` statement in one function, so when encounter multiple `return` statement in ca language, then need firstly alloca a variable and in the end function add a return label (return BasicBlock) and for each return statement, set the return value into the local variable and branch into the return label finally return the variable

is scopeline the real skip function start for debugging? try it

NEXT TODO: 
- [ ] remove entry object in ASTNode
- [ ] handle `check_fn_proto` check if it can check some item when post type definitions
- [ ] handle pointer struct type
- [ ] make typeid_t opaque for making it cannot convert from int to typeid_t directly
- [ ] handle logical operation type convert, in function `IR1::gen_sub` ..., and case '<', etc
- [ ] support other atomic type
- [ ] support point type
- [ ] support array type
- [ ] add graphviz (dot graph) option for outputing the grammar tree
- [ ] debug support inner field scope, 

detailed TODO LIST:

```
*0	        make_expr			        ARG_LISTS_ACTUAL
*al->argc 	make_expr_arglists_actual	ARG_LISTS_ACTUAL
*1 	 	    make_goto 			        GOTO
*2 	 	    make_assign 			    '='
*2 	 	    make_vardef 			    '='
*1 	 	    make_stmt_ret 			    RET
*1 	 	    make_stmt_ret_expr 		    RET
*1 	 	    make_stmt_print 		    PRINT
*listlen  	make_stmt_list_zip 		    stmt_list ';'
2	 	    make_expr  			        ';'

* make_id separate into different type: VarDef, VarAssign, FnName, Var (make_ident_expr), 
* handle case TTE_Id
make stmt type and move upper into TTE_Stmt type

*`as` is a expression 

```

# License
See `LICENSE` file in this directory, for license in directory `cruntime` see cruntime/README.md


# searches
```
reduce_node_and_type
```

# type tree
## types
```
i8 i32 f32 f64 bool ...
*i8 *i32 *f32 *f64 *bool ...
**i8 **i32 **f32 **f64 **bool ...
[i8;3] [i32;4] [f32;7] [f64;1] [bool;9] ...
[[i8;3];4] [[i32;4];3] [[f32;7];6] [[f64;1];3] [[bool;9];3] ...
*[i8;3]
[*i8;3] [*[i8;3];4] ...
[[i8;3];4] *[[i8;3];4] **[[i8;3];4] ***[[i8;3];4] [*[i8;3];4] [**[i8;3];4] *[*[i8;3];4] *[**[i8;3];4] ...
struct AA { a: i8, b: i32 } *AA [AA;3] *[AA;3] [*AA;3]
struct BB { a: AA, b: *AA }

T
*T [T;3] struct TT {a: T}
**T [*T;3] struct TT {a: *T}  *[T;3] [[T;3];3] struct TT {a: [T;3]}  *TT [TT;3] struct TT2 {a: TT}
...
```

## struct type representation
```

```

## entry representation
```
CADataType *resolve(typeid_t datatype) {
	CADataType *dt = catype_get_primitive_by_name(datatype);
	if (dt)
		return dt;
	
	dt = sym_getsym(symtable, datatype);
	if (dt->type != Sym_DataType) {
		error;
	}
	
	if (dt->u.datatype.is_non_primitive_terminal)
		resolve_non_primitive_terminal(); // like structure definition or future definition like enum tuple etc,
	else
		resolve(dt->u.datatype.datatype);
}


```

## recursive definition
```
type AA = BB;
type BB = AA;

type AA = AA;

type AA = *AA;

type AA = *BB;
type BB = AA;

type AA = *BB;
type BB = *AA;
```

```
struct AA {
	a: AA
}
```

```
struct AA {
	a: BB
}

struct BB {
	b: AA
}
```

## non-recursive definition
```
struct AA {
	a: *AA
}
```

```
struct AA {
	a: *BB
}

struct BB {
	b: *AA
}

```

## others
### how to identify named type and unnamed type?
```
let a: A;
type A = [*i32;3];

let b: [*i32;3];
```
```
a <= t:A
b <= t:[*i32;3]
A <= [*i32;3]
```
