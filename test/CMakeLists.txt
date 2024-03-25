macro (add_test_exec exec_name)
  add_executable("${exec_name}" "${exec_name}.cc")
  target_link_libraries("${exec_name}" PRIVATE cbt_lib)
endmacro (add_test_exec)

add_test_exec(cpr_test)
