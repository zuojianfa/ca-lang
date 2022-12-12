# mangling & demangling
when using mod or struct method it need mangling for the name of function, or may be for the mod itself

- [x] for rust when using #[no_mangle] for function, it will ignore the mod name, and in different mod define the function with the same name is not allowed

## mangle
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

### module mangling
using `$` as the seperator of the type

### struct mangling

### function mangling

### trait mangling


