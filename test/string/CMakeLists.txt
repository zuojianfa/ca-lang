set(test_case_seq 1)
do_test(string "Hello CA!\nabcd[\\]tdef=====123" ca cstring.ca)
do_test(string "abcd	def=====333\naaa: 1\nbbb: 2\nccc: a\nddd: abcd	def\nn: 35\n\naaa: 1\n	bbb: 2\n	ccc: a\n	ddd: abcd	def\n	n: 39\nabcd	def=====123\nabcd[\\]tdef=====123[\\]n\ndddd <===> 321\nHello CA!\nHello S2\nabcd[\\]tdef\nHello S2\n\n\nabcd[\\]tdefabc\n\nHello S2abc\n\nabcd[\\]tdef\n\nHello S2\n\n\nreturn value: 99\n" ca cstring2.ca)
do_test(string "Hello[\\]t[\\]r[\\]nCA!\nHello[\\]tCA! r\"ABCD\" \"r \nHello\n		[\\]t[\\]r[\\]nCA!\nHello[\\]tCA!\n		 r\"ABCD\" \"r \n\nreturn value: 26\n" ca cstring3.ca)
do_test(string "                A\n              / | \\n            B   C   D\n           /  / |  / \\n          E  F  G H   I\n.*                A\n              / | \\n            B   C   D\n           /  / |  / \\n          E  F  G H   I\n" ca cstring4.ca)
do_test(string "hello \nworld!\nhello world!\nworld!\n" ca cstring5.ca)

