set(test_case_seq 1)
do_test(pattern "\\( 0, \\( 0, 0 \\) \\)\n0 0\n1 2\n0 0\n\\( 0, 0 \\) \\( 1, 2 \\)\n\\( 0, 0 \\)\n0 0" ca let0.ca)
do_test(pattern "A1 { f1: 0 }\nA1 { f1: 0 }\nA1 { f1: 3 }\nA2 { f1: A1 { f1: 0 }, f2: 0 }\nA3 { f1: A1 { f1: 0 }, f2: A2 { f1: A1 { f1: 0 }, f2: 0 }, f3: 0, f4: 0 }\nA3 { f1: A1 { f1: 0 }, f2: A2 { f1: A1 { f1: 0 }, f2: 0 }, f3: 33, f4: 0 }\n2 0 0\nA1 { f1: 0 }\nA1 { f1: 0 } 0\nA2 { f1: A1 { f1: 0 }, f2: 0 }\n==\nA1 { f1: 0 }\nA2 { f1: A1 { f1: 0 }, f2: 0 }\n0 0\nA3 { f1: A1 { f1: 0 }, f2: A2 { f1: A1 { f1: 0 }, f2: 0 }, f3: 0, f4: 0 }" ca let01.ca)


