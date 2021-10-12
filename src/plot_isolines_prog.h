#ifndef PLOTTER_PLOT_ISOLINES_H
#define PLOTTER_PLOT_ISOLINES_H

static const char plot_isolines_vertex_shader_source[] =
R"(#version 330 core

uniform mat4 view;
uniform mat4 transform_xz;
uniform mat4 transform_xy;
uniform mat4 projection;
uniform vec4 in_color;

layout (location = 0) in vec3 position;

out vec4 color;

void main()
{
	gl_Position = projection * view * transform_xz * transform_xy * vec4(position[0], position[2], position[1], 1.0);
	color = in_color;
}
)";

static const char plot_isolines_fragment_shader_source[] =
R"(#version 330 core

in vec4 color;

layout (location = 0) out vec4 out_color;

void main()
{
	out_color = color;
}
)";

#endif
