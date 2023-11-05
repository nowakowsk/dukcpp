cmake_minimum_required(VERSION 3.22)

set(MODULE_NAME dukcpp)

project(${MODULE_NAME})

find_package(CallableTraits REQUIRED)

###############################################################################
# Duktape

include(cmake/duktape.cmake)

# TODO: These will be configurable.
set(DUKTAPE_PATH "c:/Users/nowak/deps/duktape/2_7_0")
set(DUKTAPE_OPTIONS
  -DDUK_USE_FATAL_HANDLER
  -DDUK_USE_CPP_EXCEPTIONS
  -DDUK_USE_SYMBOL_BUILTIN
  -DDUK_USE_VOLUNTARY_GC
)

add_duktape_library(dukcpp-duktape "${DUKTAPE_PATH}" ${DUKTAPE_OPTIONS})

###############################################################################
# dukcpp

add_library(${MODULE_NAME}
  include/duk/allocator_adapter.h
  include/duk/context.h
  include/duk/duk.h
  include/duk/error.h
  include/duk/function.h
  include/duk/value.h

  src/context.cpp
  src/error.cpp
)

target_include_directories(${MODULE_NAME}
  PUBLIC
    include
)

target_link_libraries(${MODULE_NAME}
  PUBLIC
    callable_traits
    dukcpp-duktape # TODO: Try to remove it so users can use their custom Duktape config.
)

target_compile_features(${MODULE_NAME}
  PUBLIC
    cxx_std_20
)

###############################################################################
# tests

add_subdirectory(test)
add_subdirectory(test2)
