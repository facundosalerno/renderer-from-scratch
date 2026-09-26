#include <stdlib.h>

#include "geometry.h"

Line line(Point a, Point b) {
    Line line = {.m = 0.0, .b = 0.0, .len = 0, .points = NULL};

    bool steep = abs(a.x-b.x) < abs(a.y-b.y);
    if (steep) { // if the line is steep, we transpose the image
        SWAP(int, a.x, a.y);
        SWAP(int, b.x, b.y);
    }
    if (a.x>b.x) { // make it left−to−right
        SWAP(int, a.x, b.x);
        SWAP(int, a.y, b.y);
    }
    int y = a.y;
    int ierror = 0;
    for (int x=a.x; x<=b.x; x++) {
        line.points = realloc(line.points, ++line.len * sizeof(Point));
        
        if (steep){ // if transposed, de−transpose
            line.points[line.len - 1].x = y;
            line.points[line.len - 1].y = x;
            line.points[line.len - 1].z = 0;
        }else{
            line.points[line.len - 1].x = x;
            line.points[line.len - 1].y = y;
            line.points[line.len - 1].z = 0;
        }

        ierror += 2 * abs(b.y-a.y);
        if (ierror > b.x - a.x) {
            y += b.y > a.y ? 1 : -1;
            ierror -= 2 * (b.x-a.x);
        }
    }
    // Calculo pendiente y ordenada
    float delta_y = line.points[line.len-1].y - line.points[0].y;
    float delta_x = line.points[line.len-1].x - line.points[0].x;
    line.m = delta_y / delta_x;
    line.b = line.points[0].y - line.m * line.points[0].x;
    return line;
}

Line* longest(Line* l1, Line* l2){
    return l1->len >= l2->len ? l1 : l2;
}

bool equals(Line* l1, Line* l2){
    if(l1 == l2)
        return true;
    if(
        // Sus punto iniciales (X0, Y0, Z0) son los mismos
        l1->points[0].x == l2->points[0].x && l1->points[0].y == l2->points[0].y && l1->points[0].z == l2->points[0].z
        &&
        // Sus puntos finales tambien
        l1->points[l1->len-1].x == l2->points[l1->len-1].x && l1->points[l1->len-1].y == l2->points[l1->len-1].y && l1->points[l1->len-1].z == l2->points[l1->len-1].z
    )
        return true;
    return false;
}

bool contains(Line* l1, Point* p1){
    return contains_x(l1, p1->x) && contains_y(l1, p1->y) && contains_z(l1, p1->z);
}

bool contains_x(Line* l1 , int x){
    int x_1 = l1->points[l1->len-1].x >= l1->points[0].x ? l1->points[l1->len-1].x : l1->points[0].x;
    int x_0 = l1->points[0].x <= l1->points[l1->len-1].x ? l1->points[0].x : l1->points[l1->len-1].x;
    return x >= x_0 && x <= x_1;
}

bool contains_y(Line* l1 , int y){
    int y_1 = l1->points[l1->len-1].y >= l1->points[0].y ? l1->points[l1->len-1].y : l1->points[0].y;
    int y_0 = l1->points[0].y <= l1->points[l1->len-1].y ? l1->points[0].y : l1->points[l1->len-1].y;
    return y >= y_0 && y <= y_1;
}

bool contains_z(Line* l1 , int z){
    int z_1 = l1->points[l1->len-1].z >= l1->points[0].z ? l1->points[l1->len-1].z : l1->points[0].z;
    int z_0 = l1->points[0].z <= l1->points[l1->len-1].z ? l1->points[0].z : l1->points[l1->len-1].z;
    return z >= z_0 && z <= z_1;
}

