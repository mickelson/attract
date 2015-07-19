include_directories(
  include
  ${SFML_INCLUDE_DIR}
)

set(SRC_FILES
  Audio/ALCheck.cpp
  Audio/AudioDevice.cpp
  Audio/Listener.cpp
  Audio/SoundSource.cpp
  Audio/SoundStream.cpp
)

add_library(audiolib STATIC ${SRC_FILES})

target_include_directories(audiolib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
