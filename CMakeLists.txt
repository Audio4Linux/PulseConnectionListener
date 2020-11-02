cmake_minimum_required(VERSION 3.15)
project(pa_conn_watcher)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include_directories(.)

find_package(PkgConfig REQUIRED)

file(GLOB SOURCES *.h *.cpp *.hpp)

pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glibmm-2.4 giomm-2.4)
pkg_check_modules(PULSE REQUIRED IMPORTED_TARGET libpulse)

add_executable(pa_conn_watcher ${SOURCES})

target_include_directories(pa_conn_watcher PUBLIC ${PULSE_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})
target_link_libraries(pa_conn_watcher ${PULSE_LIBRARIES} PkgConfig::PULSE PkgConfig::GLIB)

set_target_properties(pa_conn_watcher PROPERTIES OUTPUT_NAME "pa_conn_watcher")
