set(test_case_seq 1)
do_test(pattern "\\( 0, \\( 0, 0 \\) \\)\n0 0\n1 2\n0 0\n\\( 0, 0 \\) \\( 1, 2 \\)\n\\( 0, 0 \\)\n0 0" ca let0.ca)
do_test(pattern "A1 { f1: 0 }\nA1 { f1: 0 }\nA1 { f1: 3 }\nA2 { f1: A1 { f1: 0 }, f2: 0 }\nA3 { f1: A1 { f1: 0 }, f2: A2 { f1: A1 { f1: 0 }, f2: 0 }, f3: 0, f4: 0 }\nA3 { f1: A1 { f1: 0 }, f2: A2 { f1: A1 { f1: 0 }, f2: 0 }, f3: 33, f4: 0 }\n2 0 0\nA1 { f1: 0 }\nA1 { f1: 0 } 0\nA2 { f1: A1 { f1: 0 }, f2: 0 }\n==\nA1 { f1: 0 }\nA2 { f1: A1 { f1: 0 }, f2: 0 }\n0 0\nA3 { f1: A1 { f1: 0 }, f2: A2 { f1: A1 { f1: 0 }, f2: 0 }, f3: 0, f4: 0 }" ca let01.ca)
do_test(pattern "A1 { f1: 0 }\nA1 { f1: 0 }\nA1 { f1: 3 }\nA2 { f1: A1 { f1: 0 }, f2: 0 }\nA3 { f1: A1 { f1: 0 }, f2: A2 { f1: A1 { f1: 0 }, f2: 0 }, f3: 0, f4: 0 }\nA3 { f1: A1 { f1: 0 }, f2: A2 { f1: A1 { f1: 0 }, f2: 0 }, f3: 33, f4: 0 }\n2 0 0\nA1 { f1: 0 }\nA1 { f1: 0 } 0\nA2 { f1: A1 { f1: 0 }, f2: 0 }\n==\nA1 { f1: 0 }\nA2 { f1: A1 { f1: 0 }, f2: 0 }\n0 0\nA3 { f1: A1 { f1: 0 }, f2: A2 { f1: A1 { f1: 0 }, f2: 0 }, f3: 0, f4: 0 }" ca let02.ca)
do_test(pattern "0 0 C 0.000000 A\nAA { f1: 0, f2: 0, f3: C, f4: 0.000000, f5: A }\n0 0 C 0.000000 A\nAA { f1: 0, f2: 0, f3: C, f4: 0.000000, f5: A }\n0 0\nAA { f1: 0, f2: 0, f3: D, f4: 0.000000, f5: E }\nH A\nAA { f1: 0, f2: 0, f3: R, f4: 0.000000, f5: U }" ca let03.ca)
do_test(pattern "0 0 C 0.000000 A\n0 0 D 0.000000\n0 0\n0 B\n0.000000 Q P 0\nBB \\( 0, 0, R, 0.000000, S \\)" ca let04.ca)
do_test(pattern "0 C A\n0 D 0.000000\n0 0\n0 B\n0.000000 Q P 0\nBB \\( 0, 0, R, 0.000000, S \\)" ca let05.ca)
do_test(pattern "0 C A\n6 A\n0 0.000000\nBB \\( 0, 0, F, 0.000000, L \\)\nC0 Z0 B\nQ P 0\nBB \\( 0, 0, R, 0.000000, S \\)" ca let06.ca)
do_test(pattern "0 0 C 0.000000 A\n0 0 D 0.000000\n0 0\n0 B\n0.000000 Q P 0\nBB \\( 0, 0, R, 0.000000, S \\)" ca let07.ca)


