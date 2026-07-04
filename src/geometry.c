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

Triangle triangle(Point a, Point b, Point c){
    Triangle t;
    t.lines[0] = line(a, b);
    t.lines[1] = line(b, c);
    t.lines[2] = line(c, a);
    return t;
}

void line_free(Line line){
    free(line.points);
}

void triangle_free(Triangle triangle){
    line_free(triangle.lines[0]);
    line_free(triangle.lines[1]);
    line_free(triangle.lines[2]);
}