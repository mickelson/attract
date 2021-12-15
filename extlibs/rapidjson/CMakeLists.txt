set(INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/include"
)

add_library(rapidjson INTERFACE)

target_include_directories(rapidjson INTERFACE ${INCLUDE_DIRS})
