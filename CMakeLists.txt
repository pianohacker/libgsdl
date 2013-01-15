cmake_minimum_required(VERSION 2.8)
project(libgsdl)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB glib-2.0 gobject-2.0)
set(CMAKE_C_FLAGS "-std=gnu99 -g -Wall")

add_library(gsdl SHARED
	libgsdl/parser.c
	libgsdl/syntax.c
	libgsdl/tokenizer.c
	libgsdl/types.c
)
include_directories(${GLIB_INCLUDE_DIRS})
target_link_libraries(gsdl m ${GLIB_LIBRARIES})

enable_testing()
file(GLOB TESTS test/test-*.c)
include_directories(libgsdl ${GLIB_INCLUDE_DIRS})

foreach(TESTFILE ${TESTS})
	get_filename_component(TESTPROG ${TESTFILE} NAME_WE)
	add_executable(${TESTPROG} ${TESTFILE})
	target_link_libraries(${TESTPROG} gsdl ${GLIB_LIBRARIES})
	add_test(${TESTPROG} ${TESTPROG})
endforeach(TESTFILE)
