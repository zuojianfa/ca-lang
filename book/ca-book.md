# The CA Programming Language
*By Rusheng Xia (xrsh_2004@163.com)*

CA is a new language which has similar grammars as rust language, but internally in many aspects it is more similar to C language and compatible with C ABI - it support raw pointers, able to call C functions directly. CA language support garbage collections in memory management which is powered by the Mark-and-Sweep algorithm in GC library. CA language support a lot of flexible but powerful grammars which make it easy to program.

CA compiler uses LLVM as its backend, and uses JIT or AOT technology of LLVM. You can run CA programs directly  without compiling it. Of course it's also common to compile it into native executable program.

Currently, CA language is still under development but it already has most features of a general programming language should have. Also it offered huge of test cases in the test directory of the source code, all language features are covered in these test cases. So if you want to know all about the languages aspect the best way is reviewing the test cases.

# 1 Getting Started
## 1.1 Installation

Currently, you can only build from source code to get the compiler binary.

### 1.1.1 Environment

When using rpm package based Linux system like redhat / fedora, please install following packages
- gcc
- cmake
- llvm12 llvm12-devel (or llvm13 llvm13-devel)
- flex
- bison
- gc-devel

### 1.1.2 Compile from source code

```bash
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



## 1.2 Hello CA!
The simplest way, just use the `print` statement which is an inner statement for debugging, able to print any values of any type.

```rust
// hello1.ca
fn main() {
	print "Hello CA!";
}
```
You can also use `C` function `printf` to do the job. All the `C` functions in default runtime library can be used in CA directly.

```rust
// hello2.ca
extern fn printf(f: *char, ...) -> i32;

fn main() {
    printf("Hello %s\n", "CA!");
}
```

### 1.2.1 Compile and run

Compile it into executable program.

```bash
$ ca -native hello1.ca hello1
$ ./hello1
Hello CA!
```

### 1.2.2 Run directly with JIT

Run the code directly with JIT feature of CA.

```bash
$ ca hello1.ca
program `hello1.ca` :
Hello CA!

return value: 1035898880
```

Regarding the return value: it appears to be random because the prototype of the `main` function lacks an explicit return type. In such cases, the system defaults to assuming an integer return type. However, if the implementation of `main` does not include a return statement, the function will return a random value. To resolve this issue, ensure that the `main` function both specifies a return type and includes a return statement that provides a value.

```rust
fn main() -> i32 {
	print "Hello CA!\n";
	return 1;
}
```

Notice that the definition of the `main` function now returns a value of type `i32`. Make sure to run the program with an appropriate return value.

```
$ ca hello3.ca
program `hello3.ca` :
Hello CA!

return value: 1
```

# 2 Programming a Guessing Game

The traditional number guess Game.

```rust
// define a type alias
type time_t = i64;

// introduce several C functions
extern fn rand() -> i32;
extern fn srand(seed: u32);
extern fn time(tloc: *time_t) -> time_t;
extern fn printf(f: *char, ...) -> i32;
extern fn scanf(format: * char, ...) -> i32;

// the entry of 'CA' program is 'main' function
fn main() -> i32 {
    // output something with inner print statement for debugging
    print "Guess the number!\n";
    
    // call C functions
    srand(time(0) as u32);
    let secret_number = rand() % 100;

    // endless loop statement, use 'break' statement to break the loop
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
            
            // break the loop when find the number
            break;
        }
    }

    return 0;
}
```

Run result

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

## 3.1 Variable

Variable definitions in CA language are similar to those in Rust. You use `let` to bind a value to a variable, and the type of the variable can be inferred from the value on the right-hand side. You can also specify the type of the variable explicitly. If the type of the value being bound does not match the specified type, an error will be reported.

### 3.1.1 Variable Binding

```rust
// variable.ca
let num1 = 100;
let num2 = 100i32;
let num3 = 100i64;
let num4 = 100.1f32;
let num5 = 100.1f64;
let num6: u32 = 100;
let num7: u64 = 100;
let s1 = "Hello CA";
```

### 3.1.2 Variable Shadowing

On let statement section. see [7.1.3 Variable Shadowing](#7.1.3 Variable Shadowing).

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

The function definition like this:

```rust
fn add(a: i32, b: i32) -> i32 {
    return a + b;
}
```

The function named `add` takes 2 parameters, `a` and `b`, both of `i32` type, and it returns a value of type `i32`.

## 3.4 Comments

Currently only single-line comments start with `//` are supported, Support for block comments (e.g. `/* ... */`) will be added in the future.

## 3.5 Control Flow

Support for the well-known control flow.

### 3.5.1 if ... else ...

```rust
// ifelse.ca
fn main() {
    let a = 1;
    
    // single if
    if (a == 1) {
		print "if1...else0: a == 1\n";
    }

    // if + else
    if (a == 1) {
		print "if1...else1: a == 1\n";
    } else {
		print "if1...else1\n";
    }

    let b = 2;

    // if + else + if
    if (a == 2) {
        print "if2...else1: a == 1\n";
    } else if (b == 2) {
        print "if2...else1: b == 2\n";
    }

    // if + else + if + else
    if (a == 2) {
        print "if2...else2: a == 1\n";
    } else if (b == 3) {
        print "if2...else2: b == 2\n";
    } else {
        print "if2...else2\n";
    }
}
```



### 3.5.2 loop { ... }

The endless loop. Can be created using a `loop` statement. You can use `break` to exit the endless loop.

```rust
// loop.ca
fn main() {
    let i = 0;
    let j = 0;
    loop {
        i += 1;
        j += i;
        if (i > 10) {
            break;
        }

        print j; print ' ';
    }
}
```

Run result: `1 3 6 10 15 21 28 36 45 55`.

### 3.5.3 while (...) { ... }

The loop with condition.

```rust
// while.ca
fn main() {
    let i = 0;
    let j = 0;
    while (i < 10) {
        i += 1;
        j += i;
        print j; print ' ';
    }
}
```

Run result: `1 3 6 10 15 21 28 36 45 55`.

### 3.5.4 for

The `for` statement is used to iterate over a range or a collection, such as arrays.

```rust
// for.ca
fn main() {
    let a = [1, 2, 3, 4, 5, 6, 7];
    for (i in a) {
        print i; print ' ';
    }
}
```

Run result: `1 2 3 4 5 6 7`

### 3.5.5 break & continue

```rust
// breakcontinue.ca
fn main() {
    let i = 0;
    loop {
        i += 1;
        if (i % 2 == 0) {
            continue;
        }

        let j = i;
        while (true) {
            j += 1;
            if (j % 3 == 0) {
                continue;
            }

            print j; print ' ';
            if (j > 23) {
                break;
            }
        }

        print '\n';

        if (i > 20) {
            break;
        }

		print "out: "; print i; print '\n';
    }
}
```

Run result:

```
2 4 5 7 8 10 11 13 14 16 17 19 20 22 23 25 
out: 1
4 5 7 8 10 11 13 14 16 17 19 20 22 23 25 
out: 3
7 8 10 11 13 14 16 17 19 20 22 23 25 
out: 5
8 10 11 13 14 16 17 19 20 22 23 25 
out: 7
10 11 13 14 16 17 19 20 22 23 25 
out: 9
13 14 16 17 19 20 22 23 25 
out: 11
14 16 17 19 20 22 23 25 
out: 13
16 17 19 20 22 23 25 
out: 15
19 20 22 23 25 
out: 17
20 22 23 25 
out: 19
22 23 25
```

### 3.5.7 goto

Ooh, the `goto` statement is still supported, although it is generally discarded in many structured programming due to its drawbacks of making code less readable and maintainable.

```rust
fn main() {
    let count = 0;
l1:
    if (count >= 1115000000) {
    	goto l2;
    }

    count = count + 1;
    goto l1;

l2:
    print count;
}
```

Run result: `1115000000`

### 3.5.8 match (Future Planned)

Not implemented yet. Normally it just like the grammar or `rust`.

## 3.6 dbgprinttype

The built-in statement for debugging. It used to print the type information of a type. The variable's type can be get throw `typeof` statement.

