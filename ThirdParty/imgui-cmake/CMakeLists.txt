CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2)

PROJECT(ImGUI)

INCLUDE_DIRECTORIES(${ThirdParty}/imgui-cmake/Header/)
INCLUDE_DIRECTORIES(${ThirdParty}/glfw/include/)
LINK_DIRECTORIES(${ThirdParty}/imgui-cmake/freeTypeLib)

ADD_LIBRARY(ImGUI STATIC
        Source/imgui.cpp
		Source/imgui_demo.cpp
		Source/imgui_draw.cpp
		Source/imgui_impl_glfw.cpp
		Source/imgui_impl_opengl2.cpp
		Source/imgui_impl_opengl3.cpp
		Source/imgui_widgets.cpp
		Source/binary_to_compressed_c.cpp
		Source/imgui_freetype.cpp
		Source/imgui_stdlib.cpp
		Source/imgui_tables.cpp)