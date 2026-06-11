#include <stdio.h> 
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "tgaimage.h"
#include "geometry.h"
#include "wavefront.h"
#include "color.h"

int main(){
    for(int g=0; g<=0; g++){
        Model model = wavefront_read("input/diablo3_pose.obj");
        if(strcmp(model.error, "") != 0){
            printf("error loading model: %s", model.error);
            return 0;
        }

        int width = 2048;
        int height = 2048;
        TGAImage* framebuffer = tga_create(width, height, TGA_RGB);

        Point* projected = malloc(model.v_len * sizeof(Point));
        for(int i=0; i<model.v_len; i++){
            // Itero los vertices (puntos de la imagen) y los sanitizo:
            // - primero, el punto esta en 3d y necesito proyectarlo en un plano 2d
            //      - para eso lo desplazo usando geometria basica (tales) y tomo un angulo default de 45 grados
            // - segundo, necesito proyectarlo a valores enteros ya que son todos decimales entre -1 y 1
            //      - no puedo simplemente redondear, necesito expandir (multiplicar todo por algo para que el rango sea de 0 a 100 por ejemplo)
            // - tercero, tengo que hacer que todos los puntos sean positivos, el framebuffer de tga solo admite el (0, 0) como punto minimo

            Vertex v = model.v[i];

            // Transformo de 3d a 2d usando proyeccion paralela
            // p=(x1+d1, y1+d2) => Asumo angulo de 45 grados => tg(45)=d2/d1 => d1=d2 => p=(x1+d, y1+d)
            // sin(45)=d/z1 => sqrt(2)/2=d/z1
            // Generalmente los wavefront vienen en un rango de -1 a 1 por lo que puedo asumir eso como minimos y maximos absolutos
            // por lo tanto, sumar 1 a los componentes los deberia volver positivos
            // TODO esta ultima invariante puede generar bugs pero por ahora sirve
            // Por ultimo, lo multiplico por 256 tentativo para "escalar" la imagen
            float delta = sin(g)*v.z; //(sqrt(2)*v.z)/2;
            projected[i].x = round(1024*(1 + v.x + delta));
            projected[i].y = round(1024*(1 + v.y + delta));
            projected[i].z = 0;
            //printf("Punto X=%f->%d Y=%f->%d Z=%f->0\n", v.x, projected[i].x, v.y, projected[i].y, v.z);
        }


        for(int i=0; i<model.f_len; i++){
            // Las faces son simplemente instrucciones de donde poner las lineas. Son 3 indices que corresponden a
            // puntos que tienen que estar unidos con lineas.
            Face f = model.f[i];
            // Los 3 vertices del triangulo
            Point p1 = projected[f.v[0]];
            Point p2 = projected[f.v[1]];
            Point p3 = projected[f.v[2]];

            Line l1 = line(p1, p2);
            Line l2 = line(p2, p3);
            Line l3 = line(p3, p1);

            tga_draw_line(framebuffer, l1, red);
            tga_draw_line(framebuffer, l2, red);
            tga_draw_line(framebuffer, l3, red);

            line_free(l1);
            line_free(l2);
            line_free(l3);
        }

        char filename[64];
        snprintf(filename, sizeof(filename), "output/diablo2_%dg.tga", g);
        tga_write(framebuffer, filename, true, false);

        free(projected);
        tga_free(framebuffer);
        wavefront_free(&model);
    }
    return 0;
    /*
    TGAImage* framebuffer = tga_create(64, 64, TGA_RGB);

    Point a = {7, 3, 0};
    Point b = {12, 37, 0};
    Point c = {62, 53, 0};

    Line l1 = line(a, b);
    Line l2 = line(c, b);
    Line l3 = line(c, a);
    Line l4 = line(a, c);

    tga_draw_line(framebuffer, l1, blue);
    tga_draw_line(framebuffer, l2, green);
    tga_draw_line(framebuffer, l3, yellow);
    tga_draw_line(framebuffer, l4, red);

    line_free(l1);
    line_free(l2);
    line_free(l3);
    line_free(l4);

    tga_set(framebuffer, a, white);
    tga_set(framebuffer, b, white);
    tga_set(framebuffer, c, white);

    tga_write(framebuffer, "output/framebuffer.tga", true, false);

    tga_free(framebuffer);
    framebuffer = NULL;
    return 0;
    */
}