cmake_minimum_required(VERSION 2.8)
project(gb)

include(FindOpenGL)

# includes cmake/FindSDL2.cmake
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(SDL2 REQUIRED)


include_directories(${SDL2_INCLUDE_DIR}  ${OPENGL_INCLUDE_DIR} ./include)

set(GENERATED_INSTRUCTIONS ${CMAKE_CURRENT_BINARY_DIR}/instructions.c)
file(GLOB_RECURSE SOURCES_C ${CMAKE_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE SOURCES_CXX ${CMAKE_SOURCE_DIR}/src/*.cpp)

set_source_files_properties(${GENERATED_INSTRUCTIONS} PROPERTIES GENERATED TRUE)

set(SOURCE_FILES ${GENERATED_INSTRUCTIONS} ${SOURCES_C} ${SOURCES_CXX})

add_custom_command(OUTPUT ${GENERATED_INSTRUCTIONS}
	DEPENDS
		${CMAKE_SOURCE_DIR}/resources/table_gb.html
		${CMAKE_SOURCE_DIR}/tools/base_gen.sh
		${CMAKE_SOURCE_DIR}/tools/cb_gen.sh
		${CMAKE_SOURCE_DIR}/tools/gen_z80_instructions.sh
	COMMAND
		${CMAKE_SOURCE_DIR}/tools/gen_z80_instructions.sh
	ARGS
		${CMAKE_SOURCE_DIR}/resources/table_gb.html
		${GENERATED_INSTRUCTIONS})

if (UNIX)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers")
endif (UNIX)


add_executable(gb ${SOURCE_FILES})
add_executable(rom_info tools/rom_info/rom_info.c src/rom.c)

if (WIN32 OR MSYS OR MINGW )
    target_link_libraries(gb -lmingw32 -lSDL2main -lSDL2  opengl32  glu32 m dinput8 dxguid dxerr8 user32 gdi32 winmm imm32 ole32 oleaut32 shell32 version uuid)
endif (WIN32 OR MSYS OR MINGW )

option(USE_CONSOLE_DEBUGGER
        "Connect stdin and stdout to a command-line debugger" OFF)
if (USE_CONSOLE_DEBUGGER)
    add_definitions(-DEMGB_CONSOLE_DEBUGGER)
    target_link_libraries(gb -ledit -ltermcap)
endif (USE_CONSOLE_DEBUGGER)

if (UNIX)  
   target_link_libraries(gb ${SDL2_LIBRARY} ${OPENGL_LIBRARIES})
   target_link_libraries(rom_info ${SDL2_LIBRARY})
endif (UNIX)
