cmake_minimum_required(VERSION 2.8)
project(libgsdl)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB glib-2.0)
set(CMAKE_C_FLAGS "-std=gnu99 -ggdb3 -Wall")

add_library(gsdl SHARED
	libgsdl/tokenizer.c
)
include_directories(${GLIB_INCLUDE_DIRS})
target_link_libraries(gsdl m ${GLIB_LIBRARIES})