Triangle triangle(Point a, Point b, Point c, bool fill){
    Triangle t;
    t.lines[0] = line(a, b);
    t.lines[1] = line(b, c);
    t.lines[2] = line(c, a);

    if(fill){
        // Si el triangulo debe contener tambien las lineas necesarias para cubrir toda su area o caso 
        // contrario solo sus tres lineas de perimetro

        // Algoritmo Scanline rendering:
        // 1. Ordeno los vertices de menor a mayor (en funcion de la coordenada Y): esto es para establecer 
        //    el orden del render, siempre va a ser de abajo hacia arriba. Graficamente siempre me va a 
        //    quedar el segmento desde 'a' hasta 'c' como el mas largo, mientras que los segmentos de 'a' a 'b' 
        //    y 'b' a 'c' como los mas cortos.
        // 2. Rasterizo los bordes izquierdos y derechos para cada punto de 'y': en este caso rasterizacion 
        //    se debe entender como encontrar los puntos de 'x' de forma analitica tal que pueda trazar la 
        //    linea horizontal que los una. Esto para cada punto de 'y' incrementando de a una unidad hasta 
        //    el tope.
        // 3. Dibujo la linea horizontal a partir de los vertices encontrados en el paso 2

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

        // A partir de aca los vertices 'a', 'b' y 'c' pueden ser interpretados mas intuitivamente ya que estan 
        // ordenados: 'a' es el de mas abajo, no importa cual era antes. 'b' es el del medio. 'c' es el de mas 
        // arriba en la imagen.

        t.n_area = c.y - a.y + 1;
        t.area = malloc(sizeof(Line) * t.n_area);

        // 2. Aca ocurre el 50% del paso 2: encontrar para cada punto de 'y' (solo del segmento desde 'a' 
        // hasta 'b') cuales serian los puntos en 'x' del segmento izquiero y del segmento derecho. 
        // En principio no se cual es cual pero si se que uno tiene que ser el segmento 'a' y 'b', y otro 
        // tiene que ser el 'a' y 'c' ya que obligatoriamente 'a' es el mas chico (por que estan ordenados 
        // desde el paso 1)
        // En esta parte solo se pinta la primer mitad del triangulo

        // Primero chequea que el borde de abajo no sea degenerado: si 'a' y 'b' tienen la misma componente 
        // en 'y' significa que no hay "altura" entre ellos por lo tanto no hay nada que rellenar (en realidad 
        // tecnicamente si hay una sola linea horizontal que si tiene que ser dibujada). 
        // Es como si el triangulo no tuviera "parte de abajo". Esta relacion no podria darse con 'c' ya 
        // que este debe ser obligatoriamente mayor a 'b' ya que si no lo fuese, la figura seria una linea 
        // recta en lugar de un triangulo (a.y = b.y = c.y).
        if(a.y != b.y){

            for(int y=a.y; y<=b.y; y++){
                // Usando exactamente la misma formula que en 'Bresenham’s line drawing' podemos 
                // despejar 'x' a partir de 'y'.
                // Para dos puntos genericos 'a' y 'b' es: x(y) = ax + (y - ay) * (bx - ax) / (by - ay)

                // Calculo 'x1' para la linea formada entre 'a' y 'b'
                int x1 = a.x + (y - a.y) * (b.x - a.x) / (b.y - a.y);
                // Calculo 'x2' para la linea formada entre 'a' y 'c'
                int x2 = a.x + (y - a.y) * (c.x - a.x) / (c.y - a.y);
                Point px1 = {.x=x1, .y=y};
                Point px2 = {.x=x2, .y=y};
                // Si 'a.y' es la ubicacion de la parte mas baja del triangulo y 'y' es la ubicacion 
                // actual, 'y - a.y' es la ubicaicon actual relativa. Esto aplica igual para el segundo loop
                t.area[y - a.y] = line(px1, px2);
            }
        }else{
            // Solo dibujamos la linea segmento entre a y b
            t.area[0] = line(a, b);
        }

        // 2. Aca ocurre el 50% restante del paso 2: analogamente se pinta la parte restante

        // Mismo chequeo que antes pero midiendo la altura entre 'b' y 'c'

        // Aca esta representada la parte restante del triangulo: va de 'b' a 'c' en lugar de 'a' a 'b'
        // Este segundo loop no tiene chequeo de b.y == c.y por que ya este protegido implicitamente por el 
        // for que arranca en 'y=b.y+1'. si fueran iguales no haria ningun loop.
        // Esto hace surgir otra pregunta, por que arranca de 'b.y+1'? La razon es que esta es una variacion 
        // del algoritmo de la guia, el loop anterior termina cuando 'y' es igual a 'b.y', por lo tanto comenzar
        // desde ese punto seria repetir la misma linea asi que la salteamos. De esta manera, el primer loop 
        // va de [a.y, b.y] y el segundo de [b.y+1, c.y] cubriendo asi todo el rango.
        for(int y=b.y+1; y<=c.y; y++){
            // 'x1' para segmento ['b', 'c']
            int x1 = b.x + (y - b.y) * (c.x - b.x) / (c.y - b.y);
            // 'x2' para segmento ['a', 'c']
            int x2 = a.x + (y - a.y) * (c.x - a.x) / (c.y - a.y);
            Point px1 = {.x=x1, .y=y};
            Point px2 = {.x=x2, .y=y};
            t.area[y - a.y] = line(px1, px2);
        }
    }else{
        t.area = NULL;
        t.n_area = 0;
    }
    return t;
}

void line_free(Line line){
    free(line.points);
}

void triangle_free(Triangle triangle){
    line_free(triangle.lines[0]);
    line_free(triangle.lines[1]);
    line_free(triangle.lines[2]);
    if(triangle.area != NULL){
        for(int i=0; i<triangle.n_area; i++){
            line_free(triangle.area[i]);
        }
    }
    free(triangle.area);
}