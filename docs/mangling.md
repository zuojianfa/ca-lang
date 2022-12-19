# mangling & demangling
when using mod or struct method it need mangling for the name of function, or may be for the mod itself

- [x] for rust when using #[no_mangle] for function, it will ignore the mod name, and in different mod define the function with the same name is not allowed

# mangle
```rust
mod aaa {
mod bbb {
struct SAA {
}

impl SAA {
fn new() -> SAA {
}

fn adding(self, a1: i32, a2: i32) -> i32 {
	a1 + a2
}
}
}

impl SAA for Adding {
fn adding(self, a1: i32, a2: i32) -> i32 {
	a1 + a2 + 1
}
}
}

trait Adding {
fn adding(self, a1: i32, a2: i32) -> i32;
}
```

## module mangling
```
_
-
.
$
a-zA-Z0-9
```
using `$` as the seperator of the type
## function mangling
prefix for type
S => struct
F => function
M => module
T => trait # T must follow by S
TS => trait + struct
L => lexical unit

### Baring function
`fn foo() {}` => `foo`

### With module name
`mod bar { fn foo(){} }` => `$MF3bar3foo`
`mod bar { fn afoo(){} }` => `$MF3bar3afoo`
`mod baz { fn foo(){} }` => `$MF3baz3foo`
`mod bar { mod baz{ fn dda(){} } }` => `$MMF3bar3baz3dda`

### With struct name
`struct AAA {} impl AAA { fn foo(){} }` => `$SF3AAA3foo`
`mod fozz { struct AAA {} impl AAA { fn foo(){} } }` => `$MSF3fozz3AAA3foo`

### With trait
```
struct AAA {}
trait Tttt { fn fooo(); }
```
`impl AAA { fn fooo(){} }` => `$SF3AAA4fooo`
`impl Tttt for AAA { fn fooo(){} }` => `$TSF4Tttt3AAA4fooo`
`mod aagc { impl Tttt for AAA { fn fooo(){} } }` => `$MTSF4aagc4Tttt3AAA4fooo`

### Function in struct in function
```
mod ccc {
fn foo() { 
	struct AAA {}
	impl AAA { fn bar(){} }
}

struct AAA {}
impl AAA {
	fn foo() {
		struct AAA {}
		impl AAA { fn bar(){} }
	} 
}
}
```
=> `$MFSF3ccc3foo3AAA3bar`
=> `$MSFSF3ccc3AAA3foo3AAA3bar`

### Function in function
`fn foo() { fn bar() {} }` => `$FF3foo3bar`
`mod mm { fn foo() { fn bar() {} } }` => `$MFF2mm3foo3bar`
`mod tt { mod mm { fn foo() { fn bar() {} } } }` => `$MMFF2tt2mm3foo3bar`

`struct AAA {} impl AAA { fn foo(){ fn mare() {} } }` => `$SFF3AAA3foo4mare`
`mod fozz { struct AAA {} impl AAA { fn foo(){ fn mare() {} } } }` => `$MSFF3fozz3AAA3foo4mare`

`mod ccc{struct AAA {} impl AAA{fn foo(){struct AAA {} impl AAA{fn bar(){fn aada(){}}}}}}` => `$MSFSFF3ccc3AAA3foo3AAA3bar4aada`

### Lexical block
```
fn main() { // scope 1
	if (true) { // scope 2
		fn inner_func1() { // scope 3
			if (false) { // scope 4
				{ // scope 5
				}
				fn inner_func1_0() { // scope 6
					{ // scope 7
						fn inner_func1_0_0() { // scope 8
						}
					}
				}
			} else { // scope 9
				fn inner_func1_1() { // scope 10
				}
			}
		}
	} else { // scope 11
		{ // scope 12
			fn inner_func2() { // scope 13
			}
		}
	}
}
```
`inner_func1` => `$FLF4main3L2S11inner_func1` encode method 1
`inner_func1` => `$FL2F4main11inner_func1` encode method 2, currently choose encode method 2, method 1 have longer length
`inner_func2` => `$FL11L12F4main11inner_func2`
`inner_func1_0` => `$FL2FL4F4main11inner_func113inner_func1_0`
`inner_func1_0_0` => `$FL2FL4FL7F4main11inner_func113inner_func1_015inner_func1_0_0`
`inner_func1_1` => `$FL2FL9F4main11inner_func113inner_func1_1`

## struct / trait mangling
struct and trait name just use what it is, because they not exist in assembly language

# lexcial callstack for function
```
#0  walk_lexical_body (node=0x5555556ae910) at /home/rxia/git/compiler/ca/src/llvm/IR_generator.cpp:3898
#1  0x00005555555a8a80 in walk_stack (p=0x5555556ae910) at /home/rxia/git/compiler/ca/src/llvm/IR_generator.cpp:3952
#2  0x00005555555a8407 in walk_fn_define_full (p=0x5555556a7ed0, impl_info=0x0) at /home/rxia/git/compiler/ca/src/llvm/IR_generator.cpp:3799
#3  0x00005555555a866e in walk_fn_define (p=0x5555556a7ed0) at /home/rxia/git/compiler/ca/src/llvm/IR_generator.cpp:3839
#4  0x00005555555a8a80 in walk_stack (p=0x5555556a7ed0) at /home/rxia/git/compiler/ca/src/llvm/IR_generator.cpp:3952
#5  0x00005555555aa11b in walk (tree=0x555555694510) at /home/rxia/git/compiler/ca/src/llvm/IR_generator.cpp:4270
#6  0x0000555555561a60 in main (argc=2, argv=0x7fffffffdb68) at /home/rxia/git/compiler/ca/src/ca.cpp:162
```

# How to 
## find the function definition path
Using `lexical_scope_stack` remember the path of lexical scope `LexicalScope` in any place, and the function definition place can use it to find it's path, by using the path it can form the unique function name, each scope object have a sequence id (lexical_id) globally of the unique, and the id number can be used in function name

## find function current scope visible
Just using normal way like finding an variable name, not only from the global scope, with the function name (not mangled name, the mangled name is only used in generated function name, and function call generation)

## find method of an struct object or trait object
firstly find the object definition and then find method name through the object definition

## How to find struct definitions path

## struct implementation also need a scope????????

