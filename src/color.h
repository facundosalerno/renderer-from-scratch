#pragma once

#include <stdint.h>

typedef struct {
    uint8_t bgra[4];
    uint8_t bytespp;
} Color;

static const Color white  = {{255, 255, 255, 255}, 4}; // attention, BGRA order
static const Color green  = {{  0, 255,   0, 255}, 4};
static const Color red    = {{  0,   0, 255, 255}, 4};
static const Color blue   = {{255, 128,  64, 255}, 4};
static const Color yellow = {{  0, 200, 255, 255}, 4};