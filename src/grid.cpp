#include "grid.h"

Grid make_grid(float x_low, float x_high, float y_low, float y_high, size_t x_cells, size_t y_cells) {
    Grid result(y_cells + 1, std::vector<Vec2>(x_cells + 1));
    auto y_step = (y_high - y_low) / y_cells;
    auto x_step = (x_high - x_low) / x_cells;
    for (size_t r = 0; r <= y_cells; r++) {
        for (size_t c = 0; c <= x_cells; c++) {
            result[r][c] = Vec2{x_low + x_step * c, y_low + y_step * r};
        }
    }
    return result;
}
