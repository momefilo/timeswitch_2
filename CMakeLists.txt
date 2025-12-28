cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(timeswitch_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_executable(timeswitch_2 timeswitch_2.c )

# Bibliotheksverzeichnisse
add_subdirectory(./onewire_library)
add_subdirectory(./momefilo_flash)
add_subdirectory(./buttons)
add_subdirectory(./st_7735)

target_link_libraries(timeswitch_2
	onewire_library
	momefilo_flash
	buttons
	st_7735
	pico_stdlib
	pico_time)
pico_add_extra_outputs(timeswitch_2)
