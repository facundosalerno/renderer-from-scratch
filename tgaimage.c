#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tgaimage.h"

/**
 * Port directo de https://github.com/ssloy/tinyrenderer/blob/706b2dfecff65daeb93de568ee2c2bd87f277860/tgaimage.cpp
 */

static int load_rle_data(TGAImage *img, FILE *in) {
    size_t pixelcount = img->w * img->h;
    size_t currentpixel = 0;
    size_t currentbyte  = 0;
    uint8_t colorbuffer[4];
    int bpp = img->bpp;
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
                    img->data[currentbyte++] = colorbuffer[t];
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
                    img->data[currentbyte++] = colorbuffer[t];
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

static int unload_rle_data(const TGAImage *img, FILE *out) {
    const uint8_t max_chunk_length = 128;
    size_t npixels = img->w * img->h;
    int bpp = img->bpp;
    size_t curpix = 0;
    while (curpix < npixels) {
        size_t chunkstart = curpix * bpp;
        size_t curbyte = curpix * bpp;
        uint8_t run_length = 1;
        int raw = 1;
        while (curpix + run_length < npixels && run_length < max_chunk_length) {
            int succ_eq = 1;
            for (int t = 0; succ_eq && t < bpp; t++)
                succ_eq = (img->data[curbyte + t] == img->data[curbyte + t + bpp]);
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
        fwrite(img->data + chunkstart, 1, raw ? run_length * bpp : bpp, out);
        if (ferror(out)) return 0;
    }
    return 1;
}

TGAImage* tga_create(int w, int h, TGAFormat bpp) {
    TGAImage* img = malloc(sizeof(TGAImage));
    img->w = w;
    img->h = h;
    img->bpp = bpp;
    img->data = calloc(w * h * bpp, 1);
    return img;
}

void tga_free(TGAImage* img){
    if(img != NULL){
        if(img->data != NULL){
            free(img->data);
            img->data = NULL;
        }
        free(img);
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
    const int w   = header.width;
    const int h   = header.height;
    const TGAFormat bpp = header.bitsperpixel>>3;
    if (w<=0 || h<=0 || (bpp!=TGA_GRAYSCALE && bpp!=TGA_RGB && bpp!=TGA_RGBA)) {
        printf("bad bpp (or width/height) value\n");
        fclose(in);
        return NULL;
    }
    size_t nbytes = bpp * w * h;
    TGAImage* img = tga_create(w, h, bpp);
    if(img == NULL || img->data == NULL){
        printf("could not create TGAImage object\n");
        fclose(in);
        return NULL;
    }

    if (3==header.datatypecode || 2==header.datatypecode) {
        fread(img->data, 1, nbytes, in);
        if (ferror(in)) {
            printf("an error occured while reading the data\n");
            tga_free(img);
            fclose(in);
            return NULL;
        }
    } else if (10==header.datatypecode||11==header.datatypecode) {
        if (!load_rle_data(img, in)) {
            printf("an error occured while reading the data\n");
            tga_free(img);
            fclose(in);
            return NULL;
        }
    } else {
        printf("unknown file format %d\n", header.datatypecode);
        tga_free(img);
        fclose(in);
        return NULL;
    }
    if (!(header.imagedescriptor & 0x20))
        tga_flip_vertically(img);
    if (header.imagedescriptor & 0x10)
        tga_flip_horizontally(img);
    printf("%d x %d / %d\n", w, h, bpp*8);
    fclose(in);
    return img;
}

int tga_write(const TGAImage *img, const char *filename, int vflip, int rle) {
    static const uint8_t developer_area_ref[4] = {0, 0, 0, 0};
    static const uint8_t extension_area_ref[4] = {0, 0, 0, 0};
    static const uint8_t footer[18] = {'T','R','U','E','V','I','S','I','O','N','-','X','F','I','L','E','.','\0'};
    FILE *out = fopen(filename, "wb");
    if (!out) {
        printf("can't open file %s\n", filename);
        return 0;
    }
    TGAHeader header = {0};
    header.bitsperpixel = img->bpp << 3;
    header.width  = img->w;
    header.height = img->h;
    header.datatypecode = (img->bpp == TGA_GRAYSCALE ? (rle ? 11 : 3) : (rle ? 10 : 2));
    header.imagedescriptor = vflip ? 0x00 : 0x20;
    fwrite(&header, 1, sizeof(header), out);
    if (ferror(out)) goto err;
    if (!rle) {
        fwrite(img->data, 1, img->w * img->h * img->bpp, out);
        if (ferror(out)) goto err;
    } else if (!unload_rle_data(img, out)) goto err;
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

void tga_flip_horizontally(TGAImage *img) {
    int bpp = img->bpp;
    for (int i = 0; i < img->w / 2; i++)
        for (int j = 0; j < img->h; j++)
            for (int b = 0; b < bpp; b++) {
                uint8_t tmp = img->data[(i + j * img->w) * bpp + b];
                img->data[(i + j * img->w) * bpp + b] = img->data[(img->w - 1 - i + j * img->w) * bpp + b];
                img->data[(img->w - 1 - i + j * img->w) * bpp + b] = tmp;
            }
}

void tga_flip_vertically(TGAImage *img) {
    int bpp = img->bpp;
    for (int i = 0; i < img->w; i++)
        for (int j = 0; j < img->h / 2; j++)
            for (int b = 0; b < bpp; b++) {
                uint8_t tmp = img->data[(i + j * img->w) * bpp + b];
                img->data[(i + j * img->w) * bpp + b] = img->data[(i + (img->h - 1 - j) * img->w) * bpp + b];
                img->data[(i + (img->h - 1 - j) * img->w) * bpp + b] = tmp;
            }
}

TGAColor tga_get(const TGAImage *img, int x, int y) {
    TGAColor color = {{0, 0, 0, 0}, 0};
    if (!img->data || x < 0 || y < 0 || x >= img->w || y >= img->h)
        return color;
    color.bytespp = img->bpp;
    const uint8_t *p = img->data + (x + y * img->w) * img->bpp;
    for (int i = 0; i < (int)img->bpp; i++)
        color.bgra[i] = p[i];
    return color;
}

void tga_set(TGAImage *img, int x, int y, TGAColor c) {
    if (!img->data || x < 0 || y < 0 || x >= img->w || y >= img->h) return;
    memcpy(img->data + (x + y * img->w) * img->bpp, c.bgra, img->bpp);
}