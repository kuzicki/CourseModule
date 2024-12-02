file(GLOB_RECURSE SOURCES "*.cpp" "*.hpp")

add_library(course-module SHARED ${SOURCES})
target_link_libraries(course-module LINK_PUBLIC sc-memory)
target_include_directories(course-module PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
