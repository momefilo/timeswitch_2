file(GLOB FILES *.c *.h)
add_library(st_7735 ${FILES})
target_link_libraries(st_7735
	pico_stdlib
	hardware_spi
	hardware_gpio
)

target_include_directories(st_7735 PUBLIC ./)
