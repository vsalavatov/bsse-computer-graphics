#ifndef PLOTTER_PLOT_H
#define PLOTTER_PLOT_H

#include "gl_utils.h"
#include "grid.h"

#include <functional>

typedef std::function<float(float x, float y, float t)> Plottable;


class Plot3D {
public:
    Plot3D(Plottable func, Grid grid, size_t isolines_count = 5);
    Plot3D() = delete;
    Plot3D(const Plot3D&) = delete;
    Plot3D(Plot3D&&) = delete;
    void operator=(const Plot3D&) = delete;
    void operator=(Plot3D&&) = delete;

    ~Plot3D();


    void set_func(Plottable func);
    void set_grid(Grid grid);
    void set_isolines_count(size_t isolines_count);

    void plot(float t);

    float get_min_value() const;
    float get_max_value() const;

    GLuint get_mesh_vao() const;
    size_t get_mesh_elements_count() const;

    GLuint get_isolines_vao() const;
    size_t get_isolines_elements_count() const;

private:
    void build_isolines_();

    void rebuild_();
    void load_mesh_to_gpu_();
    void load_func_vals_to_gpu_();
    void load_isolines_to_gpu_();

private:
    Plottable func_;
    Grid grid_;
    size_t isolines_count_;
    std::vector<std::vector<float>> values_;
    float min_value_, max_value_;
    
    GLuint mesh_vao_;
    GLuint mesh_vbo_xy_;
    GLuint mesh_vbo_z_;
    GLuint mesh_ebo_;

    std::vector<Vec2> flat_grid_;
    std::vector<float> flat_values_;
    std::vector<uint32_t> traverse_idx_;

    GLuint isolines_vao_;
    GLuint isolines_vbo_;
    GLuint isolines_ebo_;

    std::vector<Vec3> isoline_points_;
    std::vector<uint32_t> isoline_traverse_idx_;
};

#endif
