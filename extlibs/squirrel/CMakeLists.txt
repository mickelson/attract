set(INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/include"
    "${CMAKE_CURRENT_LIST_DIR}/../sqrat/include"
    "${CMAKE_CURRENT_LIST_DIR}/../miniz"
)

file(GLOB_RECURSE squirrel_SOURCES *.cpp *.h)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions -fno-rtti -fno-strict-aliasing")

add_library(squirrel STATIC ${squirrel_SOURCES})

target_include_directories(squirrel PUBLIC ${INCLUDE_DIRS})
