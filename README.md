# The CA Programming Language

## 🌟 About

CA is a programming language which has similar grammars as rust language, similar to C language in many features, compatible with C ABI, support calling C functions directly without any matching and transformation. It uses Mark-and-Sweep style garbage collections for it's memory management.

CA compiler uses LLVM as its backend, and uses JIT or AOT technology in LLVM to run program directly without compile. Currently, CA compiler is under development, it has already realized most of grammars, but still have a lot of works to do. 

For language details see the book: [CA book](book/ca-book.md).

## ⚡ Quick Code

### Getting Started

Here’s a simple example to help you get started quickly:

```rust
// Example of basic functionality
fn main() {
    let greeting = "Hello, CA!\n";
    print(greeting);
}
```

### Key Features

#### Function Definition

```rust
fn add(a: i32, b: i32) -> i32 {
    return a + b;
}
```

### Advanced Example

Here’s a more advanced code snippet showcasing a feature:

```rust
// quickcode.ca

// introduce external function
extern fn printf(fmt: *char, ...) -> i32;

fn fibonacci_at(index: i32) -> u64 {
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

struct Integer {
    value: i64
}

impl Integer {
    fn new(value: i64) -> Integer {
		return Integer {value};
    }

    fn fibonacci(self) -> u64 {
		return fibonacci_at(self->value as i32);
    }
}

fn main() -> i32 {
    // define variable, whose type is inferred from the right-hand side value
    let f10 = fibonacci_at(10);
    printf("fibonacci number at 10 is: %llu\n", f10);
    
    // print the type information of variable f10
    print "return type is: ";
    dbgprinttype(typeof(f10));

    // create an Integer instance
    let v1 = Integer::new(30);
    printf("\nfibonacci number at 30 is: %llu\n", v1.fibonacci()); 
    
    print "return type is: ";
    dbgprinttype(typeof(v1));
    return 0;
}
```

Run result:

```
fibonacci number at 10 is: 89
return type is: size = 8, type: t:u64

fibonacci number at 30 is: 1346269
return type is: size = 8, type: t:{Integer;value:i64}
```

## 🌟 Features

- **CA is static type language**
- **Supports optimization through LLVM IR.**
- **Supports JIT functionality that run from source file directly**
- **Supports compile CA source file into native executable file: ELF file on linux**
- **Supports compile CA source file into readable IR assembly file `.ll`**
- **Supports compile CA source file into native (as) assembly file: `.s`**
- **Supports debug with gdb debugger**
- **The object file compatible with C ABI**
- **Internal Support standard C library functions**
- **Supports garbage collection feature using GC**
- **Supports run as script feature, with `-main` option**
- **Supports generate `.dot` file to show the graph of grammar tree**
- **Supports llvm12 or llvm13 library**
- **Supports compile unit which adhering with C ABI**
- **Include the whole test cases in the source code**

## 📜 Supported Grammars

The grammar of CA language is similar to rust language in most cases. For the details see [CA book](book/ca-book.md). Here list some of the special grammars in CA.

- **Support pointer like in C language**
- **Support array convert to pointer**
- **Support char array convert into string type**
- **Support `print` or `dbgprint`statement for debugging purpose, which can print any value**
- **Support `dbgprinttype` statement which is used to print the type informations**
- **Support `typeof` statement to get the type of a variable dynamically, also support and then use the type to define another variable**
- **The CA variable is mutable**
- **Support binary number literals**
- **Support zero Initialization keyword `__zero_init__`**
- **support `box`, `drop` grammar for memory management**
- **support pointer in `for .. in` statement**
- **support `goto` statement**

## 🛡️ License
This project is licensed under the Mulan PSL v2 License. You may obtain a copy of the license at http://license.coscl.org.cn/MulanPSL2.

## ✍️ Author

​	**Rusheng Xia**

- **Organization:** Rocket Software
- **GitHub:** [github.com/zuojianfa](https://github.com/zuojianfa)
  Explore my projects and contributions!
- **LinkedIn:** [linkedin.com/in/zuojianfa](https://www.linkedin.com/in/zuojianfa)
  Connect with me for professional networking!

## 📫 Contact Information

Have questions, suggestions, or want to contribute? Feel free to reach out!

- **Email:** [xrsh_2004@163.com](mailto:xrsh_2004@163.com)

Looking forward to hearing from you!

## 🚀 Roadmap for the Language

### Core Language Features

- **Support for String Type in Language Level**
- **Generic Programming**
- **Union Types**
- **Enumerated Types (Tagged Union Types)**
- **Function Pointers**
- **Module System**

### Language Structure and Organization

- **Compile Units Management**.
- **Interfaces between Compile Units**: Establish mechanisms for communication and data sharing between different compile units, crucial for modularity and reusability.
- **External Libraries Interactive**: Providing mechanisms for integrating external libraries can expand the language's capabilities and ecosystem.
- **Package Management Tools**
- **Runtime Libraries**

### Language Features and Constructs

- **Match Statement**
- **Trait Objects and Polymorphism**
- **Multi-line Comments (`/* ... */`)**
