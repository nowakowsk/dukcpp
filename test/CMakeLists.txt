cmake_minimum_required(VERSION 3.22)

project(dukcpp-test)

find_package(Catch2 REQUIRED)

###############################################################################
# dukcpp-test

add_executable(dukcpp-test
  test.cpp
)

target_link_libraries(dukcpp-test
  PRIVATE
    Catch2::Catch2WithMain
    dukcpp-duktape
    dukcpp
)

include(CTest)
include(Catch)
catch_discover_tests(dukcpp-test)
