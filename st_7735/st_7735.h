#ifndef my_st_7735_h
#define my_st_7735_h 1

#include "pico/stdlib.h"
#include "pico/double.h"
#include "font6x8.h"
#include "font7x11.h"
#include "font10x16.h"
#include "font_12x12.h"
#include "font12x16.h"
#include "font14x20.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include <string.h>

#define SPI_PORT spi1//spi0
#define PIN_SCK	14//2
#define PIN_SDA	15//3
#define PIN_A0  12//4
#define PIN_RST 11//6
#define PIN_CS  13//5

#define HORIZONTAL 0
#define VERTICAL 1

void st7735_init();
/* Setzt die Orientierung, Parameter sind HORIZONTAL und VERTICAL */
void setOrientation(uint8_t ori);
void setSeColor(uint16_t color);
void setFgColor(uint16_t color);
void setBgColor(uint16_t color);
void clearScreen();

void writeText7x11(uint8_t *pos, char *text, bool sel, bool matrix);
void writeText6x8(uint8_t *pos, char *text, bool sel, bool matrix);
void writeText10x16(uint8_t *pos, char *text, bool sel, bool matrix);
void writeText12x12(uint8_t *pos, char *text, int len, bool sel, bool matrix);
void writeText12x16(uint8_t *pos, char *text, bool sel, bool matrix);
void writeText14x20(uint8_t *pos, char *text, bool sel, bool matrix);

void drawRect(uint8_t *area, uint8_t *data);
void paintRect(uint8_t *area, uint16_t color);
void paintPixel(uint8_t x, uint8_t y, uint16_t color);
void paintLine(uint8_t x, uint8_t y, uint16_t a, uint8_t begin, uint8_t end, uint8_t width, uint16_t color);
#endif
