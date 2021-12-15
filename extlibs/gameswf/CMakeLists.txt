if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

if(USE_GLES)
    find_package(OpenGLES REQUIRED)
else()
    find_package(OpenGL REQUIRED)
endif()

find_package(JPEG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenAL REQUIRED)
find_package(Threads REQUIRED)

if(NOT APPLE)
    find_package(Freetype REQUIRED)
endif()

# Missing gameswf_xml.[cpp|h]
#find_package( XML2 )
#
#if ( XML2_FOUND )
#    add_definitions( -DHAVE_LIBXML )
#endif()

set(base_SOURCES
    base/container.cpp
    base/cschema.h
    base/ear_clip_triangulate_float.cpp
    base/ear_clip_triangulate_sint16.cpp
    base/file_util.cpp
    base/image.cpp
    base/image_filters.cpp
    base/jpeg.cpp
    base/logger.cpp
    base/membuf.cpp
    base/postscript.cpp
    base/triangulate_float.cpp
    base/triangulate_sint32.cpp
    base/tu_file.cpp
    base/tu_gc_singlethreaded_marksweep.cpp
    base/tu_loadlib.cpp
    base/tu_random.cpp
    base/tu_timer.cpp
    base/tu_types.cpp
    base/utf8.cpp
    base/utility.cpp
    base/zlib_adapter.cpp
)

file(GLOB_RECURSE gameswf_as_SOURCES gameswf/gameswf_as_classes/*.cpp gameswf/gameswf_as_classes/*.h)

set(gameswf_SOURCES
    ${gameswf_as_SOURCES}
    gameswf/gameswf_abc.cpp
    gameswf/gameswf_action.cpp
    gameswf/gameswf_avm2.cpp
    gameswf/gameswf_as_sprite.cpp
    gameswf/gameswf_button.cpp
    gameswf/gameswf_canvas.cpp
    gameswf/gameswf_character.cpp
    gameswf/gameswf_disasm.cpp
    gameswf/gameswf_dlist.cpp
    gameswf/gameswf_environment.cpp
    gameswf/gameswf_filters.cpp
    gameswf/gameswf_font.cpp
    gameswf/gameswf_function.cpp
    gameswf/gameswf_impl.cpp
    gameswf/gameswf_listener.cpp
    gameswf/gameswf_log.cpp
    gameswf/gameswf_morph2.cpp
    gameswf/gameswf_movie_def.cpp
    gameswf/gameswf_object.cpp
    gameswf/gameswf_player.cpp
    gameswf/gameswf_render.cpp
    gameswf/gameswf_root.cpp
    gameswf/gameswf_shape.cpp
    gameswf/gameswf_sound.cpp
    gameswf/gameswf_sprite.cpp
    gameswf/gameswf_sprite_def.cpp
    gameswf/gameswf_stream.cpp
    gameswf/gameswf_styles.cpp
    gameswf/gameswf_tesselate.cpp
    gameswf/gameswf_text.cpp
    gameswf/gameswf_tools.cpp
    gameswf/gameswf_types.cpp
    gameswf/gameswf_value.cpp
    gameswf/gameswf_video_impl.cpp
    gameswf/gameswf_sound_handler_openal.cpp
    gameswf/gameswf_mutex.cpp
)

if(USE_GLES)
    if(NO_MOVIE)
        message(FATAL_ERROR "SWF OpenGLES requires FFmpeg support")
    endif()
    list(APPEND gameswf_SOURCES gameswf/gameswf_render_handler_ogles.cpp)
    include_directories(${OPENGLES_INCLUDE_DIR})
else()
    list(APPEND gameswf_SOURCES gameswf/gameswf_render_handler_ogl.cpp)
    include_directories(${OPENGL_INCLUDE_DIR})
endif()

if(FREETYPE_FOUND)
    list(APPEND gameswf_SOURCES gameswf/gameswf_freetype.cpp)
    include_directories(${FREETYPE_INCLUDE_DIRS})
else()
    list(APPEND gameswf_SOURCES gameswf/gameswf_fontlib.cpp)
endif()

add_definitions(${OPENAL_CFLAGS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/base ${OPENAL_INCLUDE_DIR})

add_library(gameswf STATIC ${base_SOURCES} ${gameswf_SOURCES})
set_target_properties(gameswf PROPERTIES COMPILE_FLAGS -Wno-deprecated)

target_link_libraries(gameswf PUBLIC ${OPENGL_LIBRARIES} ${OPENGLES_LIBRARIES} ${JPEG_LIBRARY} ${ZLIB_LIBRARY} ${OPENAL_LIBRARIES} ${FREETYPE_LIBRARIES} ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(gameswf PUBLIC ${OPENGL_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${FREETYPE_INCLUDE_DIRS})
