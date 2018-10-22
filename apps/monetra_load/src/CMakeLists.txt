set(name monetra_load)

add_executable(${name}
	monetra_load.c
)

target_link_libraries(${name} PRIVATE
	LibMonetra::libmonetra
	Mstdlib::tls
)

if (LIBMONETRA_INSTALL_APPS)
	install(TARGETS ${name} RUNTIME DESTINATION bin)
	if (WIN32)
		set(EXE_NAME ${name})
		configure_file("${LIBMONETRA_TOPLEVEL_DIR}/resources/jumpscript.bat.in" ${name}.bat)
		install(PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/${name}.bat" DESTINATION .)
	endif ()
endif ()
