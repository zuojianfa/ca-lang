catype_get_by_name
	catype_unwind_type_signature
		catype_unwind_type_signature_inner
			catype_unwind_type_array
			catype_unwind_type_struct
			catype_unwind_type_name

catype_unwind_type_name
	catype_unwind_type_signature_inner

catype_unwind_type_array
	catype_unwind_type_signature_inner

catype_unwind_type_struct
	catype_unwind_type_signature_inner

walk_expr_sizeof
	catype_unwind_type_signature

walk_dbgprinttype
	catype_unwind_type_signature

walk_expr_as
	catype_get_by_name

walk_typedef
	catype_get_by_name

inference_expr_type
determine_expr_type(case TTE_Id)
make_typedef(entry->u.datatype.id = type;)
_*pointer_type
inference_assign_type(check idn)
rettype

