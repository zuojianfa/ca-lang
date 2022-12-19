# about the compile pass
1st pass: driven by parser (bison), in this pass, it construct the AST
2ed pass: `walk_tree` function, it iterater the AST and generate the llvm-ir
3rd pass: llvm-ir => ir optimization, or generate native code, or jit running

# when only support global function
so the type that function uses as parameter must be defined globally

## define type, use type && use type, define type
The type is constructed through the analyze of grammar in the first pass, and the use of type will not do checking in the first pass, so it don't care if the type is defined before or after using it. In the second pass (`walk_tree`), it will check the use of types and it's fields.

## define function, use function
1st pass, construct function tree, when use it, the parameter are compared
2ed pass, walk function definition first and after that generate llvm function object, then when use it use the llvm function objectg directly

## use function, define function
1st pass, when use it, record the function pre-usage in the container, do not check parameter, later when encounter the function definition then fill up the container item

between 1st and 2ed pass, walk the pre-usaged function AST Node and generate llvm function object

2ed pass, when call the function then llvm function object already form, so just call it directly, will check the parameter in the call

steps:
1. record unknown function call in global container - 1st pass
2. when encounter function definition or declaration, fetch the item from container with the previous unknown function name and fill with the function definition or declaration ASTNode object - 1st pass
3. walk function definition or declaration in handle_post_functions - between 1st pass and 2ed pass

# when supporting inner function and function defined in lexical scope
## define type, use type && use type, define type
Have no difference with when only allows global functions

## define function, use function
The function definition is in the space of lexical scope, not only in the global, the use of function need looking at the lexical tree structure to find the suitable function
1st pass, construct function tree *in lexcial scope*, when use it, the parameter are compared
2ed pass, walk function definition first and after that generate llvm function object, *the function name using mangled name in order to support inner function and struct method*, then when use it use the llvm function objectg directly

## use function, define function
1st pass, when use it, record the function pre-usage in the container *in lexical scope*, do not check parameter, later when encounter the function definition then fill up the container item, and check the parameter

between 1st and 2ed pass, walk the pre-usaged function AST Node and generate llvm function object

2ed pass, when call the function then llvm function object already form, so just call it directly, will check the parameter in the call


steps:
1. record unknown function call in the container in lexical scope - 1st pass
2. when encounter function definition or declaration, fetch the item from container with the previous unknown function name and fill with the function definition or declaration ASTNode object - 1st pass
3. walk function definition or declaration in handle_post_functions - between 1st pass and 2ed pass


solution 1:
first `walk_tree` for all the functions declaration with mangled name and fill sym entry with mangled name or llvm function object and other things
second `walk_tree` for all elements
for the function call how to get mangled function name?
	1. according to the tree symbol form table find the function sym entry
	2. from the entry get the mangled function name which may be filled in first `walk_tree`
	3. the call function just use the name as the call name, or just need get the llvm function object
	
it may spend more time to compile.

solution 2:
in `walk_tree` only walk the function declaration which is pre-used to shorten time, but how to find which one need walk?



fn_decl

* walk_fn_declare
* walk_fn_define
* walk_while
* walk_if
* walk_stmtlist
* walk_lexical_body
* walk_loop
* walk_for
* walk_fn_define_impl

how to find function?
```
  SymTable *symtable = sym_parent_or_global(p->symtable);
  STEntry *preventry = sym_getsym(symtable, p->fndecln.name, 1);
  typeid_it fnname_full_id = preventry->u.f.mangled_id;
  const char *fnname_full = symname_get(fnname_full_id);
  Function *fn = ir1.module().getFunction(fnname_full);
  auto itr = function_map.find(fnname_full);
  if (itr != function_map.end()) {
    if (!fn) {
      // when consider name, the function map set belongs to ir1 module set
      yyerror("very strange, function must exists in the module");
      return nullptr;
    }

    return fn;
  }

```


1. reset `curr_lexical_count=0`
2. compare `curr_lexical_count` value of 2 walk pass
3. 
