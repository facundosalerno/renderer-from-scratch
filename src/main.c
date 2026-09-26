#include <stdio.h> 
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "tgaimage.h"
#include "geometry.h"
#include "wavefront.h"
#include "color.h"

// https://haqr.eu/tinyrenderer/bresenham/#homework-wireframe-rendering


Point project(int w, int h, Vertex v){
    Point p = {
        .x = round((v.x + 1.0) * w / 2), 
        .y = round((v.y + 1.0) * h / 2), 
        .z = 0
    };
    return p;
}


void triangle_rasterization(){

    Model model = wavefront_read("input/diablo3_pose.obj");
    if(strcmp(model.error, "") != 0){
        printf("error loading model: %s", model.error);
        return;
    }

    int width = 512;
    int height = 512;
    TGAImage* framebuffer = tga_create(width, height, TGA_RGB);

    for(int i=0; i<model.f_len; i++){
        // Las faces son simplemente instrucciones de donde poner las lineas. Son 3 indices que corresponden a
        // puntos que tienen que estar unidos con lineas.
        Face f = model.f[i];

        // Los 3 vertices del triangulo
        // 'model.v' son los vertices accesibles por indice y 'model.f' son las faces que indican indices a acceder. 'f' es la face actual
        // y 'f.v' contiene los tres indices particulares que hay que conectar
        Vertex v1 = model.v[f.v[0]];
        Vertex v2 = model.v[f.v[1]];
        Vertex v3 = model.v[f.v[2]];

        Point p1 = project(width, height, v1);
        Point p2 = project(width, height, v2);
        Point p3 = project(width, height, v3);

        Triangle t = triangle(p1, p2, p3, true);
        Color random_color = {.bgra={rand() % 255, rand() % 255, rand() % 255}, .bytespp=4};
        tga_fill_triangle(framebuffer, t, random_color);
        // Para ver vertices en lugar del background
        // tga_draw_line(framebuffer, t.lines[0], red);
        // tga_draw_line(framebuffer, t.lines[1], red);
        // tga_draw_line(framebuffer, t.lines[2], red);
        triangle_free(t);
    }

    // Este for es opcional, solo para marcar los vertices de cada triangulo a fines de debug
    // for(int i=0; i<model.v_len; i++){
    //     Vertex v = model.v[i];
    //     Point p = project(width, height, v);
    //     tga_set(framebuffer, p, white);
    // }

    //char filename[64];
    //snprintf(filename, sizeof(filename), "output/diablo3_pose_%dg.tga", g);
    tga_write(framebuffer, "output/diablo3_pose.tga", true, false);

    tga_free(framebuffer);
    wavefront_free(&model);

    return;
}





int main(){
    triangle_rasterization();
    return 0;
}