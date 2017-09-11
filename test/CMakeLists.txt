enable_testing()

if (LIBMONETRA_BUILD_TESTS)
	find_package(Check)
	find_package(Valgrind)
endif ()

if (TARGET Check::check AND LIBMONETRA_BUILD_TESTS)
	include(CheckLibraryExists)

	message(STATUS "Building tests... Enabled")

	if (TARGET Valgrind::valgrind AND LIBMONETRA_USE_VALGRIND)
		message(STATUS "Running tests with Valgrind... Enabled")
	else ()
		message(STATUS "Running tests with Valgrind... Disabled")
	endif ()

	set(tests
		check_monetra.c
		check_monetra_legacy.c
	)

	# List of link dependencies for each test.
	set(test_deps LibMonetra::libmonetra Check::check)

	# Seems like libcheck needs these on Linux for some reason
	check_library_exists(rt clock_gettime "" HAVE_RT_LIB)
	if (HAVE_RT_LIB)
		list(APPEND test_deps "rt")
	endif ()
	check_library_exists(m floor "" HAVE_M_LIB)
	if (HAVE_M_LIB)
		list(APPEND test_deps "m")
	endif ()
	check_library_exists(pthread pthread_create "" HAVE_PTHREAD_LIB)
	if (HAVE_PTHREAD_LIB)
		list(APPEND test_deps "pthread")
	endif ()

	foreach (test_src ${tests})
		get_filename_component(test_prog ${test_src} NAME_WE)

		add_executable(${test_prog} ${test_src})
		target_link_libraries(${test_prog} PRIVATE ${test_deps})

		if (TARGET Valgrind::valgrind AND LIBMONETRA_USE_VALGRIND)
			add_test(
				NAME ${test_prog}_memcheck
				COMMAND Valgrind::valgrind "--leak-check=full"
				                           "--show-reachable=yes"
				                           "--suppressions=${PROJECT_SOURCE_DIR}/valgrind/check.suppressions"
				                           "--log-file=${PROJECT_BINARY_DIR}/test/${test_prog}-valgrind.log"
				                           $<TARGET_FILE:${test_prog}>
			)
		else ()
			add_test(
				NAME    ${test_prog}
				COMMAND ${test_prog}
			)
		endif ()
	endforeach ()

	# Make Check target
	add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
else ()
	message(STATUS "Building tests... Disabled")
endif ()
