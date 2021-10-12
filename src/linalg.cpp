#include "linalg.h"

Vec2 interpolate(Vec2 a, float va, Vec2 b, float vb, float target) {
    auto c = (target - vb) / (va - vb);
    return Vec2{ a.x * c + (1 - c) * b.x, a.y * c + (1 - c) * b.y };
}
