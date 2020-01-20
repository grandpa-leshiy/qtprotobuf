function(protobuf_generate_qt_headers)
    set(options)
    set(oneValueArgs COMPONENT)
    set(multiValueArgs PUBLIC_HEADER)
    cmake_parse_arguments(protobuf_generate_qt_headers "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    foreach(PUBLIC_HEADER IN LISTS protobuf_generate_qt_headers_PUBLIC_HEADER)
        get_filename_component(PUBLIC_HEADER_BASE_NAME ${PUBLIC_HEADER} NAME FALSE)
        file(STRINGS ${PUBLIC_HEADER} CLASS_NAME REGEX "#pragma once //[a-zA-Z]+")
        if (NOT "${CLASS_NAME}" STREQUAL "")
            string(REPLACE "#pragma once //" "" CLASS_NAME "${CLASS_NAME}")
            message("-- Generate Qt header for ${CLASS_NAME}")
            configure_file("${QTPROTOBUF_CMAKE_DIR}/GeneratedHeaderTemplate" "${QTPROTOBUF_BINARY_DIR}/include/${protobuf_generate_qt_headers_COMPONENT}/${CLASS_NAME}" @ONLY)
            set(GENERATED_PUBLIC_HEADER ${GENERATED_PUBLIC_HEADER} ${QTPROTOBUF_BINARY_DIR}/include/${protobuf_generate_qt_headers_COMPONENT}/${CLASS_NAME})
        endif()
    endforeach()
    set(GENERATED_PUBLIC_HEADER ${GENERATED_PUBLIC_HEADER} PARENT_SCOPE)
endfunction()
