cmake_minimum_required(VERSION 2.8)
project(LIBGSDL)
set(LIBGSDL_VERSION 0.1.0)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB glib-2.0 gobject-2.0)
set(CMAKE_C_FLAGS "-std=gnu99 -g -Wall")

add_library(gsdl SHARED
	libgsdl/parser.c
	libgsdl/syntax.c
	libgsdl/tokenizer.c
	libgsdl/types.c
)
set_target_properties(gsdl PROPERTIES
	SOVERSION 1
	VERSION 1.1
)
include_directories(${GLIB_INCLUDE_DIRS})
target_link_libraries(gsdl m ${GLIB_LIBRARIES})

#> Testing
enable_testing()
file(GLOB TESTS test/test-*.c)
include_directories(libgsdl ${GLIB_INCLUDE_DIRS})

foreach(TESTFILE ${TESTS})
	get_filename_component(TESTPROG ${TESTFILE} NAME_WE)
	add_executable(${TESTPROG} ${TESTFILE})
	target_link_libraries(${TESTPROG} gsdl ${GLIB_LIBRARIES})
	add_test(${TESTPROG} ${TESTPROG})
endforeach(TESTFILE)

#> Documentation
set(LIBDIR ${CMAKE_INSTALL_PREFIX}/lib CACHE STRING "Library installation path")
set(INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/include CACHE STRING "Header file installation path")
set(PKGCONFIGDIR ${CMAKE_INSTALL_PREFIX}/share/pkgconfig CACHE STRING "Pkg-config storage directory")

add_custom_command(OUTPUT docs/reference/libgsdl-docs.xml
	DEPENDS docs/reference/libgsdl-docs.sgml libgsdl/*.c libgsdl/*.h
	COMMAND gtkdoc-scan --module=libgsdl --source-dir=../../libgsdl/ --output-dir=.
	COMMAND gtkdoc-mkdb --sgml-mode --source-dir=../../libgsdl --output-format=xml --name-space=gsdl --main-sgml-file=libgsdl-docs.xml --module=libgsdl
	WORKING_DIRECTORY docs/reference
)
add_custom_target(docs
	COMMAND gtkdoc-mkhtml --path=../../../libgsdl/ libgsdl ../libgsdl-docs.sgml
	DEPENDS docs/reference/libgsdl-docs.xm
	WORKING_DIRECTORY docs/reference/html
)

#> Installation
install(TARGETS gsdl
	LIBRARY DESTINATION ${LIBDIR}
)

file(GLOB GSDL_HEADERS ${LIBGSDL_SOURCE_DIR}/libgsdl/*.h)
install(FILES ${GSDL_HEADERS}
	DESTINATION ${INCLUDEDIR}/libgsdl
)

configure_file(gsdl-1.0.pc.cmake gsdl-1.0.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/gsdl-1.0.pc
	DESTINATION ${PKGCONFIGDIR}
)