```rust
fn main() {
    let num1 = 100;
    let num2 = 100i32;
    let num3 = 100i64;
    let num4 = 100.1f32;
    let num5 = 100.1f64;
    let num6: u32 = 100;
    let num7: u64 = 100;
    let s1 = "Hello CA!";
    let a1: [i8; 12] = "Hello CA!";
    print s1; print '\n';
    print a1; print '\n';
    dbgprinttype typeof(num1);
    dbgprinttype typeof(num2);
    dbgprinttype typeof(num3);
    dbgprinttype typeof(num4);
    dbgprinttype typeof(num5);
    dbgprinttype typeof(num6);
    dbgprinttype typeof(num7);
    dbgprinttype typeof(s1);
    dbgprinttype i32;
    dbgprinttype u64;
    dbgprinttype f32;
    dbgprinttype bool;
}
```

Run result:

```rust
$ ca dbgprinttype.ca 
program `dbgprinttype.ca` :
0x7f5cd293b000
[H, e, l, l, o,  , W, o, r, l, d, !]
size = 4, type: t:i32
size = 4, type: t:i32
size = 8, type: t:i64
size = 4, type: t:f32
size = 8, type: t:f64
size = 4, type: t:u32
size = 8, type: t:u64
size = 8, type: t:*i8
size = 4, type: t:i32
size = 8, type: t:u64
size = 4, type: t:f32
size = 1, type: t:bool

return value: 23
```

The type information is printed with the type's size and type signature.

We should have noticed that the string literal `"Hello CA!"` can be bound to variables with two different types in CA. In statement  `let s1 = "Hello CA!";` the value `"Hello CA!"` is bound to variable `s1` with type `*i8`, and in statement `let a1: [i8; 12] = "Hello CA!";` the value is bound to variable `a1` with an array type of `[i8'12]`.

# 4 Array & Slice

## 4.1 Array

**CA** language offers comprehensive support for multi-dimensional arrays, including:

- **Type Inference**: Automatically determines the type of multi-dimensional arrays, streamlining the coding process.
- **Array Initialization**: Allows for the initialization of multi-dimensional arrays using structured values, in the initialization value, it can include literal or variables. It simplifying the creation and initialization of array.
- **Arbitrary Array Element Type**: supports arbitrary element types in arrays, enabling you to create arrays with elements of any data type, including user-defined types. 

These features allows for a wide range of applications and more dynamic and versatile data structures.

```rust
// array.ca
fn main() {
    // define and initialize an 1-dimensional array variable, with the specified type
    let a1: [i32; 3] = [1, 2, 3];
    
    // define and initialize an 1-dimensional array, with the type infered from right-hand value
    let a2 = [1, 2, 3];

    // Will report error when the specified type is different from the inferred type
    //let a3: [i32; 3] = ["1", "2", "3"];

    // define and initialize an 2-dimensional array variable, with the specified type
    let a4: [[i32; 2]; 3] = [[1, 2], [3, 4], [5, 6]];
    
    // define and initialize an 2-dimensional array, with the type infered from right-hand value
    let a5 = [["1.1", "2.2"], ["3.3", "4.4"], ["5.5", "6.6"]];
    
    let a6 = [[1.1f32, 2.2f32], [3.3f32, 4.4f32], [5.5f32, 6.6f32]];

    print a1; print '\n';
    print a2; print '\n';
    //print a3; print '\n';
    print a4; print '\n';
    print a5; print '\n';
    print a6; print '\n';
}
```

Run result:

```
[1, 2, 3]
[1, 2, 3]
[[1, 2], [3, 4], [5, 6]]
[[0x7f75f5d45000, 0x7f75f5d45004], [0x7f75f5d45008, 0x7f75f5d4500c], [0x7f75f5d45010, 0x7f75f5d45014]]
[[1.100000, 2.200000], [3.300000, 4.400000], [5.500000, 6.600000]]
```

You may noticed that the output of `a5` output the address of the elements. It's because the string literal is represented with `*i8` type, so the `print` statement only output what it sees. In order to output the value of the string element, you can call C function `printf`, it interprets the `*i8` type with a string. So

```rust
// string1.ca
extern fn printf(f: *char, ...) -> i32;
fn main() {
    let a5 = [["1.1", "2.2"], ["3.3", "4.4"], ["5.5", "6.6"]];
    for (aitem in a5) {
        for (item in aitem) {
            printf("%s ", item);
        }

        print '\n';
    }
}
```

Run result:

```
1.1 2.2 
3.3 4.4 
5.5 6.6
```



## 4.2 Array Slice

Similar to Rust, the array slice operation retrieves a range of items from an array and provides a reference to that subrange.

```rust
// slice.ca
fn main() {
    let a1 = [1, 2, 3, 4, 5, 6, 7];
    let s1 = a1[3..6];
    let s2 = a1[3..7];
    let s3 = a1[2..6];
    let s4 = a1[2..7];

    print a1; print '\n';
    print s1; print '\n';
    print s2; print '\n';
    print s3; print '\n';
    print s4; print '\n';

    dbgprinttype(typeof(a1));
    dbgprinttype(typeof(s1));
    dbgprinttype(typeof(s2));
    dbgprinttype(typeof(s3));
    dbgprinttype(typeof(s4));

    // Not implemented yet: for, element accessing
    // for (item in s1) {
    // 	print item;
    // }  
    // print s1[1];
}
```

Run result

```
[1, 2, 3, 4, 5, 6, 7]
 < 0x7ffed0fb2694, 3 >
 < 0x7ffed0fb2694, 4 >
 < 0x7ffed0fb2678, 4 >
 < 0x7ffed0fb2678, 5 >
size = 28, type: t:[i32;7]
size = 16, type: t:<slice;;*i32,i64>
size = 16, type: t:<slice;;*i32,i64>
size = 16, type: t:<slice;;*i32,i64>
size = 16, type: t:<slice;;*i32,i64>
```

We can see from the output the value of array slice that it consist of the start address and the length of the slice. The slice type is structured as two-element entity: the first element is a pointer to the array's elements, and the second element is the length of the slice.

## 4.3 Array Copy

The array assignment operation copies the elements from one array to another. Regardless of the element type, the operation duplicates the values directly. If the value is of a primitive type, the value is copied directly. If the value is of a complex type, its value is also copied directly through memory copy, without using a reference or creating a pointer.

```rust
// array_copy.ca
fn main() {
    let a1 = ["hello", "CA"];
    let a2 = a1;

    print "For string element:\n";
    print a1[0]; print '\n';
    print a2[0]; print '\n';

    dbgprinttype(typeof(a1[0]));
    dbgprinttype(typeof(a2));

    let a3 = [[1,2],[3,4],[5,6]];
    let a4 = a3;

    print "\nFor array element:\n";
    print "value: "; print a3[0]; print " address: "; print &a3[0]; print '\n';
    print "value: "; print a4[0]; print " address: "; print &a4[0]; print '\n';

    dbgprinttype(typeof(a3[0]));
    dbgprinttype(typeof(a4));

    let a5 = [(1,2),(3,4),(5,6)];
    let a6 = a5;

    print "\nFor tuple element:\n";
    print "value: "; print a5[0]; print " address: "; print &a5[0]; print '\n';
    print "value: "; print a6[0]; print " address: "; print &a6[0]; print '\n';

    dbgprinttype(typeof(a5[0]));
    dbgprinttype(typeof(a6));
}
```

Run result:

```
For string element:
0x7ff078307000
0x7ff078307000
size = 8, type: t:*i8
size = 16, type: t:[*i8;2]

For array element:
value: [1, 2] address: 0x7ffd23c18888
value: [1, 2] address: 0x7ffd23c188a0
size = 8, type: t:[i32;2]
size = 24, type: t:[[i32;2];3]

For tuple element:
value: ( 1, 2 ) address: 0x7ffd23c188b8
value: ( 1, 2 ) address: 0x7ffd23c188d0
size = 8, type: t:(;i32,i32)
size = 24, type: t:[(;i32,i32);3]
```

From the result, we can observe that for a literal string, which has a base type of `*i8`, the operation simply copies the `*i8` value into the new slot of the array. It does not copy the entire string itself but only the `*i8` pointer value. This occurs because the string type is represented as a type of ` *i8` a pointer of `i8` type.

