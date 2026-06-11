#pragma once

#define SWAP(T, a, b) do { T _tmp = (a); (a) = (b); (b) = _tmp; } while(0)

typedef struct {
    int x, y, z;
} Point;

typedef struct {
    Point* points;
    unsigned int len;
} Line;


Line line(Point a, Point b);
void line_free(Line line);