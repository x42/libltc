cmake_minimum_required(VERSION 3.6)
project(libltc LANGUAGES C)

option(BUILD_SHARED_LIBS "Build shared libaries" ON)
option(LIBLTC_USE_STATIC_MSVC_RUNTIME "Link against the static MSVC runtime" ON)

include_directories(src)

set(HEADER_FILES 
	src/decoder.h 
	src/encoder.h
	src/ltc.h)
source_group("include" FILES ${HEADER_FILES})

set(SOURCE_FILES 
	src/decoder.c
	src/encoder.c
	src/ltc.c
	src/timecode.c)
source_group("src" FILES ${SOURCE_FILES})

add_library(libltc ${HEADER_FILES} ${SOURCE_FILES})
target_include_directories(libltc PUBLIC src)

if(NOT BUILD_SHARED_LIBS)
	target_compile_definitions(libltc PUBLIC "libltc_STATIC")
endif()

if (LIBLTC_USE_STATIC_MSVC_RUNTIME)
	foreach(flag
		CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO
		CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_DEBUG_INIT)
		string(REPLACE "/MD"  "/MT" "${flag}" "${${flag}}")
		set("${flag}" "${${flag}}")
	endforeach()
endif()