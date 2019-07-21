#
# Options
#
include(CheckIncludeFiles)

# Include the directly itself as a path for includes
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Output to a directory named bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#
# Define sources
file(GLOB modplugtomp3_SOURCES *.cpp)

#
# Define needed configuration settings
#
if(WIN32)
  message("Detected windows compile - using lame blade enc dll.")
  set (modplugtomp3_USE_BLADE_ENC_DLL 1)
else(WIN32)
  set (modplugtomp3_USE_BLADE_ENC_DLL 0)
  message("Detected non-windows compile - using lame shared object.")
endif(WIN32)
# These are actually needed by the libmodplugxmms and lame libs
CHECK_INCLUDE_FILES (inttypes.h HAVE_INTTYPES_H)
CHECK_INCLUDE_FILES (stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILES (malloc.h HAVE_MALLOC_H)

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
  )

#
# Locate any third-party libraries
#
find_package(PkgConfig REQUIRED)

#
# TAGLIB finder
#
pkg_check_modules(TAGLIB REQUIRED taglib)
if(TAGLIB_FOUND)
  message("Taglib libraries located OK.")
  message("TAGLIB_LIBRARY_DIRS=${TAGLIB_LIBRARY_DIRS}")
  message("TAGLIB_INCLUDE_DIRS=${TAGLIB_INCLUDE_DIRS}")
  message("TAGLIB_LIBRARIES=${TAGLIB_LIBRARIES}")
else()
  message("Taglib libraries could not be found.")
endif()
link_directories(${TAGLIB_LIBRARY_DIRS})
include_directories(${TAGLIB_INCLUDE_DIRS})

#
# LIBMODPLUG (XMMS version) finder
# Using pkg_config means we only build under msys on windows
#
pkg_check_modules(LIBMODPLUG REQUIRED libmodplug)
if(LIBMODPLUG_FOUND)
  message("libModPlug libraries located OK.")
  message("LIBMODPLUG_LIBRARY_DIRS= ${LIBMODPLUG_LIBRARY_DIRS}")
  message("LIBMODPLUG_INCLUDE_DIRS= ${LIBMODPLUG_INCLUDE_DIRS}")
else()
  message("libModPlug libraries could not be found.")
endif()
link_directories(${LIBMODPLUG_LIBRARY_DIRS})
include_directories(${LIBMODPLUG_INCLUDE_DIRS})

#
# LIBMP3LAME finder
#
pkg_check_modules(LIBMP3LAME REQUIRED libmp3lame)
if(LIBMP3LAME_FOUND)
  message("libmp3lame libraries located OK.")
  message("LIBMP3LAME_LIBRARY_DIRS= ${LIBMP3LAME_LIBRARY_DIRS}")
  message("LIBMP3LAME_INCLUDE_DIRS= ${LIBMP3LAME_INCLUDE_DIRS}")
else()
  message("libmp3lame libraries could not be found.")
endif()
link_directories(${LIBMP3LAME_LIBRARY_DIRS})
include_directories(${LIBMP3LAME_INCLUDE_DIRS})
#
# Add the binary tree to the search path for include files
# such as the config file
#
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/externalinclude")

#
# Setup main executable
# Note: Must occur after call to link_directories
#
add_executable(modplug2mp3 ${modplugtomp3_SOURCES})

target_link_libraries(modplug2mp3 ${LIBMODPLUG_LIBRARIES})
target_link_libraries(modplug2mp3 ${TAGLIB_LIBRARIES})
target_link_libraries(modplug2mp3 ${LIBMP3LAME_LIBRARIES})

#
# Enable C++11, make warnings errors.
#
target_compile_options(modplug2mp3 PRIVATE -Wall -Werror -std=c++11)

# taglib depends on the zlib library
target_link_options(modplug2mp3 PRIVATE -lz)

MACRO_ENSURE_OUT_OF_SOURCE_BUILD()

