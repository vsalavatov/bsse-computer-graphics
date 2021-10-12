#ifndef PLOTTER_PLOT_MESH_H
#define PLOTTER_PLOT_MESH_H

static const char plot_mesh_vertex_shader_source[] =
R"(#version 330 core

uniform mat4 view;
uniform mat4 transform_xz;
uniform mat4 transform_xy;
uniform mat4 projection;
uniform vec4 low_color;
uniform vec4 high_color;
uniform float min_value;
uniform float max_value;

layout (location = 0) in vec2 grid_xy;
layout (location = 1) in float f_val;

out vec4 color;

void main()
{
	gl_Position = projection * view * transform_xz * transform_xy * vec4(grid_xy[0], f_val, grid_xy[1], 1.0);
    float coef = (f_val - min_value) / (max_value - min_value);
	color = high_color * coef + low_color * (1 - coef);
}
)";

static const char plot_mesh_fragment_shader_source[] =
R"(#version 330 core

in vec4 color;

layout (location = 0) out vec4 out_color;

void main()
{
	out_color = color;
}
)";

#endif
