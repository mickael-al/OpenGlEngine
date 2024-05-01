cmake_minimum_required(VERSION 3.0)
project(GLEW)

set(GLEW_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(GLEW_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")

include_directories(${GLEW_INCLUDE_DIR})

find_library(GLEW_LIBRARY NAMES glew32 glew32s
             HINTS ${GLEW_LIBRARY_DIR}/Release/x64
             PATH_SUFFIXES lib)


add_library(GLEW::GLEW UNKNOWN IMPORTED)
set_target_properties(GLEW::GLEW PROPERTIES
  IMPORTED_LOCATION "${GLEW_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIR}"
)