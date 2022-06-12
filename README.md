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
- [x] generate debug info for label DILabel (DIBuilder::createLabel)
- [x] record program begin location
- [x] record program end location
- [x] record main function begin location
- [x] record main function end location
- [x] handle the main function begin position
- [ ] skip the function name for debugging just like c function
- [ ] support fine location (lineno rowno) info for each symbol

## Grammar
- [x] support `__zero_init__` value for initial the value of any type with 0 initialized
- [x] add function functionality
- [x] support array
- [x] support layered variable definition
- [x] add record (struct) type
- [x] support pointer type
- [x] support array type
- [x] support addressing operator '&'
- [x] add pointer type
- [x] support store variable value in array not just literal
- [x] support array literal
- [x] add type convertion `as` keyword
- [x] implement deref_left operation
- [x] implement array to pointer as (may already implemented)
- [x] implement pointer add sub operation
- [x] support invoke extern c function
  now directly support the extern c function, the `libc` is automatically added into the linker, so all the `libc` function can be used directly 
- [x] support c variant parameter function
- [x] support comment
- [x] support `0xxxxxxxx` literal
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
- [x] how to cope with: when use `-main` but the source file already defined a main function?
  detect the conflicting
  
- [x] support post type definition
- [x] support multiple scope with bracket '{}'
- [x] support raw string literal
- [x] support multiple line string literal
- [x] support pointer cast from integer and any other pointer
- [x] support array literal
- [x] support array entire left value assignment for any part of array item at least for primitive type
- [x] support array element operation
- [x] struct with array element
- [x] support struct member operation
- [x] array of struct element
- [x] function with array parameter
- [x] function with pointer parameter
- [x] function with struct parameter
- [x] support struct literal
- [x] support named struct literal
- [x] support all kinds of type's debuggging info
- [x] support scoped debugging information
- [x] add other atomic type
- [x] '+=' '-=' '*=' '/=' '%=' '<<=' '>>=' '&=' '|=' '^='
- [x] bit operation '<<' '>>' '|' '&' '^' '~'
- [x] logical operation '&&' '||' '!'
- [x] break
- [x] continue
- [x] support octal literal: 0o777 -0o1000
- [x] support hex literal: 0xffff -0xfe23
- [x] support decimal literal
- [x] support binary literal value: 0b010100101000100101 -0b1110101001
- [x] support tuple struct type
- [x] add tuple struct literal
- [x] the named tuple and function cannot have the same name in the same scope, they are conflicting not like struct
- [x] if else if else ...
- [x] support multiple if-else else if statement
- [x] well error messages of line number
- [ ] support multiple compile unit and link them together
- [ ] support rust grammar
  - [x] heap allocate memory: `box`
  - [x] named tuple
  - [x] unnamed tuple (general tuple)
  - [x] tuple operation .0 .1
  - [x] loop
  - [x] type statement, used to define type aliases
  - [x] let pattern matching: let S(x) = A; let S{x} = A;
  - [x] support let variable rebind to new type
  - [x] for i in list
	- [x] for *i in list
	- [ ] for &i in list
  - [ ] reference type
  - [ ] match
  - [ ] support enum type
  - [ ] last expression as return value in scope
	- [ ] break expr
	- [ ] break 'label2 expr
	- [ ] continue expr
	- [ ] continue 'label2 expr
  - [ ] all kinds of expression
  - [ ] immutable
  - [ ] mutable
  - [ ] support attribute grammar like `#[derive(Clone)]`
  - [ ] scope variable release
  - [ ] range .. operator: a..b, a..=b, ... can be used in array declare, e.g. [..], [a..b], [a..=b], [a..], [..b], [..=b], or in struct declare: S { ..s }
  - [ ] 'label1: loop # labeled loop
  - [ ] break 'label1 # labeled break
  - [ ] mod scope
  - [ ] use package
  - [ ] destructure operator: let S(x) = A; let S{x} = A;
  - [ ] default acceptor _
  - [ ] slice operator and infrastructure
  - [ ] function definition can be in any scope
  - [ ] support never return type `!` like rust



## Makefile
- [x] use cmake to build the system
- [x] make test .ca file automatically generating not write one by one
	can use ctest framework, but each .ca test case need write by hand

## Optimization
- [x] support optimization for each function not only `main`
- [ ] support global optimization
- [ ] add optimization for all functions, in `do_optimize_pass` function

## System
- [x] jit (just in time running) support
- [x] compile into executable file (linux)
- [x] compile into ll (llvm assembly text file)
- [x] Passing ld options for -native command
- [x] Add option `-main` to emit main function, default not generate main function
- [ ] compile into llvm assembly binary file
- [ ] Write interactive interpreter like python command line
- [ ] support multi module compile, reference the functions / global variables in other ca module
  it should be the same as C language
- [ ] support smoothly invoke c libraries in order to make use of all kinds of C libraries
  - [ ] write a tool to converting c function declare into ca's declare
  - [ ] write a tool to convert c header file declear into ca's declare file
