cmake_minimum_required(VERSION 3.0)

project(embree3_test)

set(DEPS "D:/depend_library")

include_directories(
${DEPS}/glfw/include 
${DEPS}/glm
${DEPS}
${DEPS}/embree/include
)

link_directories(
${DEPS}/glfw/lib
${DEPS}/embree/lib
)

FIND_PACKAGE(OpenGL)
IF (NOT OPENGL_FOUND)
  MESSAGE(FATAL_ERROR "OpenGL not found! Please either install OpenGL or disable EMBREE_TUTORIALS.")
ENDIF()

INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR} ${GLFW_INCLUDE_DIRS})

file(GLOB SOURCE ./*.*)
add_executable(embree3_test ${SOURCE})

target_link_libraries(
embree3_test 
embree3
glfw3
)




