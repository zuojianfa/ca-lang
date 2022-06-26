set(test_case_seq 1)
do_test(range "0..6\n1..7\n22..33\nAA { f1: 2 }..AA { f1: 200 }\nA1 { f1: 2, f2: 3.300000 }..A1 { f1: 200, f2: 44.400000 }\nT1 \\( A1 { f1: 2, f2: 3.300000 }, A1 { f1: 200, f2: 44.400000 } \\)..T1 \\( A1 { f1: 200, f2: 44.400000 }, A1 { f1: 2, f2: 3.300000 } \\)" ca range1.ca)
do_test(range "1..4..6..9\n3..6..8..20\n3..6..8..20..1..4..6..9" ca range2.ca)
do_test(range "0..=6\n1..=7\n22..=33\nAA { f1: 2 }..=AA { f1: 200 }\nA1 { f1: 2, f2: 3.300000 }..=A1 { f1: 200, f2: 44.400000 }\nT1 \\( A1 { f1: 2, f2: 3.300000 }, A1 { f1: 200, f2: 44.400000 } \\)..=T1 \\( A1 { f1: 200, f2: 44.400000 }, A1 { f1: 2, f2: 3.300000 } \\)" ca range3.ca)
do_test(range "line: 10, col: 18:\n    let d = c .. r\\;\n                 ^\nexpect type `i32..i32..i32..i32`, but found `i32..=i32..=i32..=i32`, both side of range need to be same type" ca range_error1.ca)
do_test(range "1..=4..=6..=9\n3..=6..=8..=20\n3..=6..=8..=20..1..=4..=6..=9" ca range4.ca)
do_test(range "good" ca range_full.ca)
do_test(range "good" ca range_half1.ca)

