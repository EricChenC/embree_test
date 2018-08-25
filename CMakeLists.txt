cmake_minimum_required(VERSION 3.0)

project(embree3_test)

set(DEPS "D:/depend_library")

include_directories(
${DEPS}/glfw/include 
${DEPS}/glm
${DEPS}
${DEPS}/embree-3.0/include
${DEPS}/tbb/include
)

link_directories(
${DEPS}/glfw/lib-vc2015
${DEPS}/embree-3.0/lib
${DEPS}/tbb/lib/intel64/vc14
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
lexers
math
simd
sys
tasking
debug glfw3dll
optimized glfw3
tbb
opengl32
)