# 5 Structs type

CA Using `struct` to Structure Related Data,

## 5.1. Defining and Instantiating Structs

```rust
// struct1.ca
struct User {
    active: bool,
    username: *char,
    email: *char,
    sign_in_count: u64,
}
```

Similar to Rust, it defines the `struct` using a right-hand side type and a left-hand side value approach. `*char` stand for a `C` string. It's better to use `C` functions to coping the `C` string. Will enhance `CA` language runtime library in the future to support `string` type.

```rust
// struct1.ca
fn main() {
    let user1 = User {
        active: true,
        username: "someusername123",
        email: "someone@example.com",
        sign_in_count: 1u64
    };

    print user1;
}
```

Run result: `User { active: 1, username: 0x7fb1c6195000, email: 0x7fb1c6195010, sign_in_count: 1 }`

## 5.2 Accessing struct field

In most programming languages, including C, the dot operator (`.`) is used to access a field of an object when the object is an instance rather than a pointer to it. When dealing with pointers, the arrow operator (`->`) is used instead.

### 5.2.1 Using (`.`)

```rust
// struct2.ca
extern fn printf(f: *char, ...) -> i32;
fn main() {
    // We can also define a new type within a function in different scope,
    // and the type will be visible only within that function's scope.
    struct User {
		active: bool,
		username: *char,
		email: *char,
		sign_in_count: u64,
    }

    let user1 = User {
        active: true,
        username: "someusername123",
        email: "someone@example.com",
        sign_in_count: 1u64
    };

    print "active: "; print user1.active; print '\n';
    
    // using printf output the C string
    printf("username: %s\n", user1.username);
    printf("email: %s\n", user1.email);
 
    print "sign_in_count: "; print user1.sign_in_count; print '\n';
}
```

Result

```
active: 1
username: someusername123
email: someone@example.com
sign_in_count: 1
```

We can also define a new type within a function or within any blocks inside a function. This type will have a scope limited to the block or function where it is defined and will be visible only within that specific scope.

### 5.2.2 Using (`->`)

When the variable is a pointer to the object, in order to access the field of the object, you need using arrow `->` operator to access the field of the object.

```rust
// struct3.ca
fn main() {
    ...
    let user1 = &user1;
    print "active: "; print user1->active; print '\n';
    
    // using printf output the C string
    printf("username: %s\n", user1->username);
    printf("email: %s\n", user1->email);
 
    print "sign_in_count: "; print user1->sign_in_count; print '\n';
}
```

## 5.3 Tuple struct

The **tuple struct** is a special `struct` type which in the form of a tuple.

You can define a **tuple struct** where the fields do not have names. In such a struct, you specify the types of the fields without assigning individual names to them. This allows for a more compact definition of data structures, especially when field names are not required.

```rust
// tuple_struct.ca
struct Color(i32, i32, i32);
struct Point(i32, i32, i32);
struct Pixel(Point, Color);

fn main() {
    let black = Color(0, 0, 0);
    let origin = Point(0, 0, 0);
    let pixel = Pixel(origin, black);
    print black; print '\n';
    print origin; print '\n';
    print pixel; print '\n';
}
```

Run result

```
Color ( 255, 0, 255 )
Point ( 0, 255, 51 )
Pixel ( Point ( 0, 255, 51 ), Color ( 255, 0, 255 ) )
```

## 5.5 general tuple

The `tuple struct` is a kind of named type, while the general tuple type is unnamed type, you can create and use it flexibly. Unlike `struct` or `tuple struct`, the `general tuple` type does not have an explicit type name, additionally, the literal values of a general tuple also do not require explicit type names to create, so this approach provides greater flexibility.

```rust
// gentuple1.ca
fn main() {
    // Create an unnamed tuple, bind it into a variable and initalize it with value
    let a1: (i32, i32) = (3, 4);
    print a1; print '\n';
    
    // Assign a value to an unnamed item at a specified position within the tuple
    a1.0 = 100;
    a1.1 = 1000;
    print a1; print '\n';

    // Create a temporary unnamed tuple, and print it directly
    print (a1.0 + 1, a1.1 + 1); print '\n';

    // Create an unnamed tuple and bind it to a variable, and infer variable's type
    // from the unnamed tuple's value and structure.
    let a2 = (3, 4);
    print a2; print ' ';

    // Bind an unnamed tuple to a variable, and infer it's type. The tuple element can be of complex type
    let a3 = ([1,2,3], 4.5);
    print a3; print '\n';
}
```

Run result:

```
( 3, 4 )
( 100, 1000 )
( 101, 1001 )
( 3, 4 ) ( [1, 2, 3], 4.500000 )
```

So, a general tuple is somewhat similar to an array, but there are key differences: an array requires all its elements to be of the same type, while a general tuple allows for elements of different types.

### 5.5.1 Naming a general tuple

Although a general tuple does not have a name, you can assign it with a name using a type alias.

```rust
type A = (i32, i32);
```

Then you can use the name to declare variables

```rust
// type_gentuple.ca
type A = (i32, i32);

fn main() {
    let a: A;
    a = (100, 200);
    print a;
}
```

Run result: `( 100, 200 )`



## 5.4 Examples of Using Struct

**Using tuple**

```rust
// tuple2.ca
extern fn printf(fmt: *char, ...) -> i32;

fn main() {
    let rect1 = (30u32, 50u32);
    printf("The area of the rectangle is %d square pixels.", area(rect1));
}

fn area(dimensions: (u32, u32)) -> u32 {
    return dimensions.0 * dimensions.1;
}
```

Run result: `The area of the rectangle is 1500 square pixels.`

**Using Struct**

```rust
// struct4.ca
extern fn printf(fmt: *char, ...) -> i32;

struct Rectangle {
    width: u32,
    height: u32,
}

fn main() {
    let rect1 = Rectangle {
        width: 30u32,
        height: 5u32
    };

    printf("The area of the rectangle is %d square pixels.", area(&rect1));
}

fn area(rectangle: *Rectangle) -> u32 {
    return rectangle->width * rectangle->height;
}
```

Run result: `The area of the rectangle is 150 square pixels.`

## 5.5 Method in Struct

### 5.5.1 Method Definition

Similar to `Rust`, the `CA` language also defines method in `impl` blocks for the type. The first parameter of a method of a `struct` must be `self`, different from `Rust`, the `self` parameter is a pointer to the instance of current object.

```rust
// method1.ca
extern fn printf(fmt: *char, ...) -> i32;

struct Rectangle {
    width: u32,
    height: u32,
}

// Implementing the method of Rectangle type
impl Rectangle {
    fn area(self) -> u32 {
    	// self is of type *Rectangle, so the field must accessed through arrow -> operator, not dot .
        return self->width * self->height;
    }
}

fn main() {
    let rect1 = Rectangle {
        width: 123u32,
        height: 345u32
    };

    // call method area
    let area = rect1.area();
    printf("The area of the rectangle is %d square pixels.", area);
}
```

Run result: `The area of the rectangle is 42435 square pixels.`

You can define methods within an `impl` block. Multiple methods can be included in a single `impl` block. Additionally, you can have multiple `impl` blocks for the same type, with each block containing any number of methods.

```rust
// method2.ca
extern fn printf(fmt: *char, ...) -> i32;

struct Rectangle {
    width: u32,
    height: u32,
}

// Implementing the method of Rectangle type
impl Rectangle {
    fn area(self) -> u32 {
    	// self is of type *Rectangle, so the field must accessed through arrow -> operator, not dot .
        return self->width * self->height;
    }

    fn can_hold(self, other: *Rectangle) -> bool {
        return self->width > other->width && self->height > other->height;
    }
}

fn main() {
    let rect1 = Rectangle {
        width: 123u32,
        height: 345u32
    };

    let rect2 = Rectangle {
        width: 126u32,
        height: 344u32
    };

    // call method area
    let area = rect1.area();
    printf("The area of the rectangle 1 is %d square pixels\n", area);
    printf("The area of the rectangle 2 is %d square pixels\n", rect2.area());

    // call another method with extra parameter
    let can = rect2.can_hold(&rect1);
    let can_str: *char = __zero_init__;
    if (can) {
		can_str = "can";
    } else {
		can_str = "cannot";
    }

    printf("The first rectangle %s hold the second rectangle\n", can_str);

    // invoke the associated function
    let square = Rectangle::square(100);
    print square;
}

// Postpone the implementation
impl Rectangle {
    // The associated function, it is not a method, because the first parameter is not self
    fn square(size: u32) -> Self {
        return Self {
            width: size,
            height: size
        };
    }
}
```

