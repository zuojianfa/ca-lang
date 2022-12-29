set(test_case_seq 1)
do_test(type_impl "good" ca simple.ca)
do_test(type_impl "Hello inner function\nHello outter function\n" ca inner_function.ca)
do_test(type_impl "Hello inner function 1\nHello inner function 2\nHello outter function\n" ca inner_function2.ca)
do_test(type_impl "Hello inner function 1\nHello inner function 2\n" ca inner_function3.ca)
do_testf(type_impl "inner_function3_0.ca.ll" "inner_function3_0.ca.ll.tmp" ca -g -ll inner_function3_0.ca inner_function3_0.ca.ll.tmp)
do_test(type_impl "Hello C A" ca inner_function4.ca)
do_test(type_impl "Hello C A\nin while in while in while in while in while" ca inner_function_more_level.ca)

