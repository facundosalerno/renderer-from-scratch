#include "tgaimage.h"

int main(){
    TGAImage* img = tga_create(64, 64, TGA_RGB);

    int ax =  7, ay =  3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    tga_set(img, ax, ay, tga_white);
    tga_set(img, bx, by, tga_white);
    tga_set(img, cx, cy, tga_white);

    tga_write(img, "output/framebuffer.tga", 0, 0);

    tga_free(img);
    img = NULL;
    return 0;
}