include(CheckFunctionExists)

if(NOT WIN32)
  add_definitions("-DXML_DEV_URANDON -DHAVE_MEMMOVE")
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
