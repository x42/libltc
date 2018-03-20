libLTC
------

Linear (or Longitudinal) Timecode (LTC) is an encoding of SMPTE timecode data
as a Manchester-Biphase encoded audio signal.
The audio signal is commonly recorded on a VTR track or other storage media.

libltc provides functionality to encode and decode LTC audio from/to
SMPTE or EBU timecode, including SMPTE date support.

libltc is the successor of [libltcsmpte](https://sourceforge.net/projects/ltcsmpte/).
For more information, please see the FAQ in the documentation.

Documentation
-------------

The API reference, examples, as well as introduction can be found at

http://x42.github.com/libltc/

This site is part or the source-code in the doc/ folder.

CMAKE
-----

A sample CMakeLists.txt to reference for your project:

	cmake_minimum_required(VERSION 3.0)
	project(my_libltc_sample C CXX)

	LIST( APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} )

	add_subdirectory(libltc)
	find_package(libltc CONFIG REQUIRED)

	file(GLOB_RECURSE sources src/*.cpp src/*.h)

	add_executable(my_libltc_sample ${sources})
	target_link_libraries(my_libltc_sample ${libltc_LIBRARIES})
	target_compile_options(my_libltc_sample PUBLIC -std=c++1y)
	target_include_directories(my_libltc_sample PUBLIC src ${libltc_INCLUDES})

