
# enable testing work
enable_testing()

# following are the testing cases

# verify usage message work
add_test(NAME Usage COMMAND ca)
set_tests_properties(Usage
  PROPERTIES PASS_REGULAR_EXPRESSION ".*-jit"
  )

# verify ca can work for default parameter
add_test(NAME CanRun COMMAND ca ${CMAKE_SOURCE_DIR}/test/goto2.ca)

set(test_case_seq 1)
# define a function to check if the running result is right
function(do_test prefix result cmd args)
  math(EXPR argc "${ARGC} - 1" OUTPUT_FORMAT DECIMAL)
  set(argl ${ARGV${argc}})
  if(ARGC GREATER 3)
    list(SUBLIST ARGV 3 ${ARGC} arglist)
  endif()

  set(testname ${prefix}${test_case_seq}-${argl})

  add_test(NAME ${testname} COMMAND ${cmd} ${arglist})
  set_tests_properties(${testname}
    PROPERTIES PASS_REGULAR_EXPRESSION ${result}
    )
  math(EXPR test_case_seq "${test_case_seq} + 1" OUTPUT_FORMAT DECIMAL)
  set(test_case_seq ${test_case_seq} PARENT_SCOPE)
endfunction()

# batch of test cases
# when exists main function
do_test(t "define void @timestamp()" ca -ll ../test/extern_call1.ca)
do_test(t "from line 3, to line 5: function verify failed: Call parameter type does not match function signature!" ca -ll ../test/extern_printf.ca)
do_test(t "line: 2, col: 40: function 'putchar' parameter number not identical with previous, see: line 0, col 0." ca ../test/extern_multi.ca)
do_test(t "-824944808" ca ../test/fib2.ca)
do_test(t "801767181" ca ../test/fib.ca)
do_test(t "DISubprogram" ca -ll -g ../test/fib2.ca)
do_test(t "1964338849" ca ../test/fib_mvar.ca)
do_test(t "111445263496" ca ../test/fn_param.ca)
do_test(t "2011114452634961445263496" ca ../test/fn_param2.ca)
do_test(t "2011114452634960" ca ../test/fn_param2_comment.ca)
do_test(t "40" ca ../test/fn_param3.ca)
do_test(t "12345678910111213" ca -main ../test/nomain/fn_recursive1.ca)
do_test(t "12345678910111213141516171819200.000000" ca ../test/fn_recursive2.ca)
do_test(t "49" ca ../test/fn_recursive3.ca)
do_test(t "2" ca ../test/goto.ca)
do_test(t "-1186666367" ca ../test/goto2.ca)
do_test(t "1115000000" ca -O2 ../test/goto3.ca)
do_test(t "8000000" ca ../test/goto4.ca)
do_test(t "41" ca ../test/ife.ca)
do_test(t "1HL" ca ../test/if_expr.ca)
#do_test(t "1HL" ca ../test/if_stmt.ca)
do_test(t "1010100331" ca ../test/recur_scope2.ca)
do_test(t "131" ca ../test/recur_scope3.ca)
do_test(t "program.*return.ca.*return value: 0" ca ../test/return.ca)
do_test(t "7.900000\n7.400000" ca ../test/type2.ca)
do_test(t "2.30000011.100000" ca ../test/type3.ca)
do_test(t "13.500000.*57.830000.*13.500000.*57.830000.*1.*2.*3.*4.*5.*6.200000.*7.300000.*-2.*-3.*1.*2.*3.*4.*5.*6.200000.*7.300000.*-2.*-3.*1.*-432432.*9223372036854775807.*4000000000.*18446744073709551610.*6.200000.*7.300000.*-2147483648.*-9223372036854775808.*1.*2.*3.*4.*5.*6.200000.*7.300000.*-2147483648.*-9223372036854775808.*.*return value: 0" ca ../test/type4.ca)
do_test(t "10.000000.*79.800000.*57.830000.*2.170000.*50.530000.*12.340000.*.*return value: 0" ca ../test/type5.ca)
do_test(t "6.200000\n13.500000" ca ../test/type6.ca)
do_test(t "32.*344.343231.*3.123000.*A.*1.*323.122300.*.*return value:.*" ca ../test/typed_var1.ca)

# when not exist main function
set(test_case_seq 1)
do_test(nomain " " ca ../test/nomain/0.ca)
do_test(nomain " " ca ../test/nomain/1.ca)
