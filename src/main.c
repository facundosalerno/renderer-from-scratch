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

    // Algoritmo Scanline rendering:
    // 1. Ordeno los vertices de menor a mayor (en funcion de la coordenada Y): esto es para establecer el orden del render, siempre va a ser de abajo hacia arriba. Graficamente siempre me va a quedar el segmento desde 'a' hasta 'c' como el mas largo, mientras que los segmentos de 'a' a 'b' y 'b' a 'c' como los mas cortos.
    // 2. Rasterizo los bordes izquierdos y derechos para cada punto de 'y': en este caso rasterizacion se debe entender como encontrar los puntos de 'x' de forma analitica tal que pueda trazar la linea horizontal que los una. Esto para cada punto de 'y' incrementando de a una unidad hasta el tope.
    // 3. Dibujo la linea horizontal a partir de los vertices encontrados en el paso 2

    
    // Por cada triangulo
    for(int i=0; i<3; i++){
        Point* points = triangles[i];
        Color color = colors[i];

        Point a = triangles[i][0];
        Point b = triangles[i][1];
        Point c = triangles[i][2];

        // 1. Para el ordenado uso bubblesort adhoc: establezco el orden ascendente a < b < c
        if(a.y > b.y){
            SWAP(int, a.x, b.x);
            SWAP(int, a.y, b.y);
        }
        if(a.y > c.y){
            SWAP(int, a.x, c.x);
            SWAP(int, a.y, c.y);
        }
        if(b.y > c.y){
            SWAP(int, b.x, c.x);
            SWAP(int, b.y, c.y);
        }

        // A partir de aca los vertices 'a', 'b' y 'c' pueden ser interpretados mas intuitivamente ya que estan ordenados: 'a' es el de mas abajo, no importa cual era antes. 'b' es el del medio. 'c' es el de mas arriba en la imagen.
        // DEBUG: descomentar esto para ver los segmentos importantes en colores
        //tga_draw_line(framebuffer, line(a, b), green);
        //tga_draw_line(framebuffer, line(b, c), green);
        //tga_draw_line(framebuffer, line(a, c), red);

        // 2. Aca ocurre el 50% del paso 2: encontrar para cada punto de 'y' (solo del segmento desde 'a' hasta 'b') cuales serian los puntos en 'x' del segmento izquiero y del segmento derecho. En principio no se cual es cual pero si se que uno tiene que ser el segmento 'a' y 'b', y otro tiene que ser el 'a' y 'c' ya que obligatoriamente 'a' es el mas chico (por que estan ordenados desde el paso 1)
        // En esta parte solo se pinta la primer mitad del triangulo

        // Primero chequea que el borde de abajo no sea degenerado: si 'a' y 'b' tienen la misma componente en 'y' significa que no hay "altura" entre ellos por lo tanto no hay nada que rellenar. Es como si el triangulo no tuviera "parte de abajo". Esta relacion no podria darse con 'c' ya que este debe ser obligatoriamente mayor a 'b' ya que si no lo fuese, la figura seria una linea recta en lugar de un triangulo (a.y = b.y = c.y).
        if(a.y != b.y){

            for(int y=a.y; y<=b.y; y++){
                // Usando exactamente la misma formula que en 'Bresenham’s line drawing' podemos despejar 'x' a partir de 'y'.
                // Para dos puntos genericos 'a' y 'b' es: x(y) = ax + (y - ay) * (bx - ax) / (by - ay)

                // Calculo 'x1' para la linea formada entre 'a' y 'b'
                int x1 = a.x + (y - a.y) * (b.x - a.x) / (b.y - a.y);
                // Calculo 'x2' para la linea formada entre 'a' y 'c'
                int x2 = a.x + (y - a.y) * (c.x - a.x) / (c.y - a.y);
                Point px1 = {.x=x1, .y=y};
                Point px2 = {.x=x2, .y=y};
                Line horizontal_line = line(px1, px2);
                tga_draw_line(framebuffer, horizontal_line, color);
                line_free(horizontal_line);
            }
        }

        // 2. Aca ocurre el 50% restante del paso 2: analogamente se pinta la parte restante

        // Mismo chequeo que antes pero midiendo la altura entre 'b' y 'c'
        if(b.y != c.y){

            // Aca esta representada la parte restante del triangulo: va de 'b' a 'c' en lugar de 'a' a 'b'
            for(int y=b.y; y<=c.y; y++){
                // Ojo con el orden de los segmentos en esta parte. si bien es matematicamente lo mismo tomar de 'b' a 'c' o de 'c' a 'b',
                // 'x1' para segmento ['b', 'c']
                int x1 = b.x + (y - b.y) * (c.x - b.x) / (c.y - b.y);
                // 'x2' para segmento ['a', 'c']
                int x2 = a.x + (y - a.y) * (c.x - a.x) / (c.y - a.y);
                Point px1 = {.x=x1, .y=y};
                Point px2 = {.x=x2, .y=y};
                Line horizontal_line = line(px1, px2);
                tga_draw_line(framebuffer, horizontal_line, color);
                line_free(horizontal_line);
            }
        }
    }

    tga_write(framebuffer, "output/triangles.tga", true, false);
    tga_free(framebuffer);
}





int main(){
    triangle_rasterization();
    return 0;
}