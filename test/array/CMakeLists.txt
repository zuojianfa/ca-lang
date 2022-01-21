set(test_case_seq 1)
do_testf(array "array.ca.result" "array.ca.result.tmp" ca array.ca array.ca.result.tmp)
do_testf(array "array_2sideinfer.ca.result" "array_2sideinfer.ca.result.tmp" ca array_2sideinfer.ca array_2sideinfer.ca.result.tmp)
do_test(array "array element type not identical with variable type: `f64` != `f32`" ca array_error1.ca)
do_test(array "array element type not identical with variable type: `f32` != `f64`" ca array_error2.ca)
do_testf(array "array_dbgprint1.ca.result" "array_dbgprint1.ca.result.tmp" ca array_dbgprint1.ca array_dbgprint1.ca.result.tmp)
do_testf(array "array2string.ca.result" "array2string.ca.result.tmp" ca array2string.ca array2string.ca.result.tmp)
do_test(array "determined array size `10` not match the expression type `11`" ca array2stringbad1.ca)
do_test(array "determined array size `12` not match the expression type `11`" ca array2stringbad2.ca)
do_testf(array "array2pointer.ca.result" "array2pointer.ca.result.tmp" ca array2pointer.ca array2pointer.ca.result.tmp)
do_test(array "[\[]1, 2, 3\[\]]\n\[\[]4, 5, 6[\]]\n0x.*\n0x.*" ca array_assign.ca)
do_test(array "[\[]1, 2, 3\[\]]\n\[\[]4, 5, 6[\]]\n[\[][\[]1, 2, 3[\]], [\[]4, 5, 6[\]][\]]" ca array_var.ca)
do_test(array "[\[]1, 3, 5[\]]\n[\[][\[]1, 2, 3[\]], [\[]4, 5, 7[\]][\]]\n[\[][\[]1, 1, 2[\]], [\[]3, 4, 10[\]][\]]" ca array_var_assign.ca)