Run result:

```
The area of the rectangle 1 is 42435 square pixels
The area of the rectangle 2 is 43344 square pixels
The first rectangle cannot hold the second rectangle
Rectangle { width: 100, height: 100 }
```



### 5.5.2 Associated Functions

Similar to Rust conventions, all functions defined within an `impl` block are called associated functions. When the first parameter of associated function is not `self`, so it's not a method.

See the upper examples. It implemented function which returns a `Rectangle` object.

```rust
// method2.ca

// Postpone the implementation
impl Rectangle {
    // The associated function, it is not a method, because the first parameter is not self
    fn square(size: u32) -> Self {
        return Self {
            width: size,
            height: size
        };
    }
}
```

## 5.6 Enum type (Future Planned)

Note implemented yet.

# 6 Pointer

`CA` language supports multi-dimensional pointers, enabling complex data structures and providing flexibility in their use.

## 6.1 Pointer Declaration & Initialization

The following code demonstrates how to defines the pointers with different dimensions and type.

```rust
// pointer1.ca
fn main() {
    let i = 34i8;
    let p1: *i8 = &i;// 1-dimension pointer to 1 byte integer and initialize it
    let p2: **i32;	// 2-dimension pointer to 4 byte integer without initialize the value
    let p3: ***f64;	// 3-dimension pointer to double float
    let p4: *Point;	// 1-dimension pointer to a struct
    let p5: **Point;// 2-dimension pointer to a struct
    
    let pt = Point(3.2, 6.4);
    p4 = &pt;	// define a pointer to a Point tuple struct
    p5 = &p4;	// define a pointer to a Point tuple struct
}

struct Point (f32, f32);
```

## 6.2 Declare pointers by inferring

The following code demonstrates how to declare pointers of various dimensions, with their types inferred from the right-hand side value:

```rust
// pointer2.ca
fn main() {
    let a = 1;
    
    // following defines different pointer types, their type in inferenced
    // automatically from the value
    let pa = &a;			// Define a pointer to an integer variable.
    let ppa = &pa;			// Define a pointer to a 1-dimension pointer.
    let pppa = &ppa;		// Define a pointer to a 2-dimension pointer.
    
    // print the variable type
    dbgprinttype typeof(a);
    dbgprinttype typeof(pa);
    dbgprinttype typeof(ppa);
    dbgprinttype typeof(pppa);
}
```

Run result:

```
size = 4, type: t:i32
size = 8, type: t:*i32
size = 8, type: t:**i32
size = 8, type: t:***i32
```



## 6.3 C string pointer

Since a C string is a sequence of characters of type char stored in memory and terminated by a null character ('\0'), it is suitable to use a char pointer to point to the memory area and reference it's value. The handling of C strings relies on various C standard library functions.

In the `CA` language, pointers are supported, as well as the `i8` (or `char`) type. Therefore, you can use an `i8` pointer to point to a  memory area which is meet the definition of `C` string, and handle it with C functions.

```rust
// pointer3.ca
extern fn printf(fmt: *i8, ...) -> i32;
extern fn strlen(s: *i8) -> i32;

fn main() {
    // str is a pointer to a char type. It is initialized with a string literal
    let str: *char = "Hello CA!\n";
    printf(str);

    // call C function strlen to get the C string length
    let len = strlen(str);
    print "length: "; print len; print '\n';

    // astr is an array, and initialized with several char literals
    let astr = ['H', 'e', 'l', 'l', 'o', ' ', 'C', 'A', '\0'];
    print astr; print '\n';

    // cast char array into char pointer, array variable implicitly represents the address of the array
    let str2: *char = astr as *char;
    printf(str);

    let len = strlen(str2);
    print "length: "; print len; print '\n';
}
```

Run result:

```
Hello CA!
length: 10
[H, e, l, l, o,  , C, A, ]
Hello CA!
length: 8
```

Will support CA of it's own String in the future in runtime library 

## 6.4 Value Deref

Dereference the value from pointer.

**Dereference with star `(*)` operator**

Dereference characters from a `C` string.

```rust
// deref1.ca
extern fn printf(fmt: *i8, ...) -> i32;
fn main() {
    // define a C string, terminated by '\0' character
    let str: *char = "Hello CA!\n";
    let p = str;

    // dereference the value from pointer `p` and compare them
    while(*p != '\0') {
	// dereference the value and get the next character in its ASCII encoding
	print ((*p + 1char) as i32 % 128) as char; print ' ';

	// move the pointer forward by 1 step
	p = p + 1;
    }
}
```

Result: `I f m m p ! D B "`

**Dereference field of struct with star `(->)` operator**

If the variable is a pointer point to a `struct`, we can use `->` operator to dereference the value. As well as using `*` operator.

```rust
// deref2.ca
extern fn printf(fmt: *i8, ...) -> i32;
fn main() {
    // define a C string, terminated by '\0' character
    let pt = Point {x: 100, y: 200, z: 150};
    let ppt = &pt;

    // Dereference field of Point with '*'
    printf("Dereference field of Point with '*'\n");
    printf("Point: (%d, %d, %d)\n\n", (*ppt).x, (*ppt).y, (*ppt).z);

    // Dereference field of Point with '->'
    printf("Dereference field of Point with '->'\n");
    printf("Point: (%d, %d, %d)\n", ppt->x, ppt->y, ppt->z);
}

struct Point {
    x: i32,
    y: i32,
    z: i32
}
```

Run result:

```
Dereference field of Point with '*'
Point: (100, 200, 150)

Dereference field of Point with '->'
Point: (100, 200, 150)
```

# 7 Patterns Matching and variable binding

Pattern matching functionality is used to determine whether a value conforms to a specific pattern. The pattern represents a structure, template, or shape that defines what you are looking for. When a value matches this pattern, the system can extract or handle data according to the structure defined by the pattern.

In CA language, the `let` and `match` statements use patterns to bind values to variables and extract data.

- **`let` Statement**: For the `let` statement, both sides of the assignment must be of the same type. The pattern on the left side defines variables that will be bound to the corresponding parts of the value on the right side. These variables are assigned values based on their positions in the pattern. For example, in `let (x, y) = (1, 2);`, `x` is bound to `1` and `y` is bound to `2`.
- **`match` Statement (Future Planned)**: The `match` statement evaluates a value against several patterns. It sequentially checks each pattern until it finds one that matches the value. When a pattern matches, the corresponding block of code is executed. If no patterns match, a catch-all pattern (typically `_`) can be used to handle any remaining cases.

## 7.1 let statement

The `let` statement can be use in different aspect in CA language. I's a fundamental construct in CA for variable declaration and pattern matching.

### 7.1.1 Variable Binding

```rust
let x = 5;
```

`x` is bound to value `5` with type `i32` directly. After that `x` will be a variable with `i32` type.

### 7.1.2 Pattern Matching

***Destructing general tuples***

```rust
let (a, b, c) = (1, 2u32, 3.14f64);
```

The statement firstly do a pattern matching for the 2 sides, and then binding the variables in the left hand, so after that the variable `a` is bound to `1` with type `i32`, and `b` to 2 with type `u32`, `c` to 3.14 with type `f64`.

***Destructing structs***

```rust
struct Point {
    x: i32,
    y: i32,
}

let point = Point { x: 10, y: 20 };

// In this statement, the left-hand defines a pattern, the right-hand side is the value. The pattern
// and the value must be the same type or make it the same type during the variable binding.
// Within the pattern there are 2 variables which accept the destructed data from the left-hand value
let Point { x, y } = point;
```

