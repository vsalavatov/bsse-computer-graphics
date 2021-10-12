#include "plotter_ui.h"

#include <chrono>
#include <map>

#include "plot_mesh_prog.h"
#include "plot_isolines_prog.h"
#include "plot.h"

PlotterUI::PlotterUI() {
    window_ = sdl_init_window("3d plotter");
    
    gl_context_ = SDL_GL_CreateContext(window_);
    if (!gl_context_)
        sdl2_fail("SDL_GL_CreateContext: ");

    if (auto result = glewInit(); result != GLEW_NO_ERROR)
        glew_fail("glewInit: ", result);

    if (!GLEW_VERSION_3_3)
        throw std::runtime_error("OpenGL 3.3 is not supported");

    glClearColor(1.f, 1.f, 1.f, 0.f);

    plot_mesh_vertex_shader_ = create_shader(GL_VERTEX_SHADER, plot_mesh_vertex_shader_source);
    plot_mesh_fragment_shader_ = create_shader(GL_FRAGMENT_SHADER, plot_mesh_fragment_shader_source);
    mesh_program_ = create_program(plot_mesh_vertex_shader_, plot_mesh_fragment_shader_);

    plot_isolines_vertex_shader_ = create_shader(GL_VERTEX_SHADER, plot_isolines_vertex_shader_source);
    plot_isolines_fragment_shader_ = create_shader(GL_FRAGMENT_SHADER, plot_isolines_fragment_shader_source);
    isolines_program_ = create_program(plot_isolines_vertex_shader_, plot_isolines_fragment_shader_);
}

PlotterUI::~PlotterUI() {
    SDL_GL_DeleteContext(gl_context_);
    SDL_DestroyWindow(window_);
}

