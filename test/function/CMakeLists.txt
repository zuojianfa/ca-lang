set(test_case_seq 1)
do_test(function "21" ca fn_param_array1.ca)
do_test(function "21\nAA { f1: 12345, f2: 67890, af3: \\[7768, 8677\\] }\n12345 67890 7768 8677 -1\n" ca fn_param_pointer1.ca)
do_test(function "21\nAA { f1: 12345, f2: 67890, af3: \\[7768, 8677\\] }\n12345 67890 7768 8677 -1\n" ca fn_param_struct1.ca)
do_test(function "AA { f1: 2022, f2: \\[3.310000, 17.360000\\] }\nAA { f1: 2022, f2: \\[3.310000, 22.090000\\] }" ca fn_return_struct.ca)

