cmake_minimum_required(VERSION 3.4.3)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/")

include(DefaultBuildType)

project(LibMonetra 
	VERSION   8.0.0
	LANGUAGES C
)

include(EnableWarnings)
include(Harden)
include(InstallExports) # Provides install_exports() function.

include(AddressSanitizer) # Provides M_ASAN option, if supported by compiler

# Version info.
set(LIBMONETRA_VERSION_MAJOR    ${PROJECT_VERSION_MAJOR})
set(LIBMONETRA_VERSION_MINOR    ${PROJECT_VERSION_MINOR})
set(LIBMONETRA_VERSION_PATCH    ${PROJECT_VERSION_PATCH})
set(LIBMONETRA_VERSION_STRING   ${PROJECT_VERSION})

set(LIBMONETRA_SOVERSION_STRING ${LIBMONETRA_VERSION_MAJOR})

option(LIBMONETRA_STATIC          "Build as a static library"                              FALSE)
option(LIBMONETRA_STATIC_PIC      "Build the static library as PIC (position independent)" FALSE)
option(LIBMONETRA_INSTALL_HEADERS "Install headers"                                        TRUE)
option(LIBMONETRA_INSTALL_LIBS    "Install libraries"                                      TRUE)
option(LIBMONETRA_INSTALL_EXPORTS "Install exports"                                        ${LIBMONETRA_INSTALL_LIBS})
option(LIBMONETRA_BUILD_APPS      "Build apps"                                             TRUE)
option(LIBMONETRA_BUILD_DOCS      "Build doxygen docs"                                     TRUE)
option(LIBMONETRA_INSTALL_APPS    "Install apps"                                           TRUE)
option(LIBMONETRA_BUILD_TESTS     "Build tests"                                            TRUE)
option(LIBMONETRA_USE_VALGRIND    "Run tests with valgrind when running 'make check'"      TRUE)
mark_as_advanced(FORCE LIBMONETRA_INSTALL_EXPORTS)


# Keep build organized.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

# For chain building.
set(LIBMONETRA_TOPLEVEL_DIR "${CMAKE_CURRENT_SOURCE_DIR}")


# Docs
if (LIBMONETRA_BUILD_DOCS)
	find_package(Doxygen)
	if (DOXYGEN_FOUND)
		configure_file(Doxyfile.cmake "${PROJECT_BINARY_DIR}/Doxyfile" @ONLY)
		add_custom_target(libmonetra-doc ALL
			COMMAND  ${DOXYGEN_EXECUTABLE} "${PROJECT_BINARY_DIR}/Doxyfile"
			SOURCES  "${PROJECT_BINARY_DIR}/Doxyfile"
			COMMENT  "Generating API documentation with Doxygen"
			VERBATIM
		)
		install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs/html DESTINATION share/${PROJECT_NAME}/doc)
		# Doxygen man pages seem to generate links for things like structure members.  We need to
		# exclude these non-namespaced items, so we're just going to install files that begin with
		# M_, m_ or LM_
		install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs/man DESTINATION share
			FILES_MATCHING PATTERN "M_*"
			               PATTERN "m_*"
			               PATTERN "LM_*"
		)
	endif ()
endif ()


# Find and use mstdlib.
if (NOT TARGET Mstdlib::base OR NOT TARGET Mstdlib::formats OR NOT TARGET Mstdlib::thread OR NOT TARGET Mstdlib::io OR NOT TARGET Mstdlib::tls)
	if (IS_DIRECTORY "${LIBMONETRA_TOPLEVEL_DIR}/thirdparty/mstdlib")
			# Override the following settings in the subdirectory, and hide them from the user.
			set(MSTDLIB_BUILD_TESTS  FALSE CACHE INTERNAL "")
			set(MSTDLIB_BUILD_THREAD TRUE  CACHE INTERNAL "")
			set(MSTDLIB_BUILD_IO     TRUE  CACHE INTERNAL "")
			set(MSTDLIB_BUILD_TLS    TRUE  CACHE INTERNAL "")
			set(MSTDLIB_BUILD_LOG    FALSE CACHE INTERNAL "")
			set(MSTDLIB_BUILD_SQL    FALSE CACHE INTERNAL "")

			add_subdirectory("${LIBMONETRA_TOPLEVEL_DIR}/thirdparty/mstdlib")
	else ()
		find_package(Mstdlib REQUIRED COMPONENTS base formats thread io tls)
	endif ()
endif()


# Installation location
if (NOT LIBMONETRA_INSTALL_LOCATION_LIBS)
	set(LIBMONETRA_INSTALL_LOCATION_LIBS lib)
endif ()
if (NOT LIBMONETRA_INSTALL_LOCATION_HEADERS)
	set(LIBMONETRA_INSTALL_LOCATION_HEADERS include)
endif ()
if (NOT LIBMONETRA_INSTALL_LOCATION_SHARE)
	set(LIBMONETRA_INSTALL_LOCATION_SHARE share)
endif ()

set(lib_dest
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION ${LIBMONETRA_INSTALL_LOCATION_LIBS}
	ARCHIVE DESTINATION ${LIBMONETRA_INSTALL_LOCATION_LIBS}
)

add_subdirectory(include)
add_subdirectory(src)

if (LIBMONETRA_BUILD_APPS)
	add_subdirectory(apps)
endif ()

# Tests need to check for the test frame work so it will handle enabled/disable itself.
add_subdirectory(test)

# Trick to make headers and scripts show up in IDEs. Adds a phony target that references the files.
file(GLOB_RECURSE headers_and_scripts *.h)
add_custom_target(_libmonetra_root_extra_files ALL SOURCES ${headers_and_scripts})

if (LIBMONETRA_INSTALL_LIBS)
	if (LIBMONETRA_INSTALL_EXPORTS)
		install_exports(LibMonetra "${LIBMONETRA_INSTALL_LOCATION_SHARE}/LibMonetra/cmake")
	endif ()

	# If we're not being chain-built, and install is enabled, add code to allow package creation.
	if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND LIBMONETRA_INSTALL_LIBS)
		set(CPACK_GENERATOR ZIP)
		set(CPACK_PACKAGE_VERSION_MAJOR ${LIBMONETRA_VERSION_MAJOR})
		set(CPACK_PACKAGE_VERSION_MINOR ${LIBMONETRA_VERSION_MINOR})
		set(CPACK_PACKAGE_VERSION_PATCH ${LIBMONETRA_VERSION_PATCH})
		include(CPack)
	endif ()
endif ()