void PlotterUI::run(Plottable f) {
    GLuint view_location_mesh = glGetUniformLocation(mesh_program_, "view");
    GLuint transform_xz_location_mesh = glGetUniformLocation(mesh_program_, "transform_xz");
    GLuint transform_xy_location_mesh = glGetUniformLocation(mesh_program_, "transform_xy");
    GLuint projection_location_mesh = glGetUniformLocation(mesh_program_, "projection");	
    GLuint min_value_location_mesh = glGetUniformLocation(mesh_program_, "min_value");
    GLuint max_value_location_mesh = glGetUniformLocation(mesh_program_, "max_value");
    GLuint low_color_location_mesh = glGetUniformLocation(mesh_program_, "low_color");
    GLuint high_color_location_mesh = glGetUniformLocation(mesh_program_, "high_color");

    GLuint view_location_isolines = glGetUniformLocation(isolines_program_, "view");
    GLuint transform_xz_location_isolines = glGetUniformLocation(isolines_program_, "transform_xz");
    GLuint transform_xy_location_isolines = glGetUniformLocation(isolines_program_, "transform_xy");
    GLuint projection_location_isolines = glGetUniformLocation(isolines_program_, "projection");	
    GLuint color_location_isolines = glGetUniformLocation(isolines_program_, "in_color");

    int width, height;
    SDL_GetWindowSize(window_, &width, &height);

    bool running = true;
    auto last_frame_start = std::chrono::high_resolution_clock::now();
    float time = 0.f;

    std::map<SDL_Keycode, bool> button_down;

    float x_low = -10;
    float x_high = 10;
    float y_low = -10;
    float y_high = 10;
    size_t cells = 20;
    size_t isolines = 5;
    float rotation_xz = 0;
    float rotation_xy = 0;

    std::unique_ptr<Plot3D> plot = std::make_unique<Plot3D>(
        f, 
        make_grid(x_low, x_high, y_low, y_high, cells, cells)
    );

    while (running)
    {
        for (SDL_Event event; SDL_PollEvent(&event);) switch (event.type)
        {
        case SDL_QUIT:
            running = false;
            break;
        case SDL_WINDOWEVENT: switch (event.window.event)
            {
            case SDL_WINDOWEVENT_RESIZED:
                width = event.window.data1;
                height = event.window.data2;
                glViewport(0, 0, width, height);
                break;
            }
            break;
        case SDL_KEYDOWN:
            button_down[event.key.keysym.sym] = true;

            if (event.key.keysym.sym == SDLK_MINUS) {
                cells = std::max<size_t>(cells / 2, 1);
                plot->set_grid(make_grid(x_low, x_high, y_low, y_high, cells, cells));
            } else if (event.key.keysym.sym == SDLK_EQUALS) {
                cells = std::min<size_t>(cells * 2, 500);
                plot->set_grid(make_grid(x_low, x_high, y_low, y_high, cells, cells));
            }

            if (event.key.keysym.sym == SDLK_9) {
                isolines = std::max<size_t>(isolines - 1, 1);
                plot->set_isolines_count(isolines);
            } else if (event.key.keysym.sym == SDLK_0) {
                isolines = std::min<size_t>(isolines + 1, 30);
                plot->set_isolines_count(isolines);
            }

            break;
        case SDL_KEYUP:
            button_down[event.key.keysym.sym] = false;
            break;
        }

        if (!running)
            break;

        if (button_down[SDLK_LEFT]) {
            rotation_xz -= 0.023;
        }
        if (button_down[SDLK_RIGHT]) {
            rotation_xz += 0.023;
        } 
        if (button_down[SDLK_DOWN]) {
            rotation_xy -= 0.023;
        }
        if (button_down[SDLK_UP]) {
            rotation_xy += 0.023;
        } 

        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration_cast<std::chrono::duration<float>>(now - last_frame_start).count();
        last_frame_start = now;
        time += dt;

        plot->plot(time);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        float near = 0.1f;
        float far = 100.f;
        float top = near;
        float right = (top * width) / height;

        float view_angle = M_PI / 8.f;

        float view[16] =
        {
            1.f, 0.f, 0.f, 0.f,
            0.f, std::cos(view_angle), -std::sin(view_angle), 0.f,
            0.f, std::sin(view_angle), std::cos(view_angle), -17.f,
            0.f, 0.f, 0.f, 1.f,
        };

        float transform_xz[16] =
        {
            std::cos(rotation_xz), 0.f, std::sin(rotation_xz), 0.0,
            0.f, 1.0f, 0.f, 0.0,
            -std::sin(rotation_xz), 0.f, std::cos(rotation_xz), 0.0,
            0.f, 0.f, 0.f, 1.f,
        };

        float transform_xy[16] =
        {
            std::cos(rotation_xy), std::sin(rotation_xy), 0.f, 0.0,
            -std::sin(rotation_xy), std::cos(rotation_xy), 0.f, 0.0,
            0.f, 0.f, 1.f, 0.0,
            0.f, 0.f, 0.f, 1.f,
        };

        float projection[16] =
        {
            near / right, 0.f, 0.f, 0.f,
            0.f, near / top, 0.f, 0.f,
            0.f, 0.f, - (far + near) / (far - near), - 2.f * far * near / (far - near),
            0.f, 0.f, -1.f, 0.f,
        };

        glUseProgram(mesh_program_);
        glUniformMatrix4fv(view_location_mesh, 1, GL_TRUE, view);
        glUniformMatrix4fv(transform_xz_location_mesh, 1, GL_TRUE, transform_xz);
        glUniformMatrix4fv(transform_xy_location_mesh, 1, GL_TRUE, transform_xy);
        glUniformMatrix4fv(projection_location_mesh, 1, GL_TRUE, projection);
        glUniform4f(low_color_location_mesh, 0xA8 / 255.0, 0x26 / 255.0, 0x54 / 255.0, 0.0);
        glUniform4f(high_color_location_mesh, 0xFD / 255.0, 0x8C / 255.0, 0x69 / 255.0, 0.0);	

        glUniform1f(min_value_location_mesh, plot->get_min_value());
        glUniform1f(max_value_location_mesh, plot->get_max_value());

        glBindVertexArray(plot->get_mesh_vao());
        glDrawElements(GL_TRIANGLES, plot->get_mesh_elements_count(), GL_UNSIGNED_INT, (void*)0);

        glUseProgram(isolines_program_);
        glUniformMatrix4fv(view_location_isolines, 1, GL_TRUE, view);
        glUniformMatrix4fv(transform_xz_location_isolines, 1, GL_TRUE, transform_xz);
        glUniformMatrix4fv(transform_xy_location_isolines, 1, GL_TRUE, transform_xy);
        glUniformMatrix4fv(projection_location_isolines, 1, GL_TRUE, projection);
        glUniform4f(color_location_isolines, 0. / 255.0, 0. / 255.0, 0. / 255.0, 0.0);	

        glBindVertexArray(plot->get_isolines_vao());
        glDrawElements(GL_LINES, plot->get_isolines_elements_count(), GL_UNSIGNED_INT, (void*)0);

        SDL_GL_SwapWindow(window_);
    }
}

