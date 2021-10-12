#ifndef PLOTTER_GRID_H
#define PLOTTER_GRID_H

#include <cstddef>
#include <vector>

#include "linalg.h"


typedef std::vector<std::vector<Vec2>> Grid;
// [y_cells + 1][x_cells + 1]
// y from low to high, x from low to high
Grid make_grid(float x_low, float x_high, float y_low, float y_high, size_t x_cells, size_t y_cells); 

#endif