The statement first matches the value against the patterns and then performs bindings based on the matched pattern. So finally `x` will be 10, `y` will be 20 of type `i32`.

### 7.1.3 Variable Shadowing

In CA language the variable shadowing occurs when a new variable is declared with the same name as a previously declared variable within the same scope. The newly declared variable "shadows" the older one, meaning that any references to that name within the current scope will refer to the newer variable. When the variable with the same name is defined in different scope, they are not called shadowing.

```rust
// shadowing.ca
fn main() {
    // this declares a variable x and initializes it with the value 100
    let x = 100;
    print x; print '\n';

    // this line reuses the name x and assigns it a new value
    let x = x + 1;
    print x; print '\n';

    // this line reuses the name x again, but now assigns it a new value and type
    let x = 1.22;
    print x; print '\n';
}
```
Run result:

```
100
101
1.220000
```

## 7.2 Various examples of let statement

```rust
// various_let.ca
fn main() {
    // define a tuple variable and initalize all it's memory area with zero value
    let a: (i32, (i32, i32)) = __zero_init__;
    print a; print '\n';
    
    // this statement declares two variables: `a1` and `b1` use the form of pattern matching
    // the two variables are expected to be part of a tuple with two `i32` values, 
    // and initialize them with zero value
    let (a1, b1): (i32, i32) = (3, 4);
    print (a1, b1); print '\n';

    // define a tuple struct
    struct A(i32, i32);

    let a2 = A(300, 200);

    // uses the pattern to extract data from a2, and bind data into variable n2 and o2
    let A(n2, o2) = a2;

    // the right-hand A(n3, o3) is the pattern, the m3 stand for the whole of pattern.
    let m3@A(n3, o3) = a2;
    print (m3, n3, o3); print '\n';

    // the m3 and n3 is bound the whole value of the right-hand.
    // aa and a3 bound to value 100
    // bb is bound to (200, 300)
    // b3 is bound to 200, and b31 is bound to 300
    let m3@n3@(aa@a3, bb@(b3, b31)) = (100, (200, 300));
    print (m3, n3, aa, a3, bb, b3, b31);
}
```

Run result:

```
( 0, ( 0, 0 ) )
( 3, 4 )
( A ( 300, 200 ), 300, 200 )
( ( 100, ( 200, 300 ) ), ( 100, ( 200, 300 ) ), 100, 100, ( 200, 300 ), 200, 300 )
```

For more testing code see the directory of `test/pattern` in the source code.

## 7.3 match statement (Future Planned)

Not implemented yet.

# 8 Traits

In CA, a trait is a fundamental concept that allows for defining shared behavior across different types. It is somewhat analogous to interfaces in other languages but with some unique features.

## 8.1 Definition of a Trait

A trait defines a set of methods that types implementing the trait must provide. It can also include default method implementations. Traits are used to specify a contract that types must adhere to.

Following declared 2 method in trait, one is the normal, another method has a default implementation.

```rust
// trait1.ca
trait TraitName {
    // method signatures without implementation
    fn method_name(self);

    // method with a default implementation
    fn default_method(self) {
        print("This is the default implementation.");
    }
}
```



## 8.2 Implementing a Trait

To use a trait, a type must implement it. This is where you provide the actual code for the methods declared in the trait.

```rust
// trait1.ca
// define a empty struct or called unit struct
struct MyStruct {};

// implement the trait for the struct
impl TraitName for MyStruct {
    fn method_name(self) {
        print "Method implemented for MyStruct.\n";
    }
}
```

The implementation of the type itself, it not affected by the traits implementation, so.

```rust
// trait1.ca
impl MyStruct {
    fn new() -> Self {
	return MyStruct{};
    }
}
```



## 8.3 static method (associated function) in traits

The method defined in a trait without using `self` parameter is called static method or trait-associated function. The functions must be called using the trait's name, rather than on an instance of the implemented type.

```rust
// trait2.ca
trait TraitName {
    // static method
    fn static_method();
    
    // static method with default implmentation
    fn default_static_method() {
        print("This is the default implementation of a static method.\n");
    }
}
```

When the method in trait have no default implementation the type that implement the trait must implement the method by itself.

## 8.4 Using Traits

Traits can be used in various ways, even when the dynamic feature and generic feature is not implemented

- Providing default method implementations: traits can provide default implementation for methods, allowing implementors to inherit these implementations when they don't need to override them, this enables you to write reusable code that can be shared among types.
- Defining trait bounds of associated functions or methods for it's implementors.
- Implementing functionality that leverages trait methods internally.

When enabled the features of dynamic and generic, please reference `rust` language.

```rust
// trait1.ca
fn main() {
    let obj = MyStruct::new();
    obj.method_name();					// call method
    obj.default_method();				// call method with default implmentation in trait
    MyStruct::static_method();			// call static method
    MyStruct::default_static_method();	// call static method of default trait implmentation
}
```

A complex example

```rust
// trait2.ca
extern fn printf(fmt: *char, ...) -> i32;
trait Shape {
    // instance methods
    fn area(self) -> f64;
    fn perimeter(self) -> f64;
    fn draw(self);
    fn bounding_box(self) -> Rect;
    fn contains_point(self, point: Point) -> bool;
    fn transform(self, transformation: Transformation);

    // static methods
    fn default_shape() -> Self;
    fn from_dimensions(dimensions: *f64, size: usize) -> Self;

    fn validate_dimensions(dimensions: *f64, size: usize) -> bool {
        // default implementation: assume dimensions are valid if not empty
        return size != 0usize;
    }

    fn unit_shape() -> Self;

    fn calculate_area(dimensions: *f64, size: usize) -> f64 {
        // default implementation: returns 0 if dimensions are not suitable
		return 0.0;
    }

    fn calculate_perimeter(dimensions: *f64, size: usize) -> f64 {
        // default implementation: returns 0 if dimensions are not suitable
		return 0.0;
    }
}

// Example implementation for Circle
struct Circle {
    radius: f64,
}

impl Shape for Circle {
    fn area(self) -> f64 {
        return 3.14f64 * self->radius * self->radius;
    }

    fn perimeter(self) -> f64 {
        return 2.0 * 3.13f64 * self->radius;
    }

    fn draw(self) {
        printf("Drawing a circle with radius: %lf\n", self->radius);
    }

    fn bounding_box(self) -> Rect {
        return Rect {
            width: 2.0 * self->radius,
            height: 2.0 * self->radius
        };
    }

    fn contains_point(self, point: Point) -> bool {
        let dx = point.x;
        let dy = point.y;
        return (dx * dx + dy * dy) <= (self->radius * self->radius);
    }

    fn transform(self, _transformation: Transformation) {
        // apply transformation to the circle
    }

    // static methods
    fn default_shape() -> Self {
        return Circle { radius: 1.0 };
    }

    fn from_dimensions(dimensions: *f64, size: usize) -> Self {
        if (size == 1usize && *dimensions > 0.0) {
            return Circle { radius: *dimensions };
        } else {
            return Circle { radius: 0.0 };
        }
    }

    fn unit_shape() -> Self {
        return Circle { radius: 1.0 };
    }

    fn calculate_area(dimensions: *f64, size: usize) -> f64 {
        if (size == 1usize) {
            return 3.14f64 * *dimensions * *dimensions;
        } else {
            return 0.0;
        }
    }

    fn calculate_perimeter(dimensions: *f64, size: usize) -> f64 {
        if (size == 1usize) {
            return 2.0 * 3.14f64 * *dimensions;
        } else {
            return 0.0;
        }
    }
}

// Definitions for Rect, Point, and Transformation
struct Rect {
    width: f64,
    height: f64,
}

struct Point {
    x: f64,
    y: f64,
}

struct Transformation {
    // Define transformation details here
}

fn main() {
    // Create a default Circle
    let default_circle = Circle::default_shape();
    printf("Default Circle: radius = %lf\n", default_circle.radius);
    printf("Area: %lf\n", default_circle.area());
    printf("Perimeter: %lf\n", default_circle.perimeter());

    // Create a Circle from dimensions
    let dimensions = 5.0f64;
    let circle = Circle::from_dimensions(&dimensions, 1);
    
    if (circle.radius != 0.0){
        printf("Created Circle from dimensions:\n");
        printf("Area: %lf\n", circle.area());
        printf("Perimeter: %lf\n", circle.perimeter());
    } else {
        printf("Invalid dimensions for Circle.\n");
    }

    // Validate dimensions
    if (Circle::validate_dimensions(&dimensions, 1)) {
        printf("Dimensions are valid for Circle.\n");
    } else {
        printf("Dimensions are not valid for Circle.\n");
    }

    // Calculate area and perimeter from dimensions
    printf("Calculated Area: %lf\n", Circle::calculate_area(&dimensions, 1));
    printf("Calculated Perimeter: %lf\n", Circle::calculate_perimeter(&dimensions, 1));
}
```

