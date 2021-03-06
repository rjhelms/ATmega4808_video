#include <stdint.h>

#ifndef MEGA0_VIDEO_H
#define MEGA0_VIDEO_H

struct Video
{
    uint8_t X;
    uint8_t Y;
    volatile uint8_t *screen;
    volatile uint8_t *screen_line;
    volatile uint8_t color_bg;
    volatile uint16_t frame;
    volatile uint16_t field_line;
    uint16_t pixel_line;
    
    uint8_t scale;
    uint16_t picture_start;
    uint16_t picture_end;
    uint16_t border_width;

    volatile bool hbi = false;
    volatile bool vbi = false;
};

Video* setupResolution(uint8_t x_size, uint8_t y_size);
Video* setupVideo(uint8_t x_size, uint8_t y_size, volatile uint8_t *ptr, uint8_t background);

void setByte(uint8_t x, uint8_t y, uint8_t value);
void drawPixel(uint8_t x, uint8_t y, uint8_t color);
void fillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
void drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color);

void drawChar(uint8_t x, uint8_t y, unsigned char c, uint8_t fg, uint8_t bg, const unsigned char *f);
void drawChar(uint8_t x, uint8_t y, unsigned char c, const unsigned char *f);

uint8_t drawUnsignedInt(uint8_t x, uint8_t y, uint32_t val, uint8_t base, uint8_t fg, uint8_t bg, const unsigned char *f);
uint8_t drawUnsignedInt(uint8_t x, uint8_t y, uint32_t val, uint8_t fg, uint8_t bg, const unsigned char *f);
uint8_t drawSignedInt(uint8_t x, uint8_t y, int32_t val, uint8_t base, uint8_t fg, uint8_t bg, const unsigned char *f);
uint8_t drawSignedInt(uint8_t x, uint8_t y, int32_t val, uint8_t fg, uint8_t bg, const unsigned char *f);
uint8_t drawFloat(uint8_t x, uint8_t y, float val, uint8_t base, uint8_t precision, uint8_t fg, uint8_t bg, const unsigned char *f);
uint8_t drawFloat(uint8_t x, uint8_t y, float val, uint8_t precision, uint8_t fg, uint8_t bg, const unsigned char *f);
uint8_t drawStr(uint8_t x, uint8_t y, const char *str, uint8_t fg, uint8_t bg, const unsigned char *f);

void delayFrames(uint16_t frames);

void render_line5c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg);
void render_line6c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg);
void render_line7c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg);
void render_line8c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg);
void render_line9c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg);
void render_line10c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg);
void render_line11c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg);
void render_line12c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg);
void render_line13c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg);
void render_line14c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg);
void render_line15c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg);
void render_line16c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg);
void render_line20c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg);
void render_line24c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg);
void render_line28c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg);
void render_line32c(uint8_t x, volatile uint8_t *ptr, uint8_t color_bg);

#endif