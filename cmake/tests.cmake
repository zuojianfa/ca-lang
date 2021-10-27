
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
function(do_test testname result cmd args)
  if(ARGC GREATER 3)
    #message("ARGC value is: " ${ARGC})
    #message("ARGV value is: " ${ARGV})
    list(SUBLIST ARGV 3 ${ARGC} arglist)
  endif()

  add_test(NAME ${testname}${test_case_seq} COMMAND ${cmd} ${arglist})
  set_tests_properties(${testname}${test_case_seq}
    PROPERTIES PASS_REGULAR_EXPRESSION ${result}
    )
  math(EXPR test_case_seq "${test_case_seq} + 1" OUTPUT_FORMAT DECIMAL)
  set(test_case_seq ${test_case_seq} PARENT_SCOPE)
endfunction()

# batch of test cases
do_test(t "1234086232" ca ../test/goto2.ca)
do_test(t "DISubprogram" ca -ll -g ../test/fib2.ca)

