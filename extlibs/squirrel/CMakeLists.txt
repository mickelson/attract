set(INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include")

file(GLOB_RECURSE sources *.cpp *.h)

add_library(squirrel STATIC ${sources})

target_include_directories(squirrel PUBLIC ${INCLUDE_DIR})
