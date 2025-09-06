#include "gl_wrappers.hpp"
#include "glfw_window.hpp"
#include "parameters.hpp"
#include "interactor.hpp"
#include "parse.hpp"
#include "user_edit_glsl.hpp"
#include "simulation.hpp"

#include <GLFW/glfw3.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/bind.h>
#endif
#include <functional>
#include <utility>

#include "wasm_wrappers.hpp"


static std::function <void()> s_loop;
#ifdef __EMSCRIPTEN__
static void s_main_loop() {
    s_loop();
}
#endif

using namespace sim_2d;


void simulation_ui_interface_handler(
    MainGLFWQuad main_render,
    TextureParams default_tex_params,  // Default texture parameters
    SimParams &params  // Parameters of the simulations
) {
    Interactor interactor(main_render.get_window());
    Simulation sim(default_tex_params, params);
    SimParams modified_params {};
    UserProgramsManager user_text_edit {};

    // For handling mouse or touch interation.
    std::optional<Vec2> start_position;
    std::vector<Vec2> cursor_positions {};
    std::optional<std::pair<Vec2, Vec2>> start_double_touches;
    std::vector<std::pair<Vec2, Vec2>> double_touches_positions {};
    Quaternion rotation = Quaternion{.i=0.0, .j=0.0, .k=0.0, .real=1.0};

    {
        /* Set those parameters of the Parameters struct that are treated
        as uniforms by GLSL shaders.*/
        s_sim_params_set = [&params, &user_text_edit]
            (int c, Uniform u) {
            params.set(c, u);
            if (c == params.ADD_ABSORBING_BOUNDARIES) {
                user_text_edit.queue_current();
            }
        };
        /* Get those parameters of the Parameters struct that can be
        inputed as uniforms to GLSL shaders.*/
        s_sim_params_get = [&params]
            (int c) -> Uniform {
            return params.get(c);
        };
        /* String parametres can't be configured as uniforms, so
        are set using a different function.*/
        s_sim_params_set_string = [&params, &user_text_edit]
            (int c, int index, std::string val) {
            params.set(c, index, val);
            if (c == params.USER_TEXT_ENTRY) {
                int program;
                std::set<std::string> variables_set = 
                    initialize_glsl_program_from_strings(
                        program, params.userTextEntry);
                user_text_edit.add_new_program(program, variables_set);
                display_parameters_as_sliders(c, variables_set,  {"t"});
            }
        };
        /* Perform an action upon the press of a button. */
        s_button_pressed = [&params]
            (int param_code) {
        };
        /* Floating-point value parameters and their associated sliders
        can be created by the user. This notifies and keeps track of any
        newly created user-defined parameter. The user defined paramters are
        not part of the Parameters struct, so are stored separately.*/
        s_sim_params_set_user_float_param = [&user_text_edit]
            (int c, std::string var_name, float value) {
            user_text_edit.add_seen_variable(var_name, value);
            user_text_edit.queue_current();
        };
        /* Upon a change of a dropdown or selection menu, change its
        corresponding selection parameter in the Parameters struct so that
        it matches the dropdown.*/
        s_selection_set = [&params, &user_text_edit]
            (int c, int val) {
            if (c == params.PRESET_POTENTIAL_DROPDOWN) {
                params.presetPotentialDropdown.selected = val;
                int program;
                std::set<std::string> variables_set = 
                    initialize_glsl_program_from_strings(
                        program,
                        {params.presetPotentialDropdown.options[val]});
                user_text_edit.add_new_program(program, variables_set);
                display_parameters_as_sliders(
                    params.USER_TEXT_ENTRY, variables_set, {"t"});
            }
            if (c == params.MOUSE_USAGE_ENTRY) {
                params.mouseUsageEntry.selected = val;
            }
        };
    }

    { // Initial configuration from the default preset option
        int program;
        int index = params.presetPotentialDropdown.selected;
        std::set<std::string> variables_set 
            = initialize_glsl_program_from_strings(
                program,
                {params.presetPotentialDropdown.options[index]});
        user_text_edit.add_new_program(program, variables_set);
    }

    s_loop = [&] {

        enum {
            MOUSE_USAGE_NEW_WAVE_FUNC=0, 
            MOUSE_USAGE_DRAW_V=1, MOUSE_USAGE_ERASE_V=2};
        if (start_position.has_value()) {
            if (cursor_positions.size() > 1) {
                Vec2 delta_2d = interactor.get_mouse_delta();
                Vec3 delta {.ind={delta_2d[0], delta_2d[1], 0.0}};
                Vec3 view_vec {.ind={0.0, 0.0, -1.0}};
                Vec3 axis = cross_product(delta, view_vec);
                Quaternion rot = Quaternion::rotator(
                    3.0*axis.length(), axis);
                rotation = rotation*rot;
                Vec2 wave_num = cursor_positions.back() - start_position.value();
                wave_num.x *= float(params.waveDiscretizationDimensions[0])/4.0;
                wave_num.y *= float(params.waveDiscretizationDimensions[1])/4.0;
                if (params.mouseUsageEntry.selected 
                        == MOUSE_USAGE_NEW_WAVE_FUNC) {
                    sim.new_wave_function(
                        params, start_position.value(), wave_num);
                } else if (params.mouseUsageEntry.selected == MOUSE_USAGE_DRAW_V) {
                    sim.sketch_potential(params, cursor_positions.back(), 0.05);
                } else if (params.mouseUsageEntry.selected == MOUSE_USAGE_ERASE_V) {
                    sim.sketch_potential(params, cursor_positions.back(), -0.05);
                }
            } else {
                if (params.mouseUsageEntry.selected 
                        == MOUSE_USAGE_NEW_WAVE_FUNC) {
                    sim.new_wave_function(
                        params, 
                        cursor_positions.back(), Vec2{.ind{0.0, 0.0}});
                    sim.new_particles(params, start_position.value());
                }
            }
            params.t = 0.0;
        }
        if (!user_text_edit.program_queued() && user_text_edit.is_time_dependent()) {
            user_text_edit.queue_current();
        }
        if (user_text_edit.program_queued()) {
            UserDefinedProgram user_defined = user_text_edit.expend_program();
            sim.add_user_defined_potential(
                params, user_defined.program, user_defined.uniforms);

        }
        for (int i = 0; 
            i < params.stepsPerFrame  
            && !(start_position.has_value() 
                 && params.mouseUsageEntry.selected 
                 == MOUSE_USAGE_NEW_WAVE_FUNC); i++) {
            sim.time_step(params);
            params.t += params.dt;
        }
        
        main_render.draw(
            sim.view(params));

        auto poll_events = [&] {
            // Tell GLFW to poll events
            glfwPollEvents();

            // Get user interaction events
            interactor.click_update(main_render.get_window());

            // Handle mouse or single touch events
            Vec2 pos = interactor.get_mouse_position();
            if (pos.x > 0.0 && pos.x < 1.0 && 
                pos.y > 0.0 && pos.y < 1.0 && 
                interactor.left_pressed()) {
                if (!start_position.has_value())
                    start_position = pos;
                cursor_positions.push_back(pos);
            }
            if (interactor.left_released()) {
                if (start_position.has_value()) {
                    start_position.reset();
                    cursor_positions.clear();
                }
            }            

            // Handle double touch events
            Vec2 double_touches[2];
            double_touches[0] = interactor.get_double_touch_position(0);
            double_touches[1] = interactor.get_double_touch_position(1);
            if (interactor.double_touch_active()
                && double_touches[0].x > 0.0 && double_touches[0].x < 1.0
                && double_touches[0].y > 0.0 && double_touches[0].y < 1.0
                && double_touches[1].x > 0.0 && double_touches[1].x < 1.0
                && double_touches[1].y > 0.0 && double_touches[1].y < 1.0) {
                if (!start_double_touches.has_value())
                    start_double_touches = 
                        {double_touches[0], double_touches[1]};
                double_touches_positions.push_back(
                        {double_touches[0], double_touches[1]});
            }
            if (interactor.double_touch_released()) {
                if (start_double_touches.has_value()) {
                    start_double_touches.reset();
                    double_touches_positions.clear();
                }
            }

            #ifndef __EMSCRIPTEN__
            #endif
        };
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
    int window_width = 1024, window_height = 1024;
    if (argc >= 3) {
        window_width = std::atoi(argv[1]);
        window_height = std::atoi(argv[2]);
    }
    int filter_type = GL_LINEAR;
    if (argc >= 4) {
        std::string s(argv[3]);
        if (s == "nearest")
            filter_type = GL_NEAREST;
    }
    SimParams params {};
    TextureParams default_tex_params = {
        .format=GL_RGBA32F,
        .width=(unsigned int)window_width,
        .height=(unsigned int)window_height,
        .generate_mipmap=!(filter_type == GL_NEAREST),
        .wrap_s=GL_CLAMP_TO_EDGE,
        .wrap_t=GL_CLAMP_TO_EDGE,
        .mag_filter=(unsigned int)filter_type,
        .min_filter=(unsigned int)filter_type
    };
    MainGLFWQuad 
    main_render (default_tex_params.width, default_tex_params.height);
    simulation_ui_interface_handler(
        main_render, default_tex_params, params);
    return 0;
}
