set(test_case_seq 1)
do_test(as "55" ca as1.ca)
#do_test(as "line: 3, column: 17, cannot convert `i64` into `bool`" ca as_bool.ca)
# TODO: how to represent the brack () in regular expression
do_test(as "line: 3, col: 17:.*type `i64` cannot convert \\(as\\) to type `bool`" ca as_bool.ca)
#do_test(as "line: 3, column: 17, cannot convert `f32` into `bool`" ca as_bool2.ca)
do_test(as "line: 3, col: 17:.*type `f32` cannot convert \\(as\\) to type `bool`" ca as_bool2.ca)
do_test(as " " ca as_bool3.ca)

#do_test(ca "55" ca as_bool3.ca)
#do_test(ca "55" ca as_bool3.ca)

