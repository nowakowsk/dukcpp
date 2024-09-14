cmake_minimum_required(VERSION 3.22)

project(dukcpp-test)

find_package(Catch2 REQUIRED)

###############################################################################
# dukcpp-test

add_executable(dukcpp-test
  common.cpp
  inheritance.cpp
  test.cpp
  vector.cpp
)

target_link_libraries(dukcpp-test
  PRIVATE
    Catch2::Catch2WithMain
    dukcpp-duktape
    dukcpp
)

target_compile_features(dukcpp-test
  PRIVATE
    cxx_std_20
)

if(MSVC)
  set_target_properties(dukcpp-test
    PROPERTIES
      # LNK4099: PDB was not found
      LINK_FLAGS "/ignore:4099"
  )
endif()

include(CTest)
include(Catch)
catch_discover_tests(dukcpp-test)
