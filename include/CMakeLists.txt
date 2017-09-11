if (LIBMONETRA_INSTALL_HEADERS)
	install(FILES
		monetra_api.h
		monetra.h
		libmonetra_exp.h
		libmonetra_legacy.h
		libmonetra_deprecated.h

		DESTINATION ${LIBMONETRA_INSTALL_LOCATION_HEADERS}
	)
endif ()
