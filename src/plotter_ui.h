#ifndef PLOTTER_PLOTTER_H
#define PLOTTER_PLOTTER_H

#include "gl_utils.h"
#include "plot.h"

class PlotterUI {
public:
    PlotterUI();
    PlotterUI(const PlotterUI&) = delete;
    PlotterUI(PlotterUI&&) = delete;
    void operator=(const PlotterUI&) = delete;
    void operator=(PlotterUI&&) = delete;
    ~PlotterUI();

    void run(Plottable f);

private:
    SDL_Window* window_;
    SDL_GLContext gl_context_;
    GLuint plot_mesh_vertex_shader_;
    GLuint plot_mesh_fragment_shader_;
    GLuint mesh_program_;
    
    GLuint plot_isolines_vertex_shader_;
    GLuint plot_isolines_fragment_shader_;
    GLuint isolines_program_;
};

#endif
