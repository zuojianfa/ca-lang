set(test_case_seq 1)

do_test(pattern-noinit "1 2 3\n1.100000 2.200000 3.300000" ca let_noinit1.ca)
do_testf(pattern-noinit "let_noinit2.ca.ll" "let_noinit2.ca.ll.tmp" ca -ll let_noinit2.ca let_noinit2.ca.ll.tmp)

