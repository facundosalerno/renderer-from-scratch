#include <stdlib.h>
#include <stdbool.h>

#include "geometry.h"

Line line(Point a, Point b) {
    Line line = {.len = 0, .points = NULL};

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
    return line;
}

void line_free(Line line){
    free(line.points);
}