
cmake_minimum_required(VERSION 3.0)
set(PROJECT_NAME libltc)
project(${PROJECT_NAME} C)

# Look for config files in the build directory..
LIST( APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} )

file(GLOB_RECURSE sources src/*.c src/*.h)

add_library(libltc STATIC ${sources})
if(NOT MSVC)
	target_link_libraries(libltc m)
endif(NOT MSVC)
target_compile_options(libltc PUBLIC -Wall)
target_include_directories(libltc PUBLIC src)

option(BUILD_TESTS "Build tests" OFF)

if(${BUILD_TESTS})
	add_executable(libltc_decode tests/ltcdecode.c)
	add_executable(libltc_encode tests/ltcencode.c)
	add_executable(libltc_encode_example tests/example_encode.c)

	target_link_libraries(libltc_decode libltc)
	target_link_libraries(libltc_encode libltc)
	target_link_libraries(libltc_encode_example libltc)

	target_compile_options(libltc_decode PUBLIC -Wall)
	target_compile_options(libltc_encode PUBLIC -Wall)
	target_compile_options(libltc_encode_example PUBLIC -Wall)

	target_include_directories(libltc_decode PUBLIC src)
	target_include_directories(libltc_encode PUBLIC src)
	target_include_directories(libltc_encode_example PUBLIC src)
endif(${BUILD_TESTS})

###############################################
# Exports
###############################################

SET( CFG_TARGET ${PROJECT_NAME} )
SET( CFG_DEFINITIONS ${${PROJECT_NAME}_DEFINITIONS} )
SET( CFG_COMPILE_FLAGS "" )
SET( CFG_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src"  )
SET( CFG_LIBRARIES ${PROJECT_NAME} )
CONFIGURE_FILE( "${PROJECT_NAME}Config.cmake.in" "${CMAKE_BINARY_DIR}/${PROJECT_NAME}Config.cmake" @ONLY )
