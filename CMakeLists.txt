cmake_minimum_required(VERSION 3.4.3)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/")

include(DefaultBuildType)

project(LibMonetra
	VERSION   8.2.0
	LANGUAGES C
)

include(EnableWarnings)
include(Harden)
include(InstallExports) # Provides install_exports()
include(InstallDepLibs) # Provides install_deplibs() and install_system_deplibs()
include(SetPackageName) # Provides set_package_name()

include(AddressSanitizer) # Provides M_ASAN option, if supported by compiler

# Version info.
set(LIBMONETRA_VERSION_MAJOR    ${PROJECT_VERSION_MAJOR})
set(LIBMONETRA_VERSION_MINOR    ${PROJECT_VERSION_MINOR})
set(LIBMONETRA_VERSION_PATCH    ${PROJECT_VERSION_PATCH})
set(LIBMONETRA_VERSION_STRING   ${PROJECT_VERSION})

set(LIBMONETRA_SOVERSION_STRING ${LIBMONETRA_VERSION_MAJOR})

# Set same version variable as find_package, in parent scope, so that chain-builds
# have access to the version info.
if (NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	set(${PROJECT_NAME}_VERSION ${PROJECT_VERSION} PARENT_SCOPE)
endif ()

option(LIBMONETRA_STATIC          "Build as a static library"                              FALSE)
option(LIBMONETRA_STATIC_PIC      "Build the static library as PIC (position independent)" FALSE)
option(LIBMONETRA_BUILD_APPS      "Build apps"                                             TRUE)
option(LIBMONETRA_BUILD_DOCS      "Build doxygen docs"                                     TRUE)
option(LIBMONETRA_BUILD_TESTS     "Build tests"                                            TRUE)
option(LIBMONETRA_INSTALL_HEADERS "Install headers"                                        TRUE)
option(LIBMONETRA_INSTALL_LIBS    "Install libraries"                                      TRUE)
option(LIBMONETRA_INSTALL_EXPORTS "Install exports"                                        ${LIBMONETRA_INSTALL_LIBS})
option(LIBMONETRA_INSTALL_APPS    "Install apps"                                           TRUE)
option(LIBMONETRA_USE_VALGRIND    "Run tests with valgrind when running 'make check'"      TRUE)
mark_as_advanced(FORCE LIBMONETRA_INSTALL_EXPORTS)


# Keep build organized.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set RPATH so that executables can find their libs in the install package.
if (NOT WIN32 AND NOT APPLE)
	set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib:\$ORIGIN/")
endif ()

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
set(local_mstdlib TRUE)
if (NOT TARGET Mstdlib::base OR NOT TARGET Mstdlib::formats OR NOT TARGET Mstdlib::thread OR NOT TARGET Mstdlib::io OR NOT TARGET Mstdlib::tls)
	if (IS_DIRECTORY "${LIBMONETRA_TOPLEVEL_DIR}/thirdparty/mstdlib")
			# Override the following settings in the subdirectory, and hide them from the user.
			set(MSTDLIB_BUILD_TESTS     FALSE CACHE INTERNAL "")
			set(MSTDLIB_BUILD_THREAD    TRUE  CACHE INTERNAL "")
			set(MSTDLIB_BUILD_IO        TRUE  CACHE INTERNAL "")
			set(MSTDLIB_BUILD_TLS       TRUE  CACHE INTERNAL "")
			set(MSTDLIB_BUILD_LOG       FALSE CACHE INTERNAL "")
			set(MSTDLIB_BUILD_SQL       FALSE CACHE INTERNAL "")

			set(MSTDLIB_INSTALL_LIBS    TRUE  CACHE INTERNAL "")
			set(MSTDLIB_INSTALL_EXPORTS TRUE  CACHE INTERNAL "")
			set(MSTDLIB_INSTALL_HEADERS TRUE  CACHE INTERNAL "")

			add_subdirectory("${LIBMONETRA_TOPLEVEL_DIR}/thirdparty/mstdlib")
	else ()
		find_package(Mstdlib REQUIRED COMPONENTS base formats thread io tls)
		set(local_mstdlib FALSE)
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

# Set build flags for libmonetra library and apps, if we're not being chain-built by somebody else.
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	add_definitions(
		# Suppress annoying deprecation warnings on newer windows.
		-D_CRT_SECURE_NO_DEPRECATE
		-D_CRT_NONSTDC_NO_DEPRECATE
		# Leave out little-used parts of windows.h (can still get them by including the specific headers you need).
		-DWIN32_LEAN_AND_MEAN
		-DNOMINMAX
		# Explicitly request Windows Vista or newer feature level.
		-D_WIN32_WINNT=0x0600
	)
endif ()

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

if (LIBMONETRA_INSTALL_EXPORTS)
	install_exports(LibMonetra "${LIBMONETRA_INSTALL_LOCATION_SHARE}/LibMonetra/cmake" Mstdlib:${Mstdlib_VERSION})
endif ()

# If we're not being chain-built, and install is enabled, add code to allow package creation.
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR AND LIBMONETRA_INSTALL_LIBS)
	if (LIBMONETRA_INSTALL_LIBS)
		if (local_mstdlib OR (LIBMONETRA_BUILD_APPS AND LIBMONETRA_INSTALL_APPS))
			# Install deplibs of mstdlib.
			find_package(OpenSSL REQUIRED)
			install_deplibs(${LIBMONETRA_INSTALL_LOCATION_LIBS} bin
				OpenSSL::SSL
				OpenSSL::Crypto
			)
			if (WIN32 AND CMAKE_HOST_SYSTEM_NAME MATCHES "CYGWIN")
				# If we're cross-compiling for windows from cygwin, zlib is also a dependency.
				find_package(ZLIB REQUIRED)
				install_deplibs(${LIBMONETRA_INSTALL_LOCATION_LIBS} bin
					ZLIB::ZLIB
				)
			endif ()

			# If we're not using a chain-built mstdlib, but still need the runtime libs for
			# libmonetra's apps, install the DLL's.
			if (NOT local_mstdlib)
				install_deplibs(${LIBMONETRA_INSTALL_LOCATION_LIBS} bin
					Mstdlib::tls
					Mstdlib::io
					Mstdlib::thread
					Mstdlib::formats
					Mstdlib::base
				)
			endif ()
		endif ()

		# Install required system libs, if any (MSVC redistributables or MinGW runtimes, for example).
		install_system_deplibs(${LIBMONETRA_INSTALL_LOCATION_LIBS} bin)
	endif ()


	set(CPACK_GENERATOR)

	if (WIN32)
		# Check to see if Wix is available. If it is, enable wix packaging by default.
		if (EXISTS "$ENV{WIX}" OR EXISTS "${CPACK_WIX_ROOT}")
			set(wix_default TRUE)
		else ()
			set(wix_default FALSE)
		endif ()

		option(M_PACKAGE_WIX "Use Wix Toolset to generate MSI installer?" ${wix_default})
		if (M_PACKAGE_WIX)
			list(APPEND CPACK_GENERATOR WIX)

			# LibMonetra builds that share the same GUID are allowed to replace one another on an install (i.e., upgrade in place).
			# Since this is a library, need to allow 64-bit and 32-bit versions to both be installed at the same time on Windows.
			if (CMAKE_SIZEOF_VOID_P EQUAL 8)
				set(CPACK_WIX_UPGRADE_GUID 85E1F2BD-7502-40E4-9ECA-0BBE5D7119C4)
				set(CPACK_PACKAGE_NAME "${PROJECT_NAME} (64-bit)")
			else ()
				set(CPACK_WIX_UPGRADE_GUID 2FB87D38-2553-4FBC-B9CB-7D188B9F8199)
				set(CPACK_PACKAGE_NAME "${PROJECT_NAME} (32-bit)")
			endif ()

			# Set these variables to add branding graphics to the Windows installer.
			set(img_dir                "${CMAKE_CURRENT_SOURCE_DIR}/resources")
			set(CPACK_WIX_PRODUCT_ICON "${img_dir}/libmonetra.ico")
			set(CPACK_WIX_UI_BANNER    "${img_dir}/wix_banner.bmp") # image at top of all pages besides welcome and completion, 493 x 58 px
			set(CPACK_WIX_UI_DIALOG    "${img_dir}/wix_dialog.bmp") # background image on welcome and completion pages, 493 x 312 px

			# These variables determine what info shows up for LibMonetra in add/remove programs.
			set(CPACK_WIX_PROPERTY_ARPHELPLINK     https://github.com/monetra/libmonetra)
			set(CPACK_WIX_PROPERTY_ARPURLINFOABOUT https://monetra.github.io/libmonetra)

			# Add installed LibMonetra to the package registry.
			set(CPACK_WIX_CMAKE_PACKAGE_REGISTRY LibMonetra)

			# Don't want to put version number in default installation directory.
			set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
		endif ()
	elseif (APPLE)
		# TODO: add DMG package here, after we get Mstdlib and LibMonetra reorganized as frameworks on macos
		list(APPEND CPACK_GENERATOR ZIP)
	else ()
		# Self-extracting tarball (see https://github.com/drbenmorgan/StudyPack).
		option(M_PACKAGE_STGZ "Generate self-extracting tarball installer?" TRUE)
		if (M_PACKAGE_STGZ)
			list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/postinstall/stgz_script")
			list(APPEND CPACK_GENERATOR STGZ)
		endif ()

		# Check to see if rpm is available.
		find_program(RPMBUILD_EXECUTABLE rpmbuild)
		mark_as_advanced(FORCE RPMBUILD_EXECUTABLE)
		if (RPMBUILD_EXECUTABLE)
			option(M_PACKAGE_RPM "Add RPM to packaging output" FALSE)
		else ()
			set(M_PACKAGE_RPM FALSE)
		endif ()
		if (M_PACKAGE_RPM)
			list(APPEND CPACK_GENERATOR RPM)
			set(CPACK_RPM_PACKAGE_URL         https://github.com/monetra/libmonetra)
			set(CPACK_RPM_PACKAGE_RELOCATABLE TRUE)
		endif ()
	endif ()


	if (CPACK_GENERATOR)
		message(STATUS "Packagers enabled: ${CPACK_GENERATOR}")
	endif ()
	set(CPACK_VERBATIM_VARIABLES TRUE)

	set(CPACK_PACKAGE_VENDOR              "Main Street Softworks")
	set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Monetra Protocol Library (libmonetra)")
	set(CPACK_RESOURCE_FILE_README        "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

	string(TIMESTAMP COPYRIGHT_YEAR "%Y")
	configure_file(license.txt.in license.txt @ONLY)
	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_BINARY_DIR}/license.txt")
	set(CPACK_PACKAGE_ICON          "${CMAKE_CURRENT_SOURCE_DIR}/resources/libmonetra.ico")

	set_package_name(${PROJECT_NAME} ${PROJECT_VERSION})

	set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
	set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
	set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

	# Add "dist" target - builds, packages, then runs any post-package steps (note that any commands will be run in the root build dir).
	set(postpkg
		COMMAND "${CMAKE_COMMAND}" --build . --config $<CONFIG> # Build "all" target
		COMMAND "${CMAKE_CPACK_COMMAND}"                        # Run cpack
	)

	if (STGZ IN_LIST CPACK_GENERATOR)
		# We'd rather have the self-extracting tarball's extension be *.run, instead of *.sh.
		list(APPEND postpkg
			COMMAND "${CMAKE_COMMAND}" -E echo "PostPkg: Renaming ${CPACK_PACKAGE_FILE_NAME}.sh to ${CPACK_PACKAGE_FILE_NAME}.run"
			COMMAND "${CMAKE_COMMAND}" -E rename "${CPACK_PACKAGE_FILE_NAME}.sh" "${CPACK_PACKAGE_FILE_NAME}.run"
		)
	endif ()
	if (WIX IN_LIST CPACK_GENERATOR)
		# Sign the .msi file after creating it, if a signing certificate name or file path has been provided.
		include(CodeSign)
		code_sign_is_enabled(can_sign)
		if (can_sign)
			set(M_SIGN_UAC_URL "https://github.com/monetra/libmonetra")
			set(FILES_TO_SIGN "${CMAKE_CURRENT_BINARY_DIR}/${CPACK_PACKAGE_FILE_NAME}.msi")
			configure_file(CMakeModules/CodeSignScript.cmake.in sign_wix_msi.cmake)
			list(APPEND postpkg
				COMMAND "${CMAKE_COMMAND}" -E echo "PostPkg: Code signing ${CPACK_PACKAGE_FILE_NAME}.msi"
				COMMAND "${CMAKE_COMMAND}" -P "sign_wix_msi.cmake"
			)
		endif ()
	endif ()

	add_custom_target(dist
		${postpkg}
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		VERBATIM
		USES_TERMINAL
	)

	include(CPack)
endif ()
