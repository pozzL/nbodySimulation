cmake_minimum_required(VERSION 3.15)

project(nbody VERSION 1.0.0 LANGUAGES C CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_ARCHITECTURES 75)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(POLICY CMP0150)
  cmake_policy(SET CMP0150 NEW)
endif()
set(CMAKE_POLICY_DEFAULT_CMP0048 NEW) 
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8
)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        0.9.9.8
)

FetchContent_MakeAvailable(glfw glm)

add_executable(nbody
  src/main.cpp
  src/glad.c 
  src/Shader.cpp
  src/Sphere.cpp
  CUDA/physicsEngine.cu
)

target_include_directories(nbody PRIVATE
  include
  ${glm_SOURCE_DIR}
)

find_package(CUDAToolkit REQUIRED)
target_link_libraries(nbody PRIVATE glfw glm dl m CUDA::cudart)