Run result:

```
Default Circle: radius = 1.000000
Area: 3.140000
Perimeter: 6.260000
Created Circle from dimensions:
Area: 78.500000
Perimeter: 31.300000
Dimensions are valid for Circle.
Calculated Area: 78.500000
Calculated Perimeter: 31.400000
```



## 8.5 Trait object (Future Planned)

Will implement in the future.

# 9 Object Oriented Programming (OOP)

According to some senses, not all senses, the Object Oriented Languages have following common characteristics

- objects
- encapsulation
- inheritance
- polymorphism
- abstraction

According to the upper definition, the CA language is not a fully object oriented language.

But according to the GoF (Gang of Four), if OOP is defined like this: "Object-oriented programs are made up of objects. An *object* packages both data and the procedures that operate on that data. The procedures are typically called *methods* or *operations*." Then CA is a OOP language.

## 9.1 Objects

CA language support create object through `struct`, it can packages both data and the procedures that operate on that data.

```rust
// objects.ca
extern fn printf(fmt: *char, ...) -> i32;

struct Phone {
    size: Rectangle,
    os: *char,
    satellite: bool,
    cell: u32,
}

struct Rectangle {
    width: u32,
    height: u32,
}

impl Phone {
    fn new() -> Phone {
        return Phone {
            size: Rectangle {width: 0u32, height: 0u32},
            os: "android",
            satellite: false,
            cell: 0u32
        };
    }

    fn operating_system(self) -> *char {
		return self->os;
    }

    fn support_satellite_call(self) -> bool {
		return self->satellite;
    }
}

fn main() {
    let phone = Phone::new();
    // access the `size` field directly. It not allowed when the field accessing control is supported
    print phone.size; print '\n';
    printf("%s\n", phone.operating_system());
}
```



## 9.2 Encapsulation

According to `rust` documentation, the encapsulation means that the implementation details of an object aren’t accessible to code using that object. Therefore, the only way to interact with an object is through its public API; code using the object shouldn’t be able to reach into the object’s internals and change data or behavior directly. This enables the programmer to change and refactoring an object’s internals without needing to change the code that uses the object.

For CA language, currently all field include data field can be accessed directly. The CA compiler will add field accessing control in the future. After that, it will support `pub` keyword to control the field accessing, by default the field without a `pub` prefix, it will cannot accessed directly outside the method of the object, and that time it will fully support the encapsulation feature. 

*Notice: following program will get compile error, for currently the compiler not support the pub keyword*

```rust
// encapsulation.ca
extern fn printf(fmt: *char, ...) -> i32;

struct Phone {
    size: Rectangle,
    os: *char,
    satellite: bool,
    cell: u32,
}

struct Rectangle {width: u32, height: u32}

impl Phone {
    pub fn new() -> Phone {
        return Phone {
            size: Rectangle {width: 0u32, height: 0u32},
            os: "android",
            satellite: false,
            cell: 0u32
        };
    }

    // all field must be accessed by the method in object, when acccess control supported
    pub fn size(self) -> Rectangle { return self->size; }
    pub fn set_size(self, rect: Rectangle) { self->size = rect; }

    pub fn operating_system(self) -> *char { return self->os; }
    pub fn support_satellite_call(self) -> bool { return self->satellite; }
}

fn main() {
    let phone = Phone::new();

    // the size must be accessed through the method in the object
    print phone.size(); print '\n';
    printf("%s\n", phone.operating_system());
}
```



## 9.3 Inheritance (only for trait)

CA language only support the inheritance through `traits`, it not support object instantiation directly.

## 9.4 Polymorphism (Future Planned)

Through `trait` object, CA support polymorphism. This functionality is under development.

## 9.5 Abstraction

The `trait` and `generic` programming offers abstraction to object in CA. This functionality is under development.

# 10 Dynamic memory management 

## 10.1 dynamic memory allocation - boxing

The object mentioned above are allocated on the stack. As a result, the memory for object and any variables it references will be released once the function is returned. To ensure that the object's memory persists after the function returns, the memory must be allocated on heap.

### 10.1.1 Use box statement

The simplest way to use box

```rust
// allocate a memory of i32 type with value 0 on the heap
box(0); // or `box 0`;
```

###  10.1.2 Access heap memory

```rust
// Allocate an `i32` type on heap and initialize it with the value `1`.
let i = box 1;
print *i; // will get output of `1`
*a = 2022;
print *a; // will get output of `2022`
```

### 10.1.3 Boxing string literal

```rust
// box1.ca
extern fn printf(fmt: *char, ...) -> i32;
fn main() {
    box(0);
    
    // s1 is of **i8 type, because "Hello CA!" is a *i8 type
    let s1 = box("Hello CA!");
   	// dereference the value of **i8 and get *i8 value
    printf("%s\n", *s1);

    // boxing an char array
    let s2 = box(['H', 'e', 'l', 'l', 'o', ' ', 'C', 'A', '!', '\0']);
    printf("%s\n", s2);
}
```

Run result:

```
Hello CA!
Hello CA!
```

The string literal is treated as `*i8` type internally, so after boxing, the boxed type will be `**i8`. We must dereference it in order to make it suitable for invoking C function `printf`.

Array type is an object, so `s2` is an pointer to the array. It's different from the `*i8` string, even though the array name can be treated as the address of the array. it's different notion.

### 10.1.4 Boxing struct type

```rust
// box2.ca
extern fn printf(s: *char, ...) -> i32;
fn main() {
    // create an Record object
    let obj = Record{33, 323.333};

    // boxing the Record object, objptr1 is a pointer to Record, or `*Record` type
    let objptr1 = box(obj);
    // dereference and print it's value
    print (objptr1); print '\n';

    // create a Record object and boxing it
    let objptr2 = box(Record{33, 323.333});
    print (*objptr2); print '\n';
}

struct Record {
    f1: i32,
    f2: f64
}
```

Run result:

```
Record { f1: 33, f2: 323.333000 }
Record { f1: 3, f2: 3.330000 }
```

### 10.1.5 Boxing a buffer (Future Planned)

Not implemented current in compiler. By design, the grammar will be.

```rust
let buffer_size = 100;
let buffer = box(i8; buffer_size);
```

The type followed a size. It will allocate memory in heap without initialize a value.

### 10.1.6 Different scenarios use Heap Memory

- Pass out of the function
- Use in data structure's, like list, tree
- dynamic size, for example: allocate a buffer with unknown size at compile time
- avoid stack overflow: allocate large data structures in heap, make deep recursive

## 10.2 Garbage Collection

To simplify memory management and eliminate concerns about manual deallocation, the CA language introduced a garbage collection mechanism. Uses garbage collection, it's similar to programming in `java` or `C#`, where a garbage collection mechanism is already built-in. You don't need worry about the memory management: when you need a new memory in heap, just allocate it, and then use it without considering the release of the variable.

### 10.2.1 Use Mark-and-Sweep algorithm

Unlike other garbage-collected languages, such as `java`, which use reference counting to manage object deallocation. CA language employs a different mechanism for controlling object release, it uses Mark-and-Sweep algorithm to do the job with support of libgc library.

Simply speak Mark-and-Sweep algorithm have 2 stages: in Mark stage it scans the whole heap area and try to mark all the reachable objects. The reachable object which can be accessed directly or indirectly from root set (e.g. global variables, or stack variables in function). in Sweeping stage, it releases the object which not marked and keep the marked object.

### 10.2.2 drop

