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
    Boost::program_options
    dukcpp-duktape
    dukcpp::dukcpp
)

target_compile_features(${MODULE_NAME}
  PRIVATE
    cxx_std_20
)

if(MSVC)
  target_compile_definitions(${MODULE_NAME}
    PRIVATE
      _WIN32_WINNT=0x0601 # Windows 7
  )
endif()

list(APPEND DUKCPP_INSTALL_TARGETS ${MODULE_NAME})
set(DUKCPP_INSTALL_TARGETS ${DUKCPP_INSTALL_TARGETS} PARENT_SCOPE)
