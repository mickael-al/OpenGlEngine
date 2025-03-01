# Parametre Repertoire

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Debug)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Setting)

#User
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/User)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/User/Input)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/ProjectUser)

#Render
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Rendering)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Rendering/Window)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Rendering/Shape)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Rendering/Shader)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Rendering/Light)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Rendering/ImGUI)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Rendering/ImGUI/ImGuizmo)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Rendering/Camera)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Rendering/Materials)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Rendering/XR)

#Miscellaneous
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Miscellaneous)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Interface)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Miscellaneous/PathFinding)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Miscellaneous/Goap)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Miscellaneous/WaveFunctionCollapse)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Sound)

#Struct
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Struct)

#Physics
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ThirdParty/Bullet3/src)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ANTAGONIZ/Header/Physics)

#VR 
INCLUDE_DIRECTORIES(${OPENXR_INCLUDE_DIR})

#File
FILE(GLOB_RECURSE ENGINE_SOURCES *.cpp *.c)
FILE(GLOB_RECURSE ENGINE_HEADERS *.hpp *.h *.inl)
ADD_EXECUTABLE(ANTAGONIZ ${ENGINE_SOURCES} ${ENGINE_HEADERS})

find_package(OpenGL REQUIRED)

TARGET_LINK_LIBRARIES(ANTAGONIZ PRIVATE GLEW glfw glfw3 termcolor tinyobjloader OpenFBX stb glm ImGUI OpenAL LinearMath Bullet3Common BulletInverseDynamics BulletCollision BulletDynamics BulletSoftBody openvr_api ${OPENGL_LIBRARIES})