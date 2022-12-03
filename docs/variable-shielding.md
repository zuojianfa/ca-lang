# What's the functionality?
it resolve following issue:
```
let v1 = 1;
let v1 = 3.2;
let v1 = (1, 2);
```
or in match form, the latter a shielded the match expression
```
match (a) {
	(a) => print a,
	_ => 
}
```

The later rebinding variable with the same name as before will shielding the definition of previous defined in either value or types, they are different types

# Principle
The variable name of all the shielding variant uses the same `STEntry` structure, the field `varshielding` of `STEntry` is a list of all shielding variables in the `varlist` field, and `current` field point to current variable variant in using.
```
typedef struct CAVariableShielding {
  CAVariable *current;
  void *varlist;
} CAVariableShielding;
```

When in let or match statement, when doing shielding the function `varshielding_rotate_capattern` is invoked to change `current` variable variant so the latter use of the variable will be the right one after shielding

```
------       -----------       -----------------------      -----------
| V1 | --->  | STEntry | --->  | CAVariableShielding | ---> | current | ---|
------       --^--------       -----------------------   ^  -----------    |
               |                                         |                 |
               |										 |				   |
               |                                         |  -----------    |
               |                                         ---| varlist | <--|
               |                                            -----------
------         |
| V1 |---------|
------         |
               |
------         |
| V1 |---------|
------

```

`void varshielding_rotate_capattern(CAPattern *cap, SymTable *symtable, bool is_back)` function rotate all the variable variant which is defined in pattern `cap: CAPattern`, `is_back` indicate the rotation direction

## Action
The `varlist` of `CAVariableShielding` is registered in the first scan in function `register_variable`, and the `CAVariable` object is appended in the `varlist` in order of the appearance order

have following invoke relationship
```
make_global_vardef
	register_variable


make_let_stmt
	register_capattern_symtable
		capattern_register_variable
			register_variable
```

When only have one variable variant, only `current` have value and the size of `varlist` is `0`, so when encounter another variant, the `current` will append to `varlist` and the new variant will become `current`. So the variable variant count is `vec_size(varlist) + 1`

In the second scan `walk_xxx` serial functions, the shielding variables will travel from beginning of the variant list `varlist` + `current`, so when `walk_letbind` it need perform `varshielding_rotate_capattern` to bring the right `CAVariable` into `current`. Before the first time of travalling, the `current` point to the last variant, so the invoke of `varshielding_rotate_capattern` will bring the first variant into `current`

## avoid conflicting when both side of expression used the same name of variant

use following to protect
```
	varshielding_rotate_capattern(cap, exprn->symtable, true);
    walk_stack(exprn);
    varshielding_rotate_capattern(cap, exprn->symtable, false);

```

When handling beside the first variant the rotation back action will bring the previous (in CAPattern) variant into `current` and `walk_stack` will use the previous defined variant rightly
When handling the first variant the rotation back (with true parameter) will bring the last variant into `current`, is it right? It should have no problem, because when visit (just binding) the first variant, it should not appear in the right side expressoin, or it already blocked in the first scan.


# Rotation As a Whole
When both side used the `CAPattern` pattern object and `ASTNode` expression, it will cannot do rotation directly for both left side pattern and right side expression because they use **different** variant with same **name** of variable.

When doing variable shielding rotation of `CAPattern` for let or match statement, it should always rotate as whole. It exists problems when only rotating child `CAPattern` object. `CAPattern` is a tree form structure, below is the definition
```
typedef struct CAPattern {
  ...
  PatternGroup *items; // vec for CAPattern *
} CAPattern;

typedef struct PatternGroup {
  int size;
  int capacity;
  struct CAPattern **patterns;
} PatternGroup;
```

`cap->items->patterns[i]`

when not rotating `CAPattern` as a whole or rotating only part of child `CAPattern` (for example: in recursive function call), in following condition, it will cannot work properly.
For example:
following `CAPattern` include 2 children `CAPattern` of `b` and `a`, in first step when only rotating (in recursive function call) the child `b` (of `let (b, a)`), it will cannot work
First example: 
```
let a = 1;
let b = 2;
let (b, a) = (a, b);
```

but following can work
Second example: 
```
let a = 1;
let b = 2;
let (a, b) = (a, b);
```

why? for `let (b, a) = (a, b)`
In recursive function call, in sub-step, for example, because of recursive function call, in first step when only rotating `b`, then `b` is rotated into the previous value right and in this time if inference the value or datatype of `b`, it can get the right value and datatype because the variant of `b` already rotated into previous variant whose value and datatype already calculated. But here the `b` is corresponding the expression `a`, so here in recursive function call it need get the value or datatype of `a`, but the problem come, because in this recursive call, it only rotated a child `CAPattern` `b` but another child `CAPattern` `a` is not rotated back to prevouse variant, so variant `a` still point to current capattern variant `a` (in `let (..., a)`), and the variant is under calculating for the value of datatype, so it's value or datatype is undefined (or -1), so when getting value or datatype of `a` of `... = (a, ...)`, it will get the undefined (or -1) value or datatype, it just because the rotation is not as a whole.

The second can work because, the order or variant `a` and `b` happened to be in the same order. So the rotation of `a` in capattern can just be used by the `a` in expression





