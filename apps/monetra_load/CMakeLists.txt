cmake_minimum_required(VERSION 3.2.1)

include(DefaultBuildType)

set(ver 1.0.0)
if (BUILD_NUM)
	set(ver ${ver}.${BUILD_NUM})
endif ()


project(MonetraLoad
	VERSION   ${ver}
	LANGUAGES C
)

include(Harden)

if (NOT TARGET Mstdlib::tls)
	find_package(Mstdlib REQUIRED COMPONENTS tls)
endif ()

if (NOT TARGET LibMonetra::libmonetra)
	find_package(LibMonetra REQUIRED)
endif ()

add_subdirectory(src)
