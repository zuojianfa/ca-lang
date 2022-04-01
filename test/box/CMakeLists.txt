set(test_case_seq 1)
do_test(box "2022" ca box1.ca)
do_test(box "2 2022" ca box2.ca)
do_test(box "302\nHello CA!" ca box3.ca)
do_test(box "\\[2, 0, 2, 2, 0, 3, 3, 0\\]\n20220330" ca box4.ca)
do_test(box "AA { f1: 2022, f2: 2022.033000 }\n2022 2022.033000\n2023\n" ca box5.ca)
do_test(box "\\[AA { f1: 2022, f2: 2022.033000 }, AA { f1: 2, f2: 2.200000 }\\]\nAA { f1: 2022, f2: 2022.033000 }\n2022 2022.033000\n20232023 2022.033000 2 2.200000" ca box6.ca)
do_test(box "good" ca box_scope1.ca)

set(test_case_seq 1)
do_test(drop "2022" ca drop1.ca)

