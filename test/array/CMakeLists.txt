set(test_case_seq 1)
do_testf(array "array.ca.ll" "array.ca.ll.tmp" ca -ll array.ca array.ca.ll.tmp)
do_testf(array "array_2sideinfer.ca.ll" "array_2sideinfer.ca.ll.tmp" ca -ll array_2sideinfer.ca array_2sideinfer.ca.ll.tmp)
do_test(array "conflicting of determining literal type: literal already have a type `f64`, cannot determine into `f32`" ca array_error1.ca)
do_test(array "conflicting of determining literal type: literal already have a type `f32`, cannot determine into `f64`" ca array_error2.ca)
do_testf(array "array_dbgprint1.ca.result" "array_dbgprint1.ca.result.tmp" ca array_dbgprint1.ca array_dbgprint1.ca.result.tmp)
do_testf(array "array2string.ca.result" "array2string.ca.result.tmp" ca array2string.ca array2string.ca.result.tmp)
do_test(array "expected an array with a fixed size of 10 elements, found one with 11 elements" ca array2stringbad1.ca)
do_test(array "expected an array with a fixed size of 12 elements, found one with 11 elements" ca array2stringbad2.ca)
do_testf(array "array2pointer.ca.result" "array2pointer.ca.result.tmp" ca array2pointer.ca array2pointer.ca.result.tmp)
# array_var.ca


