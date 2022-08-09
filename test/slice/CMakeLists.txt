
set(test_case_seq 1)
do_test(slice "211232223232232" ca cmp1.ca)
do_test(slice "1\nsize = 1, type: t:bool" ca cmp2.ca)
do_test(slice "a > b" ca cmp3.ca)

