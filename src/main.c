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





void bresenhams_line_drawing(){
    for(int g=0; g<=365; g++){
        Model model = wavefront_read("input/head.obj");
        if(strcmp(model.error, "") != 0){
            printf("error loading model: %s", model.error);
            return;
        }

        int width = 512;
        int height = 512;
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
            // Por ultimo, lo multiplico por width y height para "escalar" la imagen
            // Update: la guia divide por 2: actualmente el ancho de la imagen es 2 (de -1 a 1 o de 0 a 2, cualquier opcion es lo mismo, la segunda opcion
            // es la que nos queda despues de haber sumado el 1). Para escalar una imagen de ancho 2 a ancho width regla de 3: si 2 es width -> x es x*width/2
            // Misma logica para el height al cual tambien le sumamos 1.
            float delta = sin(g * 3.14159265358979 / 180.0)*v.z;
            projected[i].x = round(width * (1.0 + v.x + delta) / 2.0);
            projected[i].y = round(height * (1.0 + v.y + delta) / 2.0);
            projected[i].z = 0;
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
        snprintf(filename, sizeof(filename), "output/head_%dg.tga", g);
        tga_write(framebuffer, filename, true, false);

        free(projected);
        tga_free(framebuffer);
        wavefront_free(&model);
    }
    return;
}





void triangle_rasterization(){
    TGAImage* framebuffer = tga_create(128, 126, TGA_RGB);

    // N triangulos cada uno con sus 3 vertices
    Point triangles[][3] = {
        {{.x=7, .y=45}, {.x=35, .y=100}, {.x=45, .y=60}},
        {{.x=120, .y=35}, {.x=90, .y=5}, {.x=45, .y=110}},
        {{.x=115, .y=83}, {.x=80, .y=90}, {.x=85, .y=120}},
    };

    Color colors[] = {red, white, green};

    for(int i=0; i<3; i++){
        Point* points = triangles[i];
        Color color = colors[i];
        Triangle t = triangle(points[0], points[1], points[2]);
        tga_draw_triangle(framebuffer, t, color);


        Line l1 = t.lines[0];
        Line l2 = t.lines[1];
        Line l3 = t.lines[2];
        //printf("Vertices para empezar a pintar %d,%d y %d,%d\n", l1.points[0].x, l1.points[0].y, l2.points[0].x, l2.points[0].y);
        // Se queda con la linea mas larga
        Line* ll = longest(longest(&l1, &l2), &l3);
        Line lines[3] = {l1, l2, l3};
        
        for(unsigned int j=0; j<ll->len; ++j){
            Point p1 = ll->points[j];
            // Por cada punto de la linea mas larga, determino mi condicion de corte. Esto es, dejar fija
            // alguna coordenada X o Y y generar una recta entre la coordenada movil y otro lado del triangulo,
            // es decir, otra linea.
            // Como no se exactamente cual de todas las lineas fue la mas larga, pregunto para cada una
            for(int k=0; k<3; ++k){
                Line lo = lines[k];

                if(!equals(ll, &lo)){
                    // Necesito determinar si dejar fija la coordenada X o Y. Esto es, probar para cada coordenada X e Y
                    // del punto actual si forma parte de l1.
                    
                    // Antes de dejar fijo Y necesito comprobar que forma parte del triangulo en lo. Es decir, p1 lo obtuvimos de ll
                    // pero aun no chequeamos que tambien lo tenga lo
                    if(contains_y(&lo, p1.y)){
                        // Si dejo fijo Y, necesito despejar el X en l1. Utilizo p.y como punto compartido entre ll y l1
                        // dado que justamente queda fija la coordenada en Y (x=(y-b)/m)
                        int x = (p1.y - lo.b) / lo.m;
                        Point p2 = {.x = x, .y = p1.y};
                        tga_draw_line(framebuffer, line(p1, p2), color);
                    }else if(contains_x(&lo, p1.x)){
                        // Lo mismo pero dejo fija la coordenada en X (y=mx+b)
                        int y = lo.m * p1.x + lo.b;
                        Point p2 = {.x = p1.x, .y = y};
                        tga_draw_line(framebuffer, line(p1, p2), color);
                    }
                }
            }
        }
        triangle_free(t);
    }
    tga_write(framebuffer, "output/triangles.tga", true, false);
    tga_free(framebuffer);
    return;
}





int main(){
    triangle_rasterization();
    return 0;
}