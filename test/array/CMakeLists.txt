set(test_case_seq 1)
do_testf(array "array.ca.ll" "array.ca.ll.tmp" ca -ll array.ca array.ca.ll.tmp)
do_testf(array "array_2sideinfer.ca.ll" "array_2sideinfer.ca.ll.tmp" ca -ll array_2sideinfer.ca array_2sideinfer.ca.ll.tmp)
do_test(array "conflicting of determining literal type: literal already have a type `f64`, cannot determine into `f32`" ca array_error1.ca)
do_test(array "conflicting of determining literal type: literal already have a type `f32`, cannot determine into `f64`" ca array_error2.ca)


