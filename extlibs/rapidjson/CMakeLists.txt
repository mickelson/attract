set(INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/include"
)

file(GLOB_RECURSE rapidjson_SOURCES *.h)

add_library(rapidjson INTERFACE ${rapidjson_SOURCES})

target_include_directories(rapidjson INTERFACE ${INCLUDE_DIRS})
