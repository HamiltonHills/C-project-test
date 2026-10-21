cmake_minimum_required(VERSION 3.20)
project(c-project-test VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(WARNINGS -Wall -Wextra -Wpedantic -Werror)

add_library(kv src/kv.c)
target_include_directories(kv PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_options(kv PRIVATE ${WARNINGS})

add_executable(kvget src/main.c)
target_link_libraries(kvget PRIVATE kv)
target_compile_options(kvget PRIVATE ${WARNINGS})

include(CTest)
if(BUILD_TESTING)
  add_executable(test_kv tests/test_kv.c)
  target_link_libraries(test_kv PRIVATE kv)
  target_compile_options(test_kv PRIVATE ${WARNINGS})
  add_test(NAME test_kv COMMAND test_kv)

  # The CLI end to end: value lookup, missing key, parse error.
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/sample.conf "name = demo\nport = 8080\n")
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/bad.conf "oops\n")
  add_test(NAME kvget_value COMMAND kvget sample.conf port)
  set_tests_properties(kvget_value PROPERTIES PASS_REGULAR_EXPRESSION "^8080\n$")
  add_test(NAME kvget_count COMMAND kvget sample.conf)
  set_tests_properties(kvget_count PROPERTIES PASS_REGULAR_EXPRESSION "^2\n$")
  add_test(NAME kvget_missing COMMAND kvget sample.conf nope)
  set_tests_properties(kvget_missing PROPERTIES WILL_FAIL TRUE)
  add_test(NAME kvget_bad_file COMMAND kvget bad.conf)
  set_tests_properties(kvget_bad_file PROPERTIES WILL_FAIL TRUE)
endif()

include(GNUInstallDirs)
install(TARGETS kvget kv
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES include/kv.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
