project(fract_render)

file(GLOB APP_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB APP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${APP_HEADERS} ${APP_SOURCES} ${APP_RESOURCE})

add_executable(${PROJECT_NAME} ${APP_HEADERS} ${APP_SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC fract_lib)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/fract_lib)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "fract_render")
