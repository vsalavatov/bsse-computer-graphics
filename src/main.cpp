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


struct Meatball {
    std::function<Vec2(float t)> position;
    float c;
    float r;
};
auto make_meatballs = [](std::vector<Meatball> spec) -> Plottable {
    return [spec=std::move(spec)] (float x, float y, float t) -> float {
        float result = 0;
        for (auto& m : spec) {
            Vec2 pos = m.position(t);
            result += m.c * exp(- (sqr(x - pos.x) + sqr(y - pos.y)) / sqr(m.r));
        }
        return result;
    };
};

auto meatballs = make_meatballs({
    Meatball{
        [](float t) { return Vec2{ 2.0f + sin(t), 3.0f + 2 * cos(t) + sqr(sin(t * 1.3f)) };},
        -3.0,
        5.0
    },
    Meatball{
        [](float t) { return Vec2{ 4.f * sin(t * -0.3f) - 1.f, 1.1f + sin(t) };},
        4.5,
        2.4
    },
    Meatball{
        [](float t) { return Vec2{ 1.f * sqr(cos(t * -5.f)) - 1.2f, 0.1f + 2.f * cos(t) - 0.3f * sin(t*0.7f) + 0.1f * sqr(sin(t)) };},
        2.1,
        10
    },
});

int main() try {

    auto plotter = PlotterUI();
    plotter.run(meatballs);

} catch (std::exception const & e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