Although the heap memory can be released automatically when they never be used, you can still use `drop` statement to release the memory manually which is allocated by box. This ensuring resources are released as soon as an object goes out of scope. This is crucial for ensuring that resources are released in a timely manner.

```rust
fn main() {
    let v1 = box(100.33f64);
    ...
    drop(v1);
}
```



## 10.3 use extern memory mechanism

It is not recommended to mix external memory management mechanisms, such as C functions like `malloc` and `free`, with the language's built-in memory management. Doing so can add complexity and may lead to issues if not handled properly.

# 11 Generic Programming (Future Planned)

Not Implemented Yet.

# 12 Module Design (Future Planned)

Not Implemented Yet.

# 13 Special Language Features

The CA language specifies several features. These features collectively enhance the language’s capability, providing a rich set of tools for diverse programming needs.

## 13.1 Internal Print Statements

`print` or `dbgprint` statement, useful for debugging by allowing developers to output information directly from the code. `print` is aliases to `dbgprint` for convenient use, They can print variables of any type with the internal format.

```rust
let v1 = 100;
print v1;
dbgprint(v1);

let a1 = [2, 0, 2, 4, 0, 8];
print(a1); print('\n');

struct Tree {
    height: f32,
    diameter: f32
}

let tree = Tree {height: 20.3, diameter: 0.5};
dbgprint(tree);
```

## 13.2 Type aliases

Type aliases allow you to create new names for existing types. Type aliases in CA are a powerful tool for making your code more readable and manageable by providing descriptive names for existing types. They do not create new types but offer a way to simplify and clarify your code. 

```rust
type GlassDegree = u32;
let degree: GlassDegree = 100;

type Point = (i32, i32);
let pt: Point = __zero_init__;

struct InternationalizationUtilities {
    charset: *char,
}

type I18N = InternationalizationUtilities;
let toolkit = I18N {charset: "utf-8"};
```

## 13.3 Type Conversions

Facilitates seamless transformation between different data types, improving flexibility and interoperability.

```rust
// cast.ca
struct Point (u32, u32);

fn main() {
    // cast an hex number into a pointer of i8
    let message_address = 0x432abc4f as *i8;

    // create unamed tuple
    let point_tuple = (100u32, 200u32);

    // cast unnamed tuple into named tuple with the same fields
    let point_ptr = &point_tuple as *Point;

    print &point_tuple; print '\n';
    print point_ptr; print '\n';
    
    dbgprinttype(typeof(point_tuple));
    print point_tuple; print "\n\n";

    dbgprinttype(typeof(*point_ptr));
    print *point_ptr; print '\n';
}
```

Result

```
0x7fffa69673d8
0x7fffa69673d8
size = 8, type: t:(;u32,u32)
( 100, 200 )

size = 8, type: t:(Point;u32,u32)
Point ( 100, 200 )
```

We can see from the result, the address and the value is not changed, but the data type is really changed.

## 13.4 Zero Initialization

CA provide special zero initialization value literal `__zero_init__`,  it is used in variable definition, it automatically initializes variables to zero, preventing issues related to uninitialized data.

```rust
// zero_init.ca
struct Bunch (i32, i32, *usize, f64, *char);
fn main() {
    let c: i32 = __zero_init__;
    let f: f64 = __zero_init__;
    let ptr: *i32 = __zero_init__;
    let arr: [usize; 10] = __zero_init__;
    let bunch: Bunch = __zero_init__;
    print c; print '\n';
    print f; print '\n';
    print ptr; print '\n';
    print arr; print '\n';
    print bunch;
}
```

Result

```
0
0.000000
(nil)
[0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
Bunch ( 0, 0, (nil), 0.000000, (nil) )
```

When you not use initialize `__zero_init__`, you will get uninitialized value. For example when you not use `__zero_init__` but just defines the variable.

```rust
// non_zero_init.ca
struct Bunch (i32, i32, *usize, f64, *char);
fn main() {
    let c: i32;
    let f: f64;
    let ptr: *i32;
    let arr: [usize; 10];
    let bunch: Bunch;
    print c; print '\n';
    print f; print '\n';
    print ptr; print '\n';
    print arr; print '\n';
    print bunch;
}
```

Result

```
32766
0.000000
0x7ffe57f03ae8
[5225760, 140730373782208, 4638881, 140730373782400, 140730373782400, 140730373782208, 4638948, 453010064, 140730373782400, 140730373782256]
Bunch ( 4581459, 0, 0x7ffe57f03aa8, 0.000000, 0x7ffe57f03ac0 )
```

## 13.5 Address Operations

Allows direct manipulation of memory addresses, which is essential for low-level programming and performance optimization.

## 13.6 Binary Literals

(`0b1010111`) Supports binary literals for clear and efficient representation of binary data in the code.

```rust
// binary_literal.ca
let num1: i32 = 0b01111111111111111111111111111111;
let num2: u64 = 0b1111111111111111111111111111111111111111111111111111111111111111;

print num1; print '\n';
print num2; print '\n';
```

Result

```
2147483647
18446744073709551615
```



## 13.7 typeof

Provides a way to get the type of a variable or expression, and the type then can be used to define variables make it convenient for programming.

```rust
// typeof.ca
fn main() {
    let c = 100.1;

    // print the type information
    dbgprinttype(typeof(c));

    // use typeof to define a variable with the same type as c
    let d: typeof(c);

    // assign value to d
    if (c != 0.0) {
	d = 200.322;
    } else {
	d = c;
    }
    print c; print '\n';
    print d;
}
```

Result

```
size = 8, type: t:f64
100.100000
200.322000
```

## 13.8 Print Types Information

`dbgprinttype` statement allows output of the types of variables or expressions, useful for debugging and type verification.

```rust
let u = 3usize;
dbgprinttype(usize);
dbgprinttype(typeof(u));
```



## 13.9 Range Operator

Useful for creating ranges that can be matched in array patterns. For example, `let a = 2..6` defines a range from 2 to 6.

```rust
fn main() {
	for (i in 1..=9) {
		let i = 2 + i;
		print i; print ' ';
	}
	print '\n';
}
```

## 13.10 Flexible Range Types

Allows ranges to include any type, not just integers. For instance, `let a = (100, 'A') ..;` defines a range starting from a tuple.

## 13.11 Boxing

Supports boxing of values, which enables dynamic typing and heap allocation of values.

## 13.12 Scope-Based Type Definitions

Types can be defined in any scope, providing flexibility in organizing and managing types.

```rust
// scope_types.ca
struct TT {
}

impl TT {
    fn test_self(self) {
		TT::test();
    }

    fn test() {
        struct AA {
            a: i32,
            b: char,
        }

        impl AA {
            fn output(self) {
            fn print_self(s: *AA) {
                print *s; print '\n';
            }

            print_self(self);
            struct CC {
                f1: *AA,
            }

            impl CC {
                fn new(f: *AA) -> CC {
                	return CC {f1: f};
                }

                fn print_AA(self) {
                	print *self->f1; print '\n';
                }
            }

            let cc = CC::new(self);
            	cc.print_AA();
            }
        }

        if (true) {
            struct BB {
            	f1: AA
            }

            let b = BB{f1: AA{a: 0x22222222, b: 'A'}};
            b.f1.output();
        }
    }
}

fn main() {
    TT::test();

    let tt = TT{};
    tt.test_self();
}
```



## 13.13 Arbitrary Dimension Array Initialization

Allows initialization of multi-dimensional arrays in a single statement, with elements of any type.

