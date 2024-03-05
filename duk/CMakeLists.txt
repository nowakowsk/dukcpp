cmake_minimum_required(VERSION 3.22)

set(MODULE_NAME duk)

project(${MODULE_NAME})

find_package(Boost REQUIRED
  COMPONENTS
    program_options
)

###############################################################################
# duk

add_executable(${MODULE_NAME}
  debugger.cpp
  main.cpp
  options.cpp
)

target_link_libraries(${MODULE_NAME}
  PRIVATE
    Boost::boost
    Boost::program_options
    dukcpp-duktape
    dukcpp
)

