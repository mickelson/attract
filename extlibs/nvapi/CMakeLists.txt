set(INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}"
)

file(GLOB_RECURSE nvapi_SOURCES *.hpp)

add_library(nvapi INTERFACE ${nvapi_SOURCES})

target_include_directories(nvapi INTERFACE ${INCLUDE_DIRS})
