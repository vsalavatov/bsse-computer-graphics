#ifndef PLOTTER_LINALG_H
#define PLOTTER_LINALG_H

struct Vec2 {
    float x;
    float y;
};

struct Vec3 {
    float x;
    float y;
    float z;
};


Vec2 interpolate(Vec2 a, float va, Vec2 b, float vb, float target);

#endif
