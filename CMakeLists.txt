cmake_minimum_required(VERSION 3.22)

set(MODULE_NAME dukcpp)

project(${MODULE_NAME})

find_package(Boost REQUIRED)

###############################################################################
# Duktape

include(cmake/duktape.cmake)

set(DUKCPP_DUKTAPE_PATH "" CACHE PATH "Path to Duktape source code directory")
if(DUKCPP_DUKTAPE_PATH STREQUAL "")
  status(FATAL_ERROR "Path to Duktape source code directory is not set (DUKCPP_DUKTAPE_PATH).")
endif()

# TODO: These will be configurable.
set(DUKTAPE_OPTIONS
  -DDUK_USE_FATAL_HANDLER
  -DDUK_USE_CPP_EXCEPTIONS
  -DDUK_USE_SYMBOL_BUILTIN
  -DDUK_USE_VOLUNTARY_GC
)

add_duktape_library(dukcpp-duktape "${DUKCPP_DUKTAPE_PATH}" ${DUKTAPE_OPTIONS})

###############################################################################
# dukcpp

add_library(${MODULE_NAME}
  include/duk/detail/function_wrapper.h
  include/duk/allocator_adapter.h
  include/duk/common.h
  include/duk/context.h
  include/duk/duk.h
  include/duk/error.h
  include/duk/function_helpers.h
  include/duk/function.h
  include/duk/scoped_pop.h
  include/duk/type_traits_helpers.h
  include/duk/type_traits.h

  src/context.cpp
  src/error.cpp
)

target_include_directories(${MODULE_NAME}
  PUBLIC
    include
)

target_link_libraries(${MODULE_NAME}
  PUBLIC
    Boost::boost
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
