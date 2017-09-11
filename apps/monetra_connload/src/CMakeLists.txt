set(name monetra_connload)

add_executable(${name}
	monetra_connload.c
)

target_link_libraries(${name} PRIVATE
	LibMonetra::libmonetra
	Mstdlib::tls
)
target_include_directories(${name} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)
set_target_properties(${name} PROPERTIES POSITION_INDEPENDENT_CODE True)

if (LIBMONETRA_INSTALL_APPS)
	install(TARGETS ${name} RUNTIME DESTINATION bin)
endif ()
