set(test_case_seq 1)
do_testf(struct "struct_print1.ca.result" "struct_print1.ca.result.tmp" ca struct_print1.ca struct_print1.ca.result.tmp)
#do_testf(struct "struct_print2.ca.result" "struct_print2.ca.result.tmp" ca struct_print2.ca struct_print2.ca.result.tmp)
do_testf(struct "struct_print3.ca.result" "struct_print3.ca.result.tmp" ca struct_print3.ca struct_print3.ca.result.tmp)
do_testf(struct "struct_print4.ca.result" "struct_print4.ca.result.tmp" ca struct_print4.ca struct_print4.ca.result.tmp)
do_testf(struct "struct_print5.ca.result" "struct_print5.ca.result.tmp" ca struct_print5.ca struct_print5.ca.result.tmp)
do_test(struct "AA { n1: 111, n2: 222, n3: 333 }\n111 222 333 \nA2 { a: 32, b: 64, c: 32, d: 32.220001, e: 64.640000, f: 1, g: A, h: 3 }\nA4 { aa: 88, b: 0x32, c: 0x3232, d: 0x123453232, h: 0x123453232 }\nA4 { aa: 88, b: 0x32, c: 0x3232, d: 0x123453232, h: 0x133453232 }\n0x3232" ca struct_field1.ca)
do_test(struct "good" ca struct_field2.ca)

