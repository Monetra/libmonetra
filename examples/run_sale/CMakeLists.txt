# CMake file to build the standalone usage example for LibMonetra.
#
# This file assumes you have LibMonetra installed someplace accessible on your
# system. If it doesn't find the LibMonetra installation automatically, pass
# -DCMAKE_INSTALL_PREFIX=/path/to/LibMonetra when calling cmake.
#
# Build instructions:
#   (1) Install CMake 3.4 or newer from here: https://cmake.org/download/
#   (2) Open a command prompt/terminal window.
#   (3) Change to directory containing this file, then run the following commands:
#      mkdir build
#      cd build
#   (4) Run one of the following commands:
#     Windows w/ 64-bit LibMonetra: cmake -G "Visual Studio 15 2017 Win64" ..
#     Any other platform:           cmake ..
#   (5) Run this final command to build:
#      cmake --build . --config RelWithDebInfo
#
# Once built, you can find the "run_sale" executable under build/bin.
#
cmake_minimum_required(VERSION 3.4.3)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif ()

project(RunSale
	VERSION   1.0.0
	LANGUAGES C
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if (MSVC)
	string(APPEND CMAKE_C_FLAGS " -W3")
else ()
	string(APPEND CMAKE_C_FLAGS " -Wall")
endif ()

find_package(LibMonetra REQUIRED)

add_executable(run_sale
	run_sale.c
)

target_link_libraries(run_sale PRIVATE
	LibMonetra::libmonetra
)

# On windows, need to copy DLL's of dependencies and place them next to the executable,
# so we can run the executable from the build directory.
if (WIN32)
	# Get directory containing LibMonetra lib (install_root/bin or install_root/lib).
	get_target_property(rootdir LibMonetra::libmonetra LOCATION)
	get_filename_component(rootdir "${rootdir}" DIRECTORY)
	# Move up a directory to get to installation root, then search recursively for all DLL's.
	get_filename_component(rootdir "${rootdir}/.." ABSOLUTE)
	file(GLOB_RECURSE dlls LIST_DIRECTORIES FALSE "${rootdir}/*.dll")
	# Copy DLL's to same directory as EXE, after it's built.
	add_custom_command(TARGET run_sale POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${dlls} $<TARGET_FILE_DIR:run_sale>
		VERBATIM
	)
endif ()