```rust
// array_init.ca

fn main() {
    // 2D array with integer value
    let arr: [[i32; 3]; 2] = [[1, 2, 3], [4, 5, 6]];

    // 4D array with integer value
    let arr: [[[[i32; 2]; 3]; 2]; 2] = [
	[
	    [[1, 2], [3, 4], [5, 6]],
	    [[7, 8], [9, 10], [11, 12]]
	],
	[
	    [[13, 14], [15, 16], [17, 18]],
	    [[19, 20], [21, 22], [23, 24]]
	]
    ];

    struct Point {
	x: i32,
	y: i32,
    }

    // 2D array with Point structs
    let arr: [[Point; 3]; 2] = [
        [Point { x: 1, y: 2 }, Point { x: 3, y: 4 }, Point { x: 5, y: 6 }],
        [Point { x: 7, y: 8 }, Point { x: 9, y: 10 }, Point { x: 11, y: 12 }]
    ];

    // 4D array of Point structs
    let arr: [[[[Point; 2]; 2]; 2]; 3] = [
        [
            [
                [Point { x: 1, y: 2 }, Point { x: 3, y: 4 }],
                [Point { x: 5, y: 6 }, Point { x: 7, y: 8 }]
            ],
            [
                [Point { x: 9, y: 10 }, Point { x: 11, y: 12 }],
                [Point { x: 13, y: 14 }, Point { x: 15, y: 16 }]
            ]
        ],
        [
            [
                [Point { x: 17, y: 18 }, Point { x: 19, y: 20 }],
                [Point { x: 21, y: 22 }, Point { x: 23, y: 24 }]
            ],
            [
                [Point { x: 25, y: 26 }, Point { x: 27, y: 28 }],
                [Point { x: 29, y: 30 }, Point { x: 31, y: 32 }]
            ]
        ],
        [
            [
                [Point { x: 17, y: 18 }, Point { x: 19, y: 20 }],
                [Point { x: 21, y: 22 }, Point { x: 23, y: 24 }]
            ],
            [
                [Point { x: 25, y: 26 }, Point { x: 27, y: 28 }],
                [Point { x: 29, y: 30 }, Point { x: 31, y: 32 }]
            ]
        ]
    ];

    // with array elements
    struct PointVector {
		x: [i32; 5],
		y: [i32; 5],
    }

    // 2D array with PointVector structs
    let arr: [[PointVector; 2]; 3] = [
        [
            PointVector { x: [1, 2, 3, 4, 5], y: [2, 3, 4, 5, 6] },
            PointVector { x: [3, 4, 5, 6, 7], y: [4, 5, 6, 7, 8] }
        ],
        [
            PointVector { x: [5, 6, 7, 8, 9], y: [4, 5, 6, 7, 8] },
            PointVector { x: [6, 7, 8, 9, 10], y: [1, 2, 3, 4, 5] }
        ],
        [
            PointVector { x: [4, 5, 6, 7, 8], y: [5, 6, 7, 8, 9] },
            PointVector { x: [1, 2, 3, 4, 5], y: [6, 7, 8, 9, 10] }
        ]
    ];
}
```



## 13.14 Arbitrary Object Assignment

Facilitates assignment of objects by copying memory directly, ensuring efficient handling of data.

```rust
// object_assign.ca
struct AComplexType {
    slot: Point,
    batch: PointVector,
    convert: PointConvert,
    area: (f64, f64)
}

impl AComplexType {
    fn new() -> AComplexType {
	return AComplexType {
	    slot: Point {
            x: 100,
            y: 150
	    },
	    batch: PointVector {
            vx: [2,3,4,5,6],
            vy: [3,2,3,4,1]
	    },
	    convert: PointConvert {
            pt: [
                Point {x: 100, y: 300},
                Point {x: 300, y: 200},
                Point {x: 300, y: 300},
                Point {x: 500, y: 400},
                Point {x: 600, y: 100}
            ]
	    },
	    area: (30.4, 9.9)
	};
    }
}

struct Point {
    x: i32,
    y: i32,
}

struct PointVector {
    vx: [i32; 5],
    vy: [i32; 5],
}

struct PointConvert {
    pt: [Point; 5]
}

fn main() {
    // create a new object
    let obj1 = AComplexType::new();
    
    // assign object with value
    let obj2 = obj1;
    print "address, obj1: "; print &obj1; print '\n';
    print "address, obj2: "; print &obj2; print "\n\n";
    print obj1; print "\n\n";
    print obj2;
}
```

Run result:

```
address, obj1: 0x7ffd70e8e628
address, obj2: 0x7ffd70e8e690

AComplexType { slot: Point { x: 100, y: 150 }, batch: PointVector { vx: [2, 3, 4, 5, 6], vy: [3, 2, 3, 4, 1] }, convert: PointConvert { pt: [Point { x: 100, y: 300 }, Point { x: 300, y: 200 }, Point { x: 300, y: 300 }, Point { x: 500, y: 400 }, Point { x: 600, y: 100 }] }, area: ( 30.400000, 9.900000 ) }

AComplexType { slot: Point { x: 100, y: 150 }, batch: PointVector { vx: [2, 3, 4, 5, 6], vy: [3, 2, 3, 4, 1] }, convert: PointConvert { pt: [Point { x: 100, y: 300 }, Point { x: 300, y: 200 }, Point { x: 300, y: 300 }, Point { x: 500, y: 400 }, Point { x: 600, y: 100 }] }, area: ( 30.400000, 9.900000 ) }
```



## 13.15 for .. in .. iterater with Pointers

Supports iteration over items in a collection, and get the pointer to the iterating item, make the ability to work with item pointer directly.

```rust
// forptr.ca
struct Point {
    x: i32,
    y: i32,
}

fn main() {
    // create an array of Point object
    let pt = [
        Point {x: 100, y: 300},
        Point {x: 300, y: 200},
        Point {x: 300, y: 300},
        Point {x: 500, y: 400},
        Point {x: 600, y: 100}
    ];

    // item hold the address of items in pt, so item is a pointer
    for (*item in pt) {
        // print type of item, it should be a pointer to Point object
        dbgprinttype(typeof(item));
        
        // print the item, it's an address
        print item; print ": ";
        
        // dereference and print the object
        print *item; print "\n\n";
    }
}
```

Run result:

```
size = 8, type: t:*{Point;x:i32,y:i32}
0x7fff79c9b090: Point { x: 100, y: 300 }

size = 8, type: t:*{Point;x:i32,y:i32}
0x7fff79c9b098: Point { x: 300, y: 200 }

size = 8, type: t:*{Point;x:i32,y:i32}
0x7fff79c9b0a0: Point { x: 300, y: 300 }

size = 8, type: t:*{Point;x:i32,y:i32}
0x7fff79c9b0a8: Point { x: 500, y: 400 }

size = 8, type: t:*{Point;x:i32,y:i32}
0x7fff79c9b0b0: Point { x: 600, y: 100 }
```



## 13.16 Script Feature

Supports running programs without a main function, executing from the top to bottom of the script. The compiler constructs a main function internally and runs the script as a whole.

```rust
// fib1.ca
fn fibonacci(index: i32) -> u64 {
    if (index == 0 || index == 1) {
	return 1;
    }

    let a = 1u64;
    let b = 1u64;
    let c = 0;
    let t = 0u64;
    index -= 1;
    while (c < index) {
	t = a;
	a = b;
	b = t + a;
	c = c + 1;
    }

    return b;
}

print fibonacci(0); print '\n';
print fibonacci(1); print '\n';
print fibonacci(2); print '\n';
print fibonacci(3); print '\n';
print fibonacci(10); print '\n';
print fibonacci(30); print '\n';
print fibonacci(40); print '\n';
```

Run result:

```
$ ca -main fib1.ca
1
1
2
3
89
1346269
165580141
```

In the source code `fib1.ca`, there is no `main` function defined. However, you can still run it using the command line option `-main`. This option adds an internal `main` function which includes all logics from the source code and executes it, similar to how a script would run.

## 13.17 Goto Statement

Provides the ability to jump to different parts of the code, useful for certain control flow scenarios.

## 13.18 Comprehensive Operators

Includes all arithmetic, logical, and bitwise operators, similar to those found in Rust.

## 13.19 Destructuring Assignment

Allows matching and extracting values from arrays and tuples, such as `let [f1, f2, _, ..] = [1, 2, 3, 4, 5];`.

# TODO

Here is the TODO List which listing the future works for the CA language.

- Runtime libraries that support String type, and memory management etc.
- Generic programming
- trait object, polymorphism
- enumerate type (or tagged union type)
- union type
- function pointer
- packages
- Tools for project management, compile unit, interfaces between compile unit
- match statement
- pass external libraries to CA compiler
- `/* ... */` style comment



