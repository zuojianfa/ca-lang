
set(test_case_seq 1)
do_test(cmp "211232223232232" ca cmp1.ca)
do_test(cmp "1\nsize = 1, type: t:bool" ca cmp2.ca)
do_test(cmp "a > b" ca cmp3.ca)

