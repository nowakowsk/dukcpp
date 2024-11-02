find_package(Python2 REQUIRED)


function(add_duktape_library TARGET_NAME DUKTAPE_PATH)
  set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}")
  set(OUTPUT_FILE "${OUTPUT_DIR}/duktape.c")
  
  message(STATUS "Duktape library: ${TARGET_NAME}")
  message(STATUS "Output directory: ${OUTPUT_DIR}")
  message(STATUS "Config options: ${ARGN}")
  
  add_custom_command(
    OUTPUT "${OUTPUT_FILE}"
    WORKING_DIRECTORY "${DUKTAPE_PATH}/tools"
    COMMAND "${Python2_EXECUTABLE}" configure.py
            --output-directory "${OUTPUT_DIR}"
            ${ARGN}
  )
  
  foreach(CONFIG_OPTION ${ARGN})
    if(CONFIG_OPTION STREQUAL "--dll")
      if(WIN32)
        message(STATUS "Linking: shared")
        set(LINK_MODE SHARED)
      endif()
    endif()
  endforeach()
  
  set_source_files_properties("${OUTPUT_FILE}"
    PROPERTIES
      LANGUAGE CXX
  )
  
  add_library(${TARGET_NAME} ${LINK_MODE}
    "${OUTPUT_FILE}"
  )
  
  target_include_directories(${TARGET_NAME} SYSTEM
    PUBLIC
      "${OUTPUT_DIR}"
  )
endfunction()
