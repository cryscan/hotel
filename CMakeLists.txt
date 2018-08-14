cmake_minimum_required(VERSION 2.8)
project(hotel)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall -Werror -pedantic -std=c11 -lm")

add_library(hotel hotel-ll.c hotel-db.c hotel-mt.c)

add_executable(hotel-demo hotel-main.c)
target_link_libraries(hotel-demo hotel)

add_executable(list-test list-test.c)
target_link_libraries(list-test hotel)

add_executable(set-test set-test.c)
target_link_libraries(set-test hotel)

# Find GTK3 for GUI.
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_libraries(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

set(CMAKE_C_FLAGS "-std=c11 -lm")
add_executable(hotel-app hotel-app.c resource.c)
target_link_libraries(hotel-app hotel ${GTK3_LIBRARIES})