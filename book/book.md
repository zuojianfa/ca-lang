# The CA Programming Language
*By Rusheng Xia (xrsh_2004@163.com)*

CA is a new language which uses rust-like grammar, but internally it just like C language in many aspects and compatible with C ABI, it support memory garbage collection which is powered by gc library which does not use reference count. CA language support a lot of flexible but powerful grammars which make it easy to program.
CA compiler is powered by LLVM. It support JIT functionality, you can run ca program directly with ca compiler without compile it. Of course you can compile it into native executable program.

Currently, Although CA language is still under development but it almost has all the features a general language should have.

# 1 Getting Started
## 1.1 Installation

Currently, you can only build from source code to get the compiler binary.

### Environment

When using rpm package based Linux system like fedora install following packages
- gcc
- cmake
- llvm13 llvm13-devel
- flex
- bison
- gc-devel

### Compile from source code

```
$ cd ca
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
$ make -j3
$ cd ..
$ ./ca
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



## 1.2 Hello World!
This is the simplest way. It uses the `print` statement, it can print any variable.

```rust
// hello1.ca
fn main() {
	print "Hello World!";
}
```
Or you can use `C` function, all the `C` functions in default runtime library can be used in `CA`.

```rust
// hello2.ca
extern fn printf(f: *char, ...) -> i32;

fn main() {
    printf("Hello %s\n", "World!");
}
```

### Compile and run

You can compile it into an executable.

```bash
$ ca -native hello1.ca hello1
$ ./hello1
Hello World!
```

### Run directly with JIT

Or, you can run the code directly with `ca` with it's JIT feature.

```bash
$ ca hello1.ca
program `hello1.ca` :
Hello World!

return value: 1035898880
```

The return value seems to  be a random value. It's because the `main` function by default returns an integer value, but in the definition of `main` function it returns nothing, so here get an random value. You can let `main` function to return a value.

```rust
fn main() -> i32 {
	print "Hello World!\n";
	return 1;
}
```

Notice that the definition of function `main` now returns a value of type `i32`. Run the program with a return value.

```
$ ca hello3.ca
program `hello3.ca` :
Hello World!

return value: 1
```

# 2 Programming a Guessing Game

The traditional number guess Game.

```rust
type time_t = i64;
extern fn rand() -> i32;
extern fn srand(seed: u32);
extern fn time(tloc: *time_t) -> time_t;
extern fn printf(f: *char, ...) -> i32;
extern fn scanf(format: * char, ...) -> i32;

fn main() -> i32 {
    print "Guess the number!\n";
    srand(time(0) as u32);
    let secret_number = rand() % 100;

    loop {
        print "Please input your guess:\n";
        let guess = 0;
        scanf("%d", &guess);

        printf("You guessed: %d\n", guess);
        if (guess < secret_number) {
            print "Too small!\n";
		} else if (guess > secret_number) {
            print "Too big!\n";
		} else {
            print "You win!\n";
            break;
        }
    }

    return 0;
}
```

Run

```
$ ca guess.ca 
program `guess.ca` :
Guess the number!
Please input your guess:
3
You guessed: 3
Too small!
Please input your guess:
66
You guessed: 66
Too big!
Please input your guess:
44
You guessed: 44
Too small!
Please input your guess:
55
You guessed: 55
Too small!
Please input your guess:
60
You guessed: 60
Too big!
Please input your guess:
57
You guessed: 57
Too big!
Please input your guess:
56
You guessed: 56
You win!

return value: 0
```

# 3 Common Programming Concept

## 3.1 Variable Definition

The variable definition just like `rust`, it uses `let` to bind the value into a variable, the variable type can be inference by the value. The variable type can also specify it's type. If the type of binding value is not identical with the type specified in variable definition, will report an error.

```rust
let num1 = 100;
let num2 = 100i32;
let num3 = 100i64;
let num4 = 100.1f32;
let num5 = 100.1f64;
let num6: u32 = 100;
let num7: u64 = 100;
let s1 = "Hello World";
```

## 3.2 Primitive Data Types

Following Primitive Types are support in `CA`.

| Type   | Size (Byte) | Comments                                |
| ------ | ----------- | --------------------------------------- |
| i8     | 1           | 1 byte signed integer                   |
| i16    | 2           | 2 byte signed integer                   |
| i32    | 4           | 4 byte signed integer                   |
| i64    | 8           | 8 byte signed integer                   |
| isize  | 8           | 8 byte signed size type, equal to i64   |
| u8     | 1           | 1 byte unsigned integer                 |
| u16    | 2           | 2 byte unsigned integer                 |
| u32    | 4           | 4 byte unsigned integer                 |
| u64    | 8           | 8 byte unsigned integer                 |
| usize  | 8           | 8 byte unsigned size type, equal to u64 |
| f32    | 4           | 4 bit float type                        |
| f64    | 8           | 8 bit float type                        |
| bool   | 1           | boolean type                            |
| char   | 1           | character type, like C                  |
| uchar  | 1           | unsigned type                           |
| short  | 2           | same as i16                             |
| ushort | 2           | same as u16                             |
| int    | 4           | same as i32                             |
| uint   | 4           | same as u32                             |
| float  | 4           | same as f32                             |
| double | 8           | same as f64                             |
| void   | no size     | empty type                              |

## 3.3 Functions

## 3.4 Comments

Now only support comment that begin with `//`, In the future will support `/* ... */` comment

## 3.5 Control Flow

### 3.5.1 if ... else ...

### 3.5.2 loop { ... }

### 3.5.3 while (...) { ... }

### 3.5.4 for

### 3.5.5 break

### 3.5.6 continue

### 3.5.7 goto

# 4 Array & Slice

## 4.1 Array

## 4.2 Array Slice

## 4.3 Copy by Value

# 5 Using Structs to Structure Related Data

## 5.1. Defining and Instantiating Structs

## 5.2 An Example Program Using Structs

## 5.3 Method Syntax

# Traits

# Pointer

# Object Oriented Programming

# Patterns and Matching



# Generic Programming

# Garbage Collection

## Not use reference count



# Grammar

## Data type

## Variable Binding

## Function

## Primitive Type

## Comment

## if statement

## loop

## for loop

## while loop

## Struct

## Match

## Pattern

## Method

## String

## Traits

## Function call

## type aliases

## type conversion

## Pointer



