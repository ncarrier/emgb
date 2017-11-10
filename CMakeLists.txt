cmake_minimum_required(VERSION 2.8)
project(gb)

include(FindOpenGL)

# includes cmake/FindSDL2.cmake
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(SDL2 REQUIRED)


include_directories(${SDL2_INCLUDE_DIR}  ${OPENGL_INCLUDE_DIR} ./include)

file(GLOB_RECURSE SOURCES_C ${CMAKE_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE SOURCES_CXX ${CMAKE_SOURCE_DIR}/src/*.cpp)

set(SOURCE_FILES ${SOURCES_C} ${SOURCES_CXX})

if (UNIX)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -Wextra")
endif (UNIX)


add_executable(gb ${SOURCE_FILES})

if (WIN32 OR MSYS OR MINGW )
    target_link_libraries(gb -lmingw32 -lSDL2main -lSDL2  opengl32  glu32 m dinput8 dxguid dxerr8 user32 gdi32 winmm imm32 ole32 oleaut32 shell32 version uuid)
endif (WIN32 OR MSYS OR MINGW )

if (UNIX)  
   target_link_libraries(gb ${SDL2_LIBRARY} ${OPENGL_LIBRARIES})
endif (UNIX)
