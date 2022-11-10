project(ori_ray_tracer)

cmake_minimum_required(VERSION 3.18)

if(MSVC)
 add_compile_options("/MP")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ori)
set(SOLUTION_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include("C:/FILES/vcpkg/scripts/buildsystems/vcpkg.cmake")

add_subdirectory(ori)
add_subdirectory(ori_render)
