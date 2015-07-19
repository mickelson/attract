include(CheckFunctionExists)

check_function_exists(memmove MEMMOVE_EXISTS)
if(MEMMOVE_EXISTS)
    add_definitions("-DHAVE_MEMMOVE")
else()
    message(FATAL_ERROR "memmove not supported")
endif()

set(expat_SOURCES
    xmlparse.c
    xmlrole.c
    xmlrole.h
    xmltok.c
    xmltok.h
    loadlibrary.c
)

add_library(expat STATIC ${expat_SOURCES})

target_include_directories(expat PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
