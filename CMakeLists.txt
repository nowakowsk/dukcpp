cmake_minimum_required(VERSION 3.22)

set(MODULE_NAME dukcpp)
set(MODULE_VERSION 0.1.0)

project(${MODULE_NAME})

find_package(Boost REQUIRED)

include(CMakePackageConfigHelpers)
include(CTest)
include(GNUInstallDirs)


###############################################################################
# dukcpp

add_library(${MODULE_NAME} INTERFACE)

target_include_directories(${MODULE_NAME}
  INTERFACE
    "$<INSTALL_INTERFACE:include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)

target_link_libraries(${MODULE_NAME}
  INTERFACE
    Boost::boost
)

target_compile_features(${MODULE_NAME}
  INTERFACE
    cxx_std_20
)

add_library(${MODULE_NAME}::${MODULE_NAME} ALIAS ${MODULE_NAME})

list(APPEND DUKCPP_INSTALL_TARGETS ${MODULE_NAME})


###############################################################################
# dukcpp-duktape
#
# This target is used only by tests. It won't be exported. Users are expected to bring their own Duktape library.
# They can generate one with add_duktape_library() macro.

include(cmake/duktape.cmake)

set(DUKCPP_DUKTAPE_PATH "" CACHE PATH "Path to Duktape source code directory")
if(DUKCPP_DUKTAPE_PATH STREQUAL "")
  message(FATAL_ERROR "Path to Duktape source code directory is not set (DUKCPP_DUKTAPE_PATH).")
endif()

add_duktape_library(dukcpp-duktape "${DUKCPP_DUKTAPE_PATH}"
  -DDUK_USE_FATAL_HANDLER
  -DDUK_USE_CPP_EXCEPTIONS
  -DDUK_USE_SYMBOL_BUILTIN
  -DDUK_USE_VOLUNTARY_GC

  -DDUK_USE_INTERRUPT_COUNTER
  -DDUK_USE_DEBUGGER_SUPPORT
  -DDUK_USE_DEBUGGER_INSPECT
)


###############################################################################
# Tests and tools

add_subdirectory(duk)
add_subdirectory(test)


###############################################################################
# Install

set(MODULE_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${MODULE_NAME}")
set(MODULE_TARGETS_NAME "${MODULE_NAME}-targets")

install(
  TARGETS ${DUKCPP_INSTALL_TARGETS}
  EXPORT ${MODULE_TARGETS_NAME}
)

install(
  EXPORT ${MODULE_TARGETS_NAME}
  FILE "${MODULE_NAME}-targets.cmake"
  NAMESPACE ${MODULE_NAME}::
  DESTINATION "${MODULE_CONFIG_INSTALL_DIR}"
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-config-version.cmake"
  VERSION ${MODULE_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/cmake/config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-config.cmake"
  INSTALL_DESTINATION "${MODULE_CONFIG_INSTALL_DIR}"
)

install(
  FILES
    "${CMAKE_CURRENT_LIST_DIR}/cmake/duktape.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-config-version.cmake"

  DESTINATION "${MODULE_CONFIG_INSTALL_DIR}"
)

install(
  DIRECTORY
    "include/"

  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
