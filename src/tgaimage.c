#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tgaimage.h"

/**
 * Port directo de https://github.com/ssloy/tinyrenderer/blob/706b2dfecff65daeb93de568ee2c2bd87f277860/tgaimage.cpp
 */

static int load_rle_data(TGAImage* fb, FILE *in) {
    size_t pixelcount = fb->w * fb->h;
    size_t currentpixel = 0;
    size_t currentbyte  = 0;
    uint8_t colorbuffer[4];
    int bpp = fb->bpp;
    do {
        uint8_t chunkheader = fgetc(in);
        if (ferror(in)) {
            printf("an error occured while reading the data\n");
            return 0;
        }
        if (chunkheader < 128) {
            chunkheader++;
            for (int i = 0; i < chunkheader; i++) {
                fread(colorbuffer, 1, bpp, in);
                if (ferror(in)) {
                    printf("an error occured while reading the header\n");
                    return 0;
                }
                for (int t = 0; t < bpp; t++)
                    fb->data[currentbyte++] = colorbuffer[t];
                currentpixel++;
                if (currentpixel > pixelcount) {
                    printf("too many pixels read\n");
                    return 0;
                }
            }
        } else {
            chunkheader -= 127;
            fread(colorbuffer, 1, bpp, in);
            if (ferror(in)) {
                printf("an error occured while reading the header\n");
                return 0;
            }
            for (int i = 0; i < chunkheader; i++) {
                for (int t = 0; t < bpp; t++)
                    fb->data[currentbyte++] = colorbuffer[t];
                currentpixel++;
                if (currentpixel > pixelcount) {
                    printf("too many pixels read\n");
                    return 0;
                }
            }
        }
    } while (currentpixel < pixelcount);
    return 1;
}

static int unload_rle_data(const TGAImage* fb, FILE *out) {
    const uint8_t max_chunk_length = 128;
    size_t npixels = fb->w * fb->h;
    int bpp = fb->bpp;
    size_t curpix = 0;
    while (curpix < npixels) {
        size_t chunkstart = curpix * bpp;
        size_t curbyte = curpix * bpp;
        uint8_t run_length = 1;
        int raw = 1;
        while (curpix + run_length < npixels && run_length < max_chunk_length) {
            int succ_eq = 1;
            for (int t = 0; succ_eq && t < bpp; t++)
                succ_eq = (fb->data[curbyte + t] == fb->data[curbyte + t + bpp]);
            curbyte += bpp;
            if (1 == run_length)
                raw = !succ_eq;
            if (raw && succ_eq) {
                run_length--;
                break;
            }
            if (!raw && !succ_eq)
                break;
            run_length++;
        }
        curpix += run_length;
        fputc(raw ? run_length - 1 : run_length + 127, out);
        if (ferror(out)) return 0;
        fwrite(fb->data + chunkstart, 1, raw ? run_length * bpp : bpp, out);
        if (ferror(out)) return 0;
    }
    return 1;
}

TGAImage* tga_create(int w, int h, TGAFormat bpp) {
    TGAImage* fb = malloc(sizeof(TGAImage));
    fb->w = w;
    fb->h = h;
    fb->bpp = bpp;
    fb->data = calloc(w * h * bpp, 1);
    return fb;
}

void tga_free(TGAImage* fb){
    if(fb != NULL){
        if(fb->data != NULL){
            free(fb->data);
            fb->data = NULL;
        }
        free(fb);
    }
}

TGAImage* tga_read(const char *filename){
    FILE *in = fopen(filename, "rb");
    if (!in) {
        printf("can't open file %s\n", filename);
        return NULL;
    }
    TGAHeader header;
    fread(&header, 1, sizeof(header), in);
    if (ferror(in)) {
        printf("an error occured while reading the header\n");
        fclose(in);
        return NULL;
    }
    const int w = header.width;
    const int h = header.height;
    const TGAFormat bpp = header.bitsperpixel>>3;
    if (w<=0 || h<=0 || (bpp!=TGA_GRAYSCALE && bpp!=TGA_RGB && bpp!=TGA_RGBA)) {
        printf("bad bpp (or width/height) value\n");
        fclose(in);
        return NULL;
    }
    size_t nbytes = bpp * w * h;
    TGAImage* fb = tga_create(w, h, bpp);
    if(fb == NULL || fb->data == NULL){
        printf("could not create TGAImage object\n");
        fclose(in);
        return NULL;
    }

    if (3==header.datatypecode || 2==header.datatypecode) {
        fread(fb->data, 1, nbytes, in);
        if (ferror(in)) {
            printf("an error occured while reading the data\n");
            tga_free(fb);
            fclose(in);
            return NULL;
        }
    } else if (10==header.datatypecode||11==header.datatypecode) {
        if (!load_rle_data(fb, in)) {
            printf("an error occured while reading the data\n");
            tga_free(fb);
            fclose(in);
            return NULL;
        }
    } else {
        printf("unknown file format %d\n", header.datatypecode);
        tga_free(fb);
        fclose(in);
        return NULL;
    }
    if (!(header.imagedescriptor & 0x20))
        tga_flip_vertically(fb);
    if (header.imagedescriptor & 0x10)
        tga_flip_horizontally(fb);
    printf("%d x %d / %d\n", w, h, bpp*8);
    fclose(in);
    return fb;
}

