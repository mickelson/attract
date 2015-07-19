include_directories(
  ${EXTLIBS_PATH}/sqrat/include
  ${EXTLIBS_PATH}/miniz
  ${PROJECT_BINARY_DIR}
)

set(sources
	fe_base.cpp
	fe_base.hpp
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
	fe_xml.cpp
	fe_xml.hpp
	fe_settings.cpp
	fe_settings.hpp
	fe_build.cpp
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
	fe_icon.hpp
	zip.cpp
	zip.hpp
	main.cpp
)

find_library(EXPAT_LIBRARY expat)

set( libs
	squirrel
	${EXPAT_LIBRARY}
	${SFML_GRAPHICS_DEPENDENCIES}
	${SFML_GRAPHICS_LIBRARY}
	${SFML_WINDOW_DEPENDENCIES}
	${SFML_WINDOW_LIBRARY}
	${SFML_SYSTEM_DEPENDENCIES}
	${SFML_SYSTEM_LIBRARY}
)

if ( SFML_NETWORK_FOUND AND NOT NO_NET )
    list( APPEND libs
        ${SFML_NETWORK_DEPENDENCIES}
        ${SFML_NETWORK_LIBRARY}
    )
    list( APPEND sources
        fe_net.cpp
        fe_net.hpp
    )
endif()

#   - SFML_XXX_LIBRARY:         the name of the library to link to for the xxx module (includes both debug and optimized names if necessary)
#   - SFML_XXX_FOUND:           true if either the debug or release library of the xxx module is found
#   - SFML_XXX_DEPENDENCIES:    the list of libraries the module depends on, in case of static linking

if ( NOT NO_MOVIE )
  add_definitions( -DWITH_MOVIE )
  list( APPEND sources media.hpp media.cpp )
  if ( FFMPEG_SWRESAMPLE_FOUND )
    list( APPEND libs ${FFMPEG_SWRESAMPLE_LIBRARIES} )
    add_definitions( -DUSE_SWRESAMPLE )
  elseif ( FFMPEG_AVRESAMPLE_FOUND )
    list( APPEND libs ${FFMPEG_AVRESAMPLE_LIBRARIES} )
    add_definitions( -DUSE_AVRESAMPLE )
  endif()
  list( APPEND libs ${FFMPEG_LIBRARIES} openal audiolib )
else()
  list( APPEND libs ${SFML_AUDIO_DEPENDENCIES} ${SFML_AUDIO_LIBRARY} )
endif()

if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
  list(APPEND sources
	  fe_util_osx.mm
	  fe_util_osx.hpp
  )
  set_source_files_properties(fe_util_osx.mm PROPERTIES LANGUAGE C)
  find_library( COCOA_LIBRARY Cocoa REQUIRED )
  list(APPEND libs ${COCOA_LIBRARY} )
endif()

add_executable( ${EXECUTABLE_NAME} ${sources} )
target_link_libraries( ${EXECUTABLE_NAME} ${libs} )

# WITH_MOVIE
#  audiolib