- [ ] create self memory management runtime system, can invoke malloc directly
- [ ] define system limitations: such as max identifier name, max function parameters, ...
  
## Testing
- [ ] transplant c test suite 
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

123 - type49-struct_use2.ca (Failed) because of stack is too small, resolve with: ulimit -s 102400

NEXT TODO:
- [ ] range .. operator: a..b, a..=b, ... can be used in array declare, e.g. [..], [a..b], [a..=b], [a..], [..b], [..=b], or in struct declare: S { ..s }
- [ ] 'label1: loop
- [ ] match
- [ ] mod scope
- [ ] use package
- [ ] default acceptor _
- [ ] slice operator and infrastructure
- [ ] implement following functions: 
- [ ] for &i in list
- [ ] reference type
- [ ] resolve for stmt debug issue: cannot skip to for after one loop
- [ ] implement following function
  `catype_compare_type_signature`, 
  `catype_make_type_closure`,
  `catype_create_type_from_unwind`
- [ ] remove entry object in ASTNode
- [ ] make typeid_t opaque for making it cannot convert from int to typeid_t directly
- [ ] set right line number for structure definition lines
- [ ] refactor factor where to find CADataType object using quickest way **to distinguish which is unwinded typeid which winded typeid**
- [ ] add graphviz (dot graph) option for outputing the grammar tree
- [ ] function definition can be in any scope

# License
See `LICENSE` file in this directory, for license in directory `cruntime` see cruntime/README.md

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

## struct type inner representation
```
struct AA {} => t:{AA}
struct A2 {a:i32,b:i64,c:u32,d:f32,e:f64,f:bool,g:i8,h:u8} => t:{A2;a:i32,b:i64,c:u32,d:f32,e:f64,f:bool,g:i8,h:u8}
struct A3 {a: AA, b: A1, c: A2} => t:{A3;a:{AA},b:{A1},c:{A2;a:i32,b:i64,c:u32,d:f32,e:f64,f:bool,g:i8,h:u8}}
? => t:*{A5;a:{A4;aa:i32,b:*i32,c:**i32,d:***i32,e:*{AA},f:**{A1},g:*{A3;a:{AA},b:{A1},c:{A2;a:i32,b:i64,c:u32,d:f32,e:f64,f:bool,g:i8,h:u8}},h:**{A2;a:i32,b:i64,c:u32,d:f32,e:f64,f:bool,g:i8,h:u8}},b:*{A4;aa:i32,b:*i32,c:**i32,d:***i32,e:*{AA},f:**{A1},g:*{A3;a:{AA},b:{A1},c:{A2;a:i32,b:i64,c:u32,d:f32,e:f64,f:bool,g:i8,h:u8}},h:**{A2;a:i32,b:i64,c:u32,d:f32,e:f64,f:bool,g:i8,h:u8}},c:**{A4;aa:i32,b:*i32,c:**i32,d:***i32,e:*{AA},f:**{A1},g:*{A3;a:{AA},b:{A1},c:{A2;a:i32,b:i64,c:u32,d:f32,e:f64,f:bool,g:i8,h:u8}},h:**{A2;a:i32,b:i64,c:u32,d:f32,e:f64,f:bool,g:i8,h:u8}},d:***{A4;aa:i32,b:*i32,c:**i32,d:***i32,e:*{AA},f:**{A1},g:*{A3;a:{AA},b:{A1},c:{A2;a:i32,b:i64,c:u32,d:f32,e:f64,f:bool,g:i8,h:u8}},h:**{A2;a:i32,b:i64,c:u32,d:f32,e:f64,f:bool,g:i8,h:u8}},e:****{A4;aa:i32,b:*i32,c:**i32,d:***i32,e:*{AA},f:**{A1},g:*{A3;a:{AA},b:{A1},c:{A2;a:i32,b:i64,c:u32,d:f32,e:f64,f:bool,g:i8,h:u8}},h:**{A2;a:i32,b:i64,c:u32,d:f32,e:f64,f:bool,g:i8,h:u8}}}
```

## tuple type inner representation
```
struct AA () => t:(AA)
struct A1 (i32) => t:(A1;i32)
struct A2 (i32, bool) => t:(A2;i32,bool)
struct A3 (i32, AA, A1, A2) => t:(A3; i32, (AA), (A1;i32), (A2; i32, bool))
```

## array type inner representation
```
t:[[[[[[i32;4];3];5];5];3];5]
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

# another representation of type
using `symname_id` instead of the string representation of name. keep the special element unchanged, such as * [] {} ... 
this should can speed up the find performance 
```
*i32 => *i32id, i32id == symname_check_insert("t:i32")
*nameA => *nameAid, nameAid == symname_check_insert("t:nameA")
[i32;3] => [i32id;3]
struct A {a: i32, b: *A} => {Aid; aid:i32id, bid:*Aid}, Aid == symname_check_insert("t:Aid")
```

