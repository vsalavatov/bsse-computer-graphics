#ifdef WIN32
#include <SDL.h>
#undef main
#else
#include <SDL2/SDL.h>
#endif

#include <GL/glew.h>

#include <string_view>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <vector>
#include <map>
#include <cmath>

#include "plotter_ui.h"

auto sqr(auto x) { return x * x; };

auto some_sin_cos_stuff = [](float x, float y, float t) { return sin(x) + cos(y) + sin(t) * sin(t);};
auto booba = [](float x, float y, float t) {
    auto shift = sin(t) * 0.7 + sin(t*1.3)*0.3;
    return 3 * (1 + 0.5 * sqr(sin(t)))*(
        exp(-sqr(sqr(x-4 + shift)+sqr(y-4 - shift))/1000) + 
        exp(-sqr(sqr(x+4 + shift)+sqr(y+4- shift))/1000) + 
        0.2 * exp(-sqr(sqr(x+4 + shift)+sqr(y+4- shift))) +
        0.2 * exp(-sqr(sqr(x-4 + shift)+sqr(y-4- shift)))
    );
};


int main() try {

    auto plotter = PlotterUI();
    plotter.run(booba);

} catch (std::exception const & e)
{
	std::cerr << e.what() << std::endl;
	return EXIT_FAILURE;
}
