
set(test_case_seq 1)

# define a function to check if the running result is right
function(do_test prefix result cmd args)
  math(EXPR argc "${ARGC} - 1" OUTPUT_FORMAT DECIMAL)
  set(argl ${ARGV${argc}})
  # if(ARGC GREATER 3)
  #   list(SUBLIST ARGV 3 ${ARGC} arglist)
  # endif()

  set(testname ${prefix}${test_case_seq}-${argl})

  add_test(NAME ${testname} COMMAND ${cmd} ${args} ${ARGN})
  set_tests_properties(${testname} PROPERTIES
    PASS_REGULAR_EXPRESSION ${result}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
  math(EXPR test_case_seq "${test_case_seq} + 1" OUTPUT_FORMAT DECIMAL)
  set(test_case_seq ${test_case_seq} PARENT_SCOPE)
endfunction()

function(do_testf prefix samplef resultf cmd args)
  math(EXPR argc "${ARGC} - 1" OUTPUT_FORMAT DECIMAL)
  set(argl ${ARGV${argc}})
  set(testname ${prefix}${test_case_seq}-${argl})
  set(auxtestname aux-${testname})
  set(cleantestname clean-${testname})

  set(resultf ${CMAKE_CURRENT_SOURCE_DIR}/${resultf})
  set(samplef ${CMAKE_CURRENT_SOURCE_DIR}/${samplef})

  add_test(NAME ${auxtestname} COMMAND ${cmd} ${args} ${ARGN})
  set_tests_properties(${auxtestname} PROPERTIES
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
  add_test(NAME ${testname} COMMAND cmp ${samplef} ${resultf})
  set_tests_properties(${testname} PROPERTIES
    DEPENDS ${auxtestname}
    REQUIRED_FILES ${resultf}
    FAIL_REGULAR_EXPRESSION "differ"
    FAIL_REGULAR_EXPRESSION "No such file or directory"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
  add_test(NAME ${cleantestname} COMMAND rm -f ${resultf})
  set_tests_properties(${cleantestname} PROPERTIES DEPENDS ${testname})
  math(EXPR test_case_seq "${test_case_seq} + 1" OUTPUT_FORMAT DECIMAL)
  set(test_case_seq ${test_case_seq} PARENT_SCOPE)
endfunction()

