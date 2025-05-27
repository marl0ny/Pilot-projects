#include "glfw_window.hpp"
#include "interactor.hpp"
#include "parameters.hpp"
#include "simulation_pilot.hpp"

#ifdef __EMSCRIPTEN__
#include "wasm_wrappers.hpp"
#else
#include "imgui_wrappers.hpp"
#endif

static std::function <void()> s_loop;
#ifdef __EMSCRIPTEN__
static void s_main_loop() {
    s_loop();
}
#endif


void oscillators(MainGLFWQuad main_render,
    int window_width, int window_height,
    sim_2d::SimParams &params,
    Interactor interactor) {
    sim_pilot::Simulation sim(
        params, window_width, window_height);
    sim.compute_configurations(params);
    {
        s_sim_params_set = [&](int c, Uniform u) {
            if (c == params.SQUEEZED_FACTOR_GLOBAL) {
                sim.set_relative_standard_deviation(1.0/u.f32);
                if (params.useSqueezed) {
                    params.t = 0.0;
                    sim.compute_configurations(params);
                }
            }
            params.set(c, u);
            if (c == params.NUMBER_OF_OSCILLATORS) {
                sim.reset_oscillator_count(params);
                sim.compute_configurations(params);
            }
            if (c == params.RELATIVE_DELTA) {
                sim.compute_configurations(params);
            }
        };
        s_sim_params_get = [&params](int c) -> Uniform {
            return params.get(c);
        };
        // s_sim_params_set_string = [&params, &sim](int c, int i, std::string s) {
        //     params.set(c, i, s);
        //     sim.modify_dispersion_with_user_input(
        //         params, params.dispersionRelation[0]);
        //     // display_parameters_as_sliders(c, {});
        // };
        s_selection_set = [&params, &sim](
            int c, int val) {
            printf("%d\n", c);
            if (c == params.DISPLAY_TYPE)
                params.displayType.selected = val;
            if (c == params.CLICK_ACTION_NORMAL)
                params.clickActionNormal.selected = val;
            if (c == params.BOUNDARY_TYPE) {
                params.boundaryType.selected = val;
                sim.modify_boundaries(params);
                params.t = 0.0;
            }
            if (c == params.PRESET_DISPERSION_RELATION) {
                params.presetDispersionRelation.selected = val;
                sim.reset_omega(params);
                // sim.compute_configurations(params);
            }
        };
    }
    start_gui(main_render.get_window());
    s_loop = [&] {
        // sim.compute_configurations(params);
        sim.time_step(params);
        main_render.draw(sim.render_view(params));
        params.t += params.dt;
        params.stepCount++;
        Vec2 pos = interactor.get_mouse_position();
        if (params.stepCount % 3 == 0) {
            std::string text_content = "Acceptance rate: "
                + std::to_string(100.0*params.acceptanceRate) 
                + "% (33-50% ideal)";
            edit_label_display(params.ACCEPTANCE_RATE_LABEL, text_content);
        }
        auto poll_events = [&] {
            glfwPollEvents();
            interactor.click_update(main_render.get_window());
            if (pos.x > 0.0 && pos.x < 1.0 && 
                pos.y > 0.0 && pos.y < 1.0 && interactor.left_pressed()
                && outside_gui()) {
                params.t -= params.dt;
                sim.cursor_set_initial_wave_function(params, pos);
            }
        };
        display_gui(&params);
        poll_events();
        glfwSwapBuffers(main_render.get_window());
    };
    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(s_main_loop, 0, true);
    #else
    while (!glfwWindowShouldClose(main_render.get_window()))
        s_loop();
    #endif

}

int main(int argc, char *argv[]) {
    int window_width = 2500, window_height = 1500;
    // Construct the main window quad
    if (argc >= 3) {
        window_width = std::atoi(argv[1]);
        window_height = std::atoi(argv[2]);
    }
    auto main_quad = MainGLFWQuad(window_width, window_height);
    // Initialize Interactor instance
    Interactor interactor(main_quad.get_window());
    sim_2d::SimParams sim_params;
    oscillators(main_quad, 
        window_width, window_height, sim_params, interactor);
    return 1;
}