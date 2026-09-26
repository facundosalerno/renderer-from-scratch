#pragma once

#include <stdbool.h>
#define SWAP(T, a, b) do { T _tmp = (a); (a) = (b); (b) = _tmp; } while(0)

typedef struct {
    int x, y, z;
} Point;

typedef struct {
    Point* points;
    unsigned int len;
    // y = mx + b
    float m; // Pendiente
    float b; // Ordenada
} Line;

typedef struct {
    Line lines[3];
    int n_area;
    Line* area;
} Triangle;


Line line(Point, Point);
void line_free(Line);

Line* longest(Line*, Line*);
bool equals(Line*, Line*);
bool contains(Line*, Point*);
bool contains_x(Line*, int);
bool contains_y(Line*, int);
bool contains_z(Line*, int);

Triangle triangle(Point, Point, Point, bool fill);
void triangle_free(Triangle);