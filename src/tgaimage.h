#pragma once

#include <stdbool.h>

#include "geometry.h"
#include "color.h"
/**
 * Port directo de https://github.com/ssloy/tinyrenderer/blob/706b2dfecff65daeb93de568ee2c2bd87f277860/tgaimage.h
 */

#pragma pack(push, 1)
typedef struct {
    uint8_t idlength;
    uint8_t colormaptype;
    uint8_t datatypecode;
    uint16_t colormaporigin;
    uint16_t colormaplength;
    uint8_t colormapdepth;
    uint16_t x_origin;
    uint16_t y_origin;
    uint16_t width;
    uint16_t height;
    uint8_t bitsperpixel;
    uint8_t imagedescriptor;
} TGAHeader;
#pragma pack(pop)

typedef enum {
    TGA_GRAYSCALE = 1,
    TGA_RGB = 3,
    TGA_RGBA = 4
} TGAFormat;

typedef struct {
    int w;
    int h;
    TGAFormat  bpp;
    uint8_t* data;
} TGAImage;

TGAImage* tga_create(int w, int h, TGAFormat bpp);
TGAImage* tga_read(const char* filename);
void tga_free(TGAImage* fb);

int tga_write(const TGAImage* fb, const char* filename, bool vflip, bool rle);

void tga_flip_horizontally(TGAImage* fb);
void tga_flip_vertically(TGAImage* fb);

Color tga_get(const TGAImage* fb, Point point);
void tga_set(TGAImage* fb, Point point, Color c);

void tga_draw_line(TGAImage* fb, Line line, Color color);