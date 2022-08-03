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
