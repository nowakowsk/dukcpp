cmake_minimum_required(VERSION 3.22)

set(MODULE_NAME dukcpp-test)

project(dukcpp-test)

find_package(Catch2 REQUIRED)

include(Catch)


###############################################################################
# dukcpp-test

add_executable(${MODULE_NAME}
  character.cpp
  common.cpp
  inheritance.cpp
  test.cpp
  vector.cpp
)

target_link_libraries(${MODULE_NAME}
  PRIVATE
    Catch2::Catch2WithMain
    dukcpp-duktape
    dukcpp::dukcpp
)

target_compile_features(${MODULE_NAME}
  PRIVATE
    cxx_std_20
)

if(MSVC)
  set_target_properties(${MODULE_NAME}
    PROPERTIES
      # LNK4099: PDB was not found
      LINK_FLAGS "/ignore:4099"
  )

  target_compile_definitions(${MODULE_NAME}
    PRIVATE
      _WIN32_WINNT=0x0601 # Windows 7
  )
endif()

catch_discover_tests(${MODULE_NAME})

add_test(NAME ${MODULE_NAME} COMMAND ${MODULE_NAME})

list(APPEND DUKCPP_INSTALL_TARGETS ${MODULE_NAME})
set(DUKCPP_INSTALL_TARGETS ${DUKCPP_INSTALL_TARGETS} PARENT_SCOPE)
