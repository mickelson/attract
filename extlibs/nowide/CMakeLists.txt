file(GLOB nowide_SOURCES *.cpp)

add_library(nowide STATIC ${nowide_SOURCES})
target_include_directories(nowide PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
