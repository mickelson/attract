set(INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}"
)

add_library(nvapi INTERFACE)

target_include_directories(nvapi INTERFACE ${INCLUDE_DIRS})
