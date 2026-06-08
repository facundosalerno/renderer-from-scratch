#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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

typedef struct {
    uint8_t bgra[4];
    uint8_t bytespp;
} TGAColor;

static const TGAColor tga_white  = {{255, 255, 255, 255}, 4}; // attention, BGRA order
static const TGAColor tga_green  = {{  0, 255,   0, 255}, 4};
static const TGAColor tga_red    = {{  0,   0, 255, 255}, 4};
static const TGAColor tga_blue   = {{255, 128,  64, 255}, 4};
static const TGAColor tga_yellow = {{  0, 200, 255, 255}, 4};

typedef enum {
    TGA_GRAYSCALE = 1,
    TGA_RGB = 3,
    TGA_RGBA = 4
} TGAFormat;

typedef struct {
    int w;
    int h;
    TGAFormat  bpp;
    uint8_t *data;
} TGAImage;

TGAImage* tga_create(int w, int h, TGAFormat bpp);
TGAImage* tga_read(const char *filename);
void tga_free(TGAImage *img);
int tga_write(const TGAImage *img, const char *filename, int vflip, int rle);
void tga_flip_horizontally(TGAImage *img);
void tga_flip_vertically(TGAImage *img);
TGAColor tga_get(const TGAImage *img, int x, int y);
void tga_set(TGAImage *img, int x, int y, TGAColor c);