#pragma once

#include "geometry.h"

typedef struct {
    float x, y, z;
} Vertex;

typedef struct {
    int v[3];
} Face;

typedef struct {
    Vertex* v;
    int v_len;
    float v_x_lower, v_x_higher;
    float v_y_lower, v_y_higher;
    float v_z_lower, v_z_higher;

    Face* f;
    int f_len;

    const char* error;
} Model;

Model wavefront_read(const char* filename);
void wavefront_free(Model* model);