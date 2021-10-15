if(NOT NO_MOVIE)
	add_subdirectory(audio)
endif()

add_subdirectory(squirrel)
add_subdirectory(nowide)
add_subdirectory(rapidjson)
add_subdirectory(nvapi)

if(NOT EXPAT_FOUND)
	add_subdirectory(expat)
endif()

if(NOT NO_SWF)
	add_subdirectory(gameswf)
endif()
