project(libmonetra C)

if (LIBMONETRA_STATIC)
	set(libtype STATIC)
	set(libdefs PUBLIC LIBMONETRA_STATIC)
else ()
	set(libtype SHARED)
	set(libdefs PRIVATE LIBMONETRA_INTERNAL)
endif ()

# Build the library.
set(sources
	monetra_conn.c
	monetra_conn_event.c
	monetra_deprecated.c
	monetra_init.c
	monetra_legacy.c
	monetra_parse.c
	monetra_trans.c
	monetra_trans_structure.c
)

add_library(${PROJECT_NAME} ${libtype}
	${sources}
)

target_compile_definitions(${PROJECT_NAME}
	${libdefs}
)

target_link_libraries(${PROJECT_NAME} PUBLIC
	Mstdlib::tls
	Mstdlib::io
	Mstdlib::formats
)

target_include_directories(${PROJECT_NAME}
	PUBLIC  "$<INSTALL_INTERFACE:include/>"
	        "$<BUILD_INTERFACE:${LIBMONETRA_TOPLEVEL_DIR}/include/>"
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)

if (NOT ANDROID)
	set_target_properties(${PROJECT_NAME} PROPERTIES
		VERSION   ${LIBMONETRA_VERSION_STRING}
		SOVERSION ${LIBMONETRA_SOVERSION_STRING}
	)
endif ()

# Setup library output name.
#
# We actually want to make sure there is always a "lib" prefix for legacy reasons.
# We've always distributed libmonetra on windows as libmonetra.dll not monetra.dll
# as is classic for windows libraries.
set_target_properties(${PROJECT_NAME} PROPERTIES
	OUTPUT_NAME   monetra
	PREFIX        "lib"
	IMPORT_PREFIX "lib"
)

# On windows, make a version that uses the stdcall calling convention instead of cdecl.
if (WIN32 AND NOT LIBMONETRA_STATIC)
	add_library(${PROJECT_NAME}_stdcall SHARED
		${sources}
	)
	target_link_libraries(${PROJECT_NAME}_stdcall
		PUBLIC Mstdlib::formats Mstdlib::io Mstdlib::tls
	)
	target_include_directories(${PROJECT_NAME}_stdcall
		PUBLIC  "$<INSTALL_INTERFACE:include/>"
				"$<BUILD_INTERFACE:${LIBMONETRA_TOPLEVEL_DIR}/include/>"
		PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
	)
	target_compile_definitions(${PROJECT_NAME}_stdcall
		PRIVATE LIBMONETRA_INTERNAL
		        LIBMONETRA_SPEC_STDCALL
	)
	set_target_properties(${PROJECT_NAME}_stdcall PROPERTIES
		OUTPUT_NAME   monetra_stdcall
		PREFIX        "lib"
		IMPORT_PREFIX "lib"
	)

	if (MINGW)
		target_link_libraries(${PROJECT_NAME}_stdcall
			PRIVATE -Wl,--add-stdcall-alias
		)
	endif ()

	convert_mingw_implibs_to_vs(libmonetra libmonetra_stdcall)
endif ()

# Installation rules.
if (LIBMONETRA_INSTALL_LIBS)
	set(targets ${PROJECT_NAME})
	if (WIN32)
		list(APPEND targets ${PROJECT_NAME}_stdcall)
	endif ()

	install(TARGETS ${targets} EXPORT LibMonetra ${lib_dest})
endif ()


# For linking with other internal libs.
add_library(LibMonetra::libmonetra ALIAS ${PROJECT_NAME})

# TODO: get rid of legacy variables once everything is updated to use alias targets
set(LIBMONETRA_LIBRARIES LibMonetra::libmonetra CACHE INTERNAL "")
