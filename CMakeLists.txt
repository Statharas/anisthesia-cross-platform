cmake_minimum_required(VERSION 3.20)

project(anisthesia
	VERSION 1.0.0
	DESCRIPTION "Media detection library"
	HOMEPAGE_URL https://github.com/erengy/anisthesia
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ANISTHESIA_BUILD_TESTS "Build Anisthesia tests" ${PROJECT_IS_TOP_LEVEL})
option(ANISTHESIA_ENABLE_LINUX_MPRIS "Build the Linux MPRIS backend when Qt DBus is available" ON)

if (ANISTHESIA_ENABLE_LINUX_MPRIS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
	find_package(Qt6 QUIET COMPONENTS Core DBus)
endif()

add_library(anisthesia INTERFACE)

target_include_directories(anisthesia INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(anisthesia INTERFACE
	src/matroska.cpp
	src/platform.cpp
	src/player.cpp
	src/util.cpp
)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_compile_definitions(anisthesia INTERFACE
		_UNICODE
		UNICODE
	)

	target_sources(anisthesia INTERFACE
		src/win_open_files.cpp
		src/win_platform.cpp
		src/win_strategies.cpp
		src/win_ui_automation.cpp
		src/win_util.cpp
		src/win_windows.cpp
	)
endif()

if (ANISTHESIA_ENABLE_LINUX_MPRIS AND NOT CMAKE_SYSTEM_NAME STREQUAL "Windows" AND TARGET Qt6::DBus)
	target_sources(anisthesia INTERFACE
		src/linux_mpris.cpp
	)

	target_compile_definitions(anisthesia INTERFACE ANISTHESIA_HAS_LINUX_MPRIS)
	target_link_libraries(anisthesia INTERFACE Qt6::Core Qt6::DBus)
endif()

if (ANISTHESIA_BUILD_TESTS)
	enable_testing()
	add_subdirectory(test)
endif()
