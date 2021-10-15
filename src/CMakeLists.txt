include_directories(
    ${PROJECT_BINARY_DIR}
)

set(attract_SOURCES
    fe_base.cpp
    fe_base.hpp
    fe_file.cpp
    fe_file.hpp
    fe_cmdline.cpp
    fe_util.cpp
    fe_util.hpp
    fe_util_sq.cpp
    fe_util_sq.hpp
    fe_info.cpp
    fe_info.hpp
    fe_input.cpp
    fe_input.hpp
    fe_romlist.cpp
    fe_romlist.hpp
    scraper_base.cpp
    scraper_base.hpp
    scraper_general.cpp
    scraper_gamesdb.cpp
    scraper_net.cpp
    scraper_xml.cpp
    scraper_xml.hpp
    fe_settings.cpp
    fe_settings.hpp
    fe_config.cpp
    fe_config.hpp
    fe_presentable.cpp
    fe_presentable.hpp
    fe_present.cpp
    fe_present.hpp
    sprite.cpp
    sprite.hpp
    fe_image.cpp
    fe_image.hpp
    fe_sound.cpp
    fe_sound.hpp
    fe_shader.cpp
    fe_shader.hpp
    fe_overlay.cpp
    fe_overlay.hpp
    fe_window.cpp
    fe_window.hpp
    tp.cpp
    tp.hpp
    fe_text.cpp
    fe_text.hpp
    fe_listbox.cpp
    fe_listbox.hpp
    fe_vm.cpp
    fe_vm.hpp
    fe_blend.cpp
    fe_blend.hpp
    zip.cpp
    zip.hpp
    path_cache.cpp
    path_cache.hpp
    image_loader.cpp
    image_loader.hpp
    main.cpp
)

#### BEGIN: Required

set(attract_LIBS squirrel nowide rapidjson)

# SFML
set(sfmlcomponents graphics window system)
if(NO_MOVIE)
    list(APPEND sfmlcomponents audio)
endif()
if(WIN32)
    # Flags are set by MXE cmake toolchain
    if(LIBTYPE_STATIC OR NOT BUILD_SHARED_LIBS)
        set(SFML_STATIC_LIBRARIES TRUE)
    endif()
    if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
        list(APPEND sfmlcomponents main)
    endif()
endif()
find_package(SFML 2.5 COMPONENTS ${sfmlcomponents} REQUIRED)

list(APPEND attract_LIBS sfml-graphics sfml-window sfml-system)

#### END: Required

#### BEGIN: Features

# System FontConfig, Expat or included Expat?
if(USE_FONTCONFIG)
    list(APPEND attract_LIBS ${FONTCONFIG_LIBRARIES})
    include_directories(${FONTCONFIG_INCLUDE_DIR})
endif()

if(EXPAT_FOUND)
    list(APPEND attract_LIBS ${EXPAT_LIBRARIES})
    include_directories(${EXPAT_INCLUDE_DIR})
else()
    list(APPEND attract_LIBS expat)
endif()

# FFMPEG support
if(NOT NO_MOVIE)
    if(FFMPEG_SWRESAMPLE_FOUND)
        add_definitions(-DUSE_SWRESAMPLE)
    elseif(FFMPEG_AVRESAMPLE_FOUND)
        add_definitions(-DUSE_AVRESAMPLE)
    endif()
    list(APPEND attract_SOURCES media.hpp media.cpp)
    include_directories(${FFMPEG_INCLUDE_DIRS})
    if(WIN32 AND (LIBTYPE_STATIC OR NOT BUILD_SHARED_LIBS))
        list(APPEND attract_LIBS audiolib ${FFMPEG_STATIC_LIBRARIES})
    else()
        list(APPEND attract_LIBS audiolib ${FFMPEG_LIBRARIES})
    endif()
    set_source_files_properties(media.cpp PROPERTIES COMPILE_FLAGS -D__STDC_CONSTANT_MACROS)
endif()

# Network support
if (USE_LIBCURL)
    list(APPEND attract_LIBS ${CURL_LIBRARIES})
    include_directories(${CURL_INCLUDE_DIRS})
    add_definitions(${CURL_CFLAGS})
    list(APPEND attract_SOURCES fe_net.cpp fe_net.hpp)
endif()

# SWF support
if(NOT NO_SWF)
    list(APPEND attract_SOURCES swf.cpp swf.hpp)
    list(APPEND attract_LIBS gameswf)
    set_source_files_properties(swf.cpp PROPERTIES COMPILE_FLAGS -Wno-deprecated)
endif()

# LibArchive support
if(USE_LIBARCHIVE)
    list(APPEND attract_LIBS ${LIBARCHIVE_LIBRARIES})
    include_directories(${LIBARCHIVE_INCLUDE_DIRS})
    add_definitions(${LIBARCHIVE_CFLAGS})
else()
    include_directories(${CMAKE_SOURCE_DIR}/extlibs/miniz)
endif()

# Xinerama support
if(USE_XINERAMA)
    list(APPEND attract_LIBS ${XINERAMA_LIBRARIES} X11)
    include_directories(${XINERAMA_INCLUDE_DIRS})
endif()

#### END: Features ####

#### BEGIN: Platforms ####

if(APPLE)
  list(APPEND attract_SOURCES fe_util_osx.mm fe_util_osx.hpp)
  set_source_files_properties(fe_util_osx.mm PROPERTIES LANGUAGE C)
  find_library(COCOA_LIBRARY Cocoa REQUIRED)
  list(APPEND attract_LIBS ${COCOA_LIBRARY})
  find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)
  list(APPEND attract_LIBS ${COREVIDEO_LIBRARY})
endif()

if(WIN32)
    enable_language(RC)
    list(APPEND attract_LIBS nvapi dwmapi)
    list(APPEND attract_SOURCES attract.rc)
    if(WINDOWS_CONSOLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mconsole")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--subsystem,windows")
    endif()
    set(CMAKE_EXE_LINKER_FLAGS "-static")
endif()

add_executable(attract ${attract_SOURCES})
target_link_libraries(attract ${attract_LIBS})

install(TARGETS attract
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
