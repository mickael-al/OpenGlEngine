cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
cmake_policy(VERSION 3.2)

project(OpenFBX)

if(MSVC)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    foreach(flag_var CMAKE_C_FLAGS_DEBUG CMAKE_CXX_FLAGS_DEBUG)
      string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
    endforeach()
  else()
    foreach(flag_var CMAKE_C_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endforeach()
  endif()
endif()

include_directories(${CMAKE_SOURCE_DIR}/ThirdParty/OpenFBX/src)

add_library(OpenFBX
    src/ofbx.cpp
    src/ofbx.h
    src/miniz.c
    src/miniz.h
)