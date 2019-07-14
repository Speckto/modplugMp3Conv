#
# Any initialisation opens
#
cmake_minimum_required(VERSION 2.8)
set (CMAKE_VERBOSE_MAKEFILE on)
set (CMAKE_CXX_STANDARD 11)

#
# Define project
#
project(modplugtomp3)

# Quick macros
MACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD)
    STRING(COMPARE EQUAL "${${PROJECT_NAME}_SOURCE_DIR}"
"${${PROJECT_NAME}_BINARY_DIR}" insource)
    GET_FILENAME_COMPONENT(PARENTDIR ${${PROJECT_NAME}_SOURCE_DIR} PATH)
    STRING(COMPARE EQUAL "${${PROJECT_NAME}_SOURCE_DIR}"
"${PARENTDIR}" insourcesubdir)
    IF(insource OR insourcesubdir)
        MESSAGE(FATAL_ERROR "${PROJECT_NAME} requires an out of source build.")
    ENDIF(insource OR insourcesubdir)
ENDMACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD)


#
# Build subdirectories
#
add_subdirectory(src)

