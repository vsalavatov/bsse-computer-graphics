#include "plot.h"

#include <map>
#include <iostream>

Plot3D::Plot3D(Plottable func, Grid grid, size_t isolines_count)
    : func_(std::move(func)), grid_(std::move(grid)), isolines_count_(isolines_count) 
{   
    glGenVertexArrays(1, &mesh_vao_);
    glGenBuffers(1, &mesh_vbo_xy_);
    glGenBuffers(1, &mesh_vbo_z_);
    glGenBuffers(1, &mesh_ebo_);
    
    glBindVertexArray(mesh_vao_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_ebo_);
    
    glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo_xy_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo_z_);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);

    glGenVertexArrays(1, &isolines_vao_);
    glGenBuffers(1, &isolines_vbo_);
    glGenBuffers(1, &isolines_ebo_);

    glBindVertexArray(isolines_vao_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, isolines_ebo_);

    glBindBuffer(GL_ARRAY_BUFFER, isolines_vbo_);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void*)0);

    rebuild_();
}

Plot3D::~Plot3D() {
    glDeleteVertexArrays(1, &isolines_vao_);
    glDeleteBuffers(1, &isolines_vbo_);
    glDeleteBuffers(1, &isolines_ebo_);

    glDeleteVertexArrays(1, &mesh_vao_);
    glDeleteBuffers(1, &mesh_ebo_);
    glDeleteBuffers(1, &mesh_vbo_xy_);
    glDeleteBuffers(1, &mesh_vbo_z_);
}

void Plot3D::set_func(Plottable f) {
    func_ = std::move(f);
    rebuild_();
}

void Plot3D::set_grid(Grid g) {
    grid_ = std::move(g);
    rebuild_();
}

void Plot3D::set_isolines_count(size_t isolines_count) {
    isolines_count_ = isolines_count;
    build_isolines_();
}

void Plot3D::plot(float t) {
    size_t flat_it = 0;
    for (size_t r = 0; r < grid_.size(); r++) {
        for (size_t c = 0; c < grid_[0].size(); c++, flat_it++) {
            flat_values_[flat_it] = values_[r][c] = func_(grid_[r][c].x, grid_[r][c].y, t);
        }
    }

    min_value_ = max_value_ = flat_values_[0];
    for (auto x : flat_values_) {
        min_value_ = std::min(min_value_, x);
        max_value_ = std::max(max_value_, x);
    }

    build_isolines_();

    load_func_vals_to_gpu_();
}

float Plot3D::get_min_value() const {
    return min_value_;
}

float Plot3D::get_max_value() const {
    return max_value_;
}

GLuint Plot3D::get_mesh_vao() const {
    return mesh_vao_;
}

size_t Plot3D::get_mesh_elements_count() const {
    return traverse_idx_.size();
}

GLuint Plot3D::get_isolines_vao() const {
    return isolines_vao_;
}

size_t Plot3D::get_isolines_elements_count() const {
    return isoline_traverse_idx_.size();
}

static std::pair<size_t, size_t> sorted_pair(size_t a, size_t b) {
    if (a < b) {
        return {a, b};
    }
    return {b, a};
}

void Plot3D::build_isolines_() {
    const float z_shift = 0.001;
    
    isoline_points_.clear();
    isoline_traverse_idx_.clear();

    for (size_t level = 0; level < isolines_count_; level++) {
        float target_value = min_value_ + (max_value_ - min_value_) / (isolines_count_ + 1) * (level + 1);
        std::map<std::pair<size_t, size_t>, size_t> memoize;
        for (size_t i = 0; i < traverse_idx_.size(); i += 3) {
            size_t v0 = traverse_idx_[i + 0];
            size_t v1 = traverse_idx_[i + 1];
            size_t v2 = traverse_idx_[i + 2];
            bool l0 = flat_values_[v0] < target_value;
            bool l1 = flat_values_[v1] < target_value;
            bool l2 = flat_values_[v2] < target_value;
            int s = (int)l0 + (int)l1 + (int)l2;
            if (s == 0 || s == 3) continue;
            // swap things around so that l0 == l1, l0 != l2, l1 != l2
            if (l0 == l2) {
                std::swap(l1, l2);
                std::swap(v1, v2);
            } else if (l1 == l2) {
                std::swap(l0, l2);
                std::swap(v0, v2);
            }

            if (!memoize.contains(sorted_pair(v0, v2))) {
                auto pt = interpolate(flat_grid_[v0], flat_values_[v0], flat_grid_[v2], flat_values_[v2], target_value);
                memoize[sorted_pair(v0, v2)] = isoline_points_.size();
                isoline_points_.push_back(Vec3{pt.x, pt.y, target_value + z_shift});
            }
            if (!memoize.contains(sorted_pair(v1, v2))) {
                auto pt = interpolate(flat_grid_[v1], flat_values_[v1], flat_grid_[v2], flat_values_[v2], target_value);
                memoize[sorted_pair(v1, v2)] = isoline_points_.size();
                isoline_points_.push_back(Vec3{pt.x, pt.y, target_value + z_shift});
            }
            isoline_traverse_idx_.push_back(memoize[sorted_pair(v0, v2)]);
            isoline_traverse_idx_.push_back(memoize[sorted_pair(v1, v2)]);
        }
    }

    load_isolines_to_gpu_();
}

void Plot3D::rebuild_() {
    values_ = std::vector<std::vector<float>>(grid_.size(), std::vector<float>(grid_[0].size()));
    flat_grid_.resize(grid_.size() * grid_[0].size());
    flat_values_.resize(grid_.size() * grid_[0].size());
    traverse_idx_.resize((grid_.size() - 1) * (grid_[0].size() - 1) * 6);

    size_t flat_it = 0;
    for (size_t r = 0; r < grid_.size(); r++) {
        for (size_t c = 0; c < grid_[0].size(); c++, flat_it++) {
            flat_grid_[flat_it] = grid_[r][c];
            flat_values_[flat_it] = 0;
        }
    }

    auto conv_idx = [this](size_t r, size_t c) -> size_t {
        return grid_[0].size() * r + c;
    };

    size_t it = 0;
    for (size_t r = 1; r < grid_.size(); r++) {
        for (size_t c = 1; c < grid_[0].size(); c++) {
            traverse_idx_[it++] = conv_idx(r, c - 1);
            traverse_idx_[it++] = conv_idx(r - 1, c - 1);
            traverse_idx_[it++] = conv_idx(r, c);

            traverse_idx_[it++] = conv_idx(r, c);
            traverse_idx_[it++] = conv_idx(r - 1, c - 1);
            traverse_idx_[it++] = conv_idx(r - 1, c);
        }
    }

    load_mesh_to_gpu_();

    build_isolines_();
}

void Plot3D::load_mesh_to_gpu_() {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * traverse_idx_.size(), traverse_idx_.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo_xy_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec2) * flat_grid_.size(), flat_grid_.data(), GL_STATIC_DRAW);

    load_func_vals_to_gpu_();
}

void Plot3D::load_func_vals_to_gpu_() {
    glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo_z_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * flat_values_.size(), flat_values_.data(), GL_STATIC_DRAW);
}

void Plot3D::load_isolines_to_gpu_() {
    glBindBuffer(GL_ARRAY_BUFFER, isolines_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3) * isoline_points_.size(), isoline_points_.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, isolines_ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * isoline_traverse_idx_.size(), isoline_traverse_idx_.data(), GL_STATIC_DRAW);
}
