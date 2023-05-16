cmake_minimum_required(VERSION 3.22)

set(MODULE_NAME dukcpp)

project(${MODULE_NAME})

find_package(CallableTraits)

###############################################################################
# Duktape

include(cmake/duktape.cmake)

# TODO: These will be configurable.
set(DUKTAPE_PATH "E:/Source/whale_deps/src/duktape-2.6.0")
set(DUKTAPE_OPTIONS
  -DDUK_USE_FATAL_HANDLER
  -DDUK_USE_CPP_EXCEPTIONS
  -DDUK_USE_SYMBOL_BUILTIN
  -DDUK_USE_VOLUNTARY_GC
)

add_duktape_library(dukcpp-duktape "${DUKTAPE_PATH}" ${DUKTAPE_OPTIONS})

###############################################################################
# dukcpp

add_library(${MODULE_NAME} INTERFACE
  include/duk/context.h
  include/duk/duk.h
  include/duk/function.h
  include/duk/value.h
)

target_include_directories(${MODULE_NAME}
  INTERFACE
    include
)

target_link_libraries(${MODULE_NAME}
  INTERFACE
    callable_traits
)

target_compile_features(${MODULE_NAME}
  INTERFACE
    cxx_std_20
)

###############################################################################
# tests

add_subdirectory(test)
add_subdirectory(test2)
