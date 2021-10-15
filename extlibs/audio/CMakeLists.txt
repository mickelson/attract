find_package(OpenAL REQUIRED)
if(WIN32 AND (LIBTYPE_STATIC OR NOT BUILD_SHARED_LIBS))
    set(SFML_STATIC_LIBRARIES TRUE)
endif()
find_package(SFML 2.5 COMPONENTS system REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENAL_INCLUDE_DIR}
    ${SFML_INCLUDE_DIR}
)

set(audio_SOURCES
    Audio/ALCheck.cpp
    Audio/AudioDevice.cpp
    Audio/Listener.cpp
    Audio/SoundSource.cpp
    Audio/SoundStream.cpp
)

add_definitions(${OPENAL_CFLAGS})
add_library(audiolib STATIC ${audio_SOURCES})

target_link_libraries(audiolib PUBLIC sfml-system ${OPENAL_LIBRARIES})
target_include_directories(audiolib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
