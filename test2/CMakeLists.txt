cmake_minimum_required(VERSION 3.22)

set(MODULE_NAME dukcpp-test2)

project(${MODULE_NAME})

###############################################################################
# dukcpp-test2

add_executable(${MODULE_NAME}
  main.cpp
)

target_link_libraries(${MODULE_NAME}
  PRIVATE
    dukcpp-duktape
    dukcpp
)

if(EMSCRIPTEN)
  set_target_properties(${MODULE_NAME}
    PROPERTIES 
      SUFFIX ".html"
  )
endif()
