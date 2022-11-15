project(fract_lib)

file(GLOB_RECURSE HEADER_FILES "*.h")
file(GLOB_RECURSE SOURCE_FILES "*.cpp" "*.c")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HEADER_FILES} ${SOURCE_FILES})

add_library(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/fract_lib)

#target_link_libraries(${PROJECT_NAME} PUBLIC third_party)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "fract_lib")

# glfw
find_package(glfw3 REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)

# directxtk
find_package(directxtk12 REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Microsoft::DirectXTK12)

# directx shader compiler
find_package(directx-dxc REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Microsoft::DirectXShaderCompiler)

# d3dma
find_package(d3d12-memory-allocator REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC unofficial::d3d12-memory-allocator)

#argparse
find_package(argparse CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC argparse::argparse)

find_path(STB_INCLUDE_DIRS "stb_c_lexer.h")
target_include_directories(${PROJECT_NAME} PUBLIC ${STB_INCLUDE_DIRS})

find_package(assimp CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE assimp::assimp)


# spdlog
find_package(spdlog REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog spdlog::spdlog_header_only)


find_package(mimalloc CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE mimalloc mimalloc)
