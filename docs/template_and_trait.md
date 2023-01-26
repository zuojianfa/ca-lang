How to handle the template skeleton type and the trait or struct Self representer?

Because the trait self and Self have the similar functionality as the template parameter, so the processing of them are also similar, so put them here together for designing.

# 
```
fn add<I, R>(a: I, b: I) -> R {
	
}
```

# about symbol table
## for trait default method
the default implementation of trait default method uses 2 different symbol tables
- the `self` or `Self` using the symbol table in the position of the type definition of the real `Self`
- the other logic will use the symbol table in the position of the trait definition

for example, it will report error, when `output` function defined in `main` function which cannot be seen by the trait method with default implementation (copy). When `output` defined in global position, will no error occurs because they can be seen by the trait method.

```rust
trait TT {
    fn abc(self) -> Self
	where Self: Sized + std::ops::Add<Output=Self> + Clone 
	{
        output();
        self.clone() + self
    }
}

#[derive(Debug, Clone)]
struct AA {
    pub v: i32,
}

impl std::ops::Add for AA {
    type Output = AA;
    fn add(mut self, rhs: Self) -> Self {
        self.v += rhs.v;
        self
    } 
}

fn output() {
	println!("outter output");
}
    
impl TT for AA {
}

fn main() {
	/* will report error when the output function defined here, although output can be seen from the symbol table of implementation of `TT`
    fn output() {
        println!("outter output");
    }
    
    impl TT for AA {
    }
	*/
    
    let a = AA{v: 3};
    let b = AA{v: 5};
    let v2 = a + b;
    println!("v2: {:?}", &v2);
    let v = v2.abc();
    println!("v: {:?}", v);
}
```
### Note
For `self` or `Self` in struct implementation, they using the same symbol table of struct (function) definition, so it can insert a type alias of `Self = <Struct_Type>` in the method parameter symbol table or body symbol table of struct (function) definition.
it cannot insert type alias into trait definition, because all implementation for the trait share the same symbol table.

The search of symbols must choose which symbol table will used, for `Self` type the struct (function) definition table will be used, else the others will be used.

So here must record a list to record the generic types (here is `Self` for trait, or generic type for generic functions) which will look up the struct table.

## for generic function definition
Like trait definition, the generic function definition also uses 2 different symbol tables
- the generic type - from example, T, U, V, ... - using the symbol table in the position of generic function call
- the other logic will use the symbol table in the position of the generic function definition

```
fn add_aa<T>(a: T, b: T) -> T
where T: Sized + std::ops::Add<Output=T> + Clone
{
        output();
        a + b
}

#[derive(Debug, Clone)]
struct AA {
    pub v: i32,
}

impl std::ops::Add for AA {
    type Output = AA;
    fn add(mut self, rhs: Self) -> Self {
        self.v += rhs.v;
        self
    } 
}

fn output() {
    println!("outter output");
}

fn main() {
    // fn output() {
    //     println!("outter output");
    // }

    let a = AA{v: 3};
    let b = AA{v: 5};
    let v2 = add_aa(a, b);
    println!("v2: {:?}", &v2);
}
```

### Note
Each call to the generic function, will instance a conconrete function (of course can use a cache for accelating), an copy of ASTNode of the generic function implementation is need in the expandition of generic function.

Just like the condition for trait method, the generic type in generic function / method have the similiar situation. it can insert a type alias of `Self = <Struct_Type>` in the caller expand position.

For generic types in struct implementation, they using the same symbol table of struct (function) definition, so it can insert a type alias of `Self = <Struct_Type>` in the method parameter symbol table or body symbol table of struct (function) definition.
it cannot insert type alias into trait definition, because all implementation for the trait share the same symbol table.

The search of symbols must choose which symbol table will used, for generic type the struct (function) definition table will be used, else the others will be used.

So here should provide a list to record the generic types (the generic type here is for generic functions) which will look up the struct table