int tga_write(const TGAImage* fb, const char *filename, bool vflip, bool rle) {
    static const uint8_t developer_area_ref[4] = {0, 0, 0, 0};
    static const uint8_t extension_area_ref[4] = {0, 0, 0, 0};
    static const uint8_t footer[18] = {'T','R','U','E','V','I','S','I','O','N','-','X','F','I','L','E','.','\0'};
    FILE *out = fopen(filename, "wb");
    if (!out) {
        printf("can't open file %s\n", filename);
        return 0;
    }
    TGAHeader header = {0};
    header.bitsperpixel = fb->bpp << 3;
    header.width  = fb->w;
    header.height = fb->h;
    header.datatypecode = (fb->bpp == TGA_GRAYSCALE ? (rle ? 11 : 3) : (rle ? 10 : 2));
    header.imagedescriptor = vflip ? 0x00 : 0x20;
    fwrite(&header, 1, sizeof(header), out);
    if (ferror(out)) goto err;
    if (!rle) {
        fwrite(fb->data, 1, fb->w * fb->h * fb->bpp, out);
        if (ferror(out)) goto err;
    } else if (!unload_rle_data(fb, out)) goto err;
    fwrite(developer_area_ref, 1, sizeof(developer_area_ref), out);
    if (ferror(out)) goto err;
    fwrite(extension_area_ref, 1, sizeof(extension_area_ref), out);
    if (ferror(out)) goto err;
    fwrite(footer, 1, sizeof(footer), out);
    if (ferror(out)) goto err;
    fclose(out);
    return 1;
err:
    printf("can't dump the tga file\n");
    fclose(out);
    return 0;
}

void tga_flip_horizontally(TGAImage* fb) {
    int bpp = fb->bpp;
    for (int i = 0; i < fb->w / 2; i++)
        for (int j = 0; j < fb->h; j++)
            for (int b = 0; b < bpp; b++) {
                uint8_t tmp = fb->data[(i + j * fb->w) * bpp + b];
                fb->data[(i + j * fb->w) * bpp + b] = fb->data[(fb->w - 1 - i + j * fb->w) * bpp + b];
                fb->data[(fb->w - 1 - i + j * fb->w) * bpp + b] = tmp;
            }
}

void tga_flip_vertically(TGAImage* fb) {
    int bpp = fb->bpp;
    for (int i = 0; i < fb->w; i++)
        for (int j = 0; j < fb->h / 2; j++)
            for (int b = 0; b < bpp; b++) {
                uint8_t tmp = fb->data[(i + j * fb->w) * bpp + b];
                fb->data[(i + j * fb->w) * bpp + b] = fb->data[(i + (fb->h - 1 - j) * fb->w) * bpp + b];
                fb->data[(i + (fb->h - 1 - j) * fb->w) * bpp + b] = tmp;
            }
}

Color tga_get(const TGAImage* fb, Point point) {
    Color color = {{0, 0, 0, 0}, 0};
    if (!fb->data || point.x < 0 || point.y < 0 || point.x >= fb->w || point.y >= fb->h)
        return color;
    color.bytespp = fb->bpp;
    const uint8_t *p = fb->data + (point.x + point.y * fb->w) * fb->bpp;
    for (int i = 0; i < (int)fb->bpp; i++)
        color.bgra[i] = p[i];
    return color;
}

void tga_set(TGAImage* fb, Point point, Color c) {
    if (!fb->data || point.x < 0 || point.y < 0 || point.x >= fb->w || point.y >= fb->h) return;
    memcpy(fb->data + (point.x + point.y * fb->w) * fb->bpp, c.bgra, fb->bpp);
}

void tga_draw_line(TGAImage* fb, Line line, Color color){
    for(unsigned int i=0; i < line.len; i++){
        tga_set(fb, line.points[i], color);
    }
}