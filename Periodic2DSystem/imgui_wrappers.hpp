
#include "parameters.hpp"

#ifndef _IMGUI_CONTROLS_
#define _IMGUI_CONTROLS_
using namespace sim_2d;

#include "gl_wrappers.hpp"

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include <functional>
#include <set>

#include "parameters.hpp"

static std::function<void(int, Uniform)> s_sim_params_set;
static std::function<void(int, int, std::string)> s_sim_params_set_string;
static std::function<Uniform(int)> s_sim_params_get;
static std::function<void(int, std::string, float)> s_user_edit_set_value;
static std::function<float(int, std::string)> s_user_edit_get_value;
static std::function<std::string(int)>
    s_user_edit_get_comma_separated_variables;
static std::function<void(int)> s_button_pressed;
static std::function<void(int, int)> s_selection_set;
static std::function<void(int, std::string, float)>
    s_sim_params_set_user_float_param;

static ImGuiIO global_io;
static std::map<int, std::string> global_labels;

void edit_label_display(int c, std::string text_content) {
    global_labels[c] = text_content;
}

void display_parameters_as_sliders(
    int c, std::set<std::string> variables) {
    std::string string_val = "[";
    for (auto &e: variables)
        string_val += """ + e + "", ";
    string_val += "]";
    string_val 
        = "modifyUserSliders(" + std::to_string(c) + ", " + string_val + ");";
    // TODO
}

void start_gui(void *window) {
    bool show_controls_window = true;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsClassic();
    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow *)window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void imgui_controls(void *void_params) {
    SimParams *params = (SimParams *)void_params;
    for (auto &e: global_labels)
        params->set(e.first, 0, e.second);
    if (ImGui::SliderInt("Steps per frame", &params->stepsPerFrame, 0, 20))
            s_sim_params_set(params->STEPS_PER_FRAME, params->stepsPerFrame);
    if (ImGui::SliderFloat("Wave function brightness", &params->brightness, 0.0, 2.0))
           s_sim_params_set(params->BRIGHTNESS, params->brightness);
    if (ImGui::SliderFloat("Potential brightness", &params->brightnessV, 0.0, 2.0))
           s_sim_params_set(params->BRIGHTNESS_V, params->brightnessV);
    if (ImGui::SliderFloat("Particle brightness", &params->brightnessParticles, 0.0, 1.0))
           s_sim_params_set(params->BRIGHTNESS_PARTICLES, params->brightnessParticles);
    if (ImGui::SliderFloat("New wave function size", &params->waveFuncSize, 0.02, 0.05))
           s_sim_params_set(params->WAVE_FUNC_SIZE, params->waveFuncSize);
    if (ImGui::SliderFloat("Mass", &params->m, 1.0, 5.0))
           s_sim_params_set(params->M, params->m);
    if (ImGui::SliderFloat("Time step", &params->dt, 0.0, 0.3))
           s_sim_params_set(params->DT, params->dt);
    if (ImGui::BeginMenu("Use mouse to:")) {
        if (ImGui::MenuItem( "Create new wave function"))
            s_selection_set(params->MOUSE_USAGE_ENTRY, 0);
        if (ImGui::MenuItem( "Draw potential barrier"))
            s_selection_set(params->MOUSE_USAGE_ENTRY, 1);
        if (ImGui::MenuItem( "Erase potential barrier"))
            s_selection_set(params->MOUSE_USAGE_ENTRY, 2);
        ImGui::EndMenu();
    }
    if (ImGui::SliderInt("Particle count upon placement of new wave function", &params->numberOfParticles, 4096, 1048576))
            s_sim_params_set(params->NUMBER_OF_PARTICLES, params->numberOfParticles);
    ImGui::Checkbox("Show particle trails", &params->showTrails);
    if (ImGui::BeginMenu("Preset V(x, y, t)")) {
        if (ImGui::MenuItem( "((x/width)^2 + (y/height)^2)"))
            s_selection_set(params->PRESET_POTENTIAL_DROPDOWN, 0);
        if (ImGui::MenuItem( "0"))
            s_selection_set(params->PRESET_POTENTIAL_DROPDOWN, 1);
        if (ImGui::MenuItem( "amp*((x/width)^2 + (y/height)^2)"))
            s_selection_set(params->PRESET_POTENTIAL_DROPDOWN, 2);
        if (ImGui::MenuItem( "0.4*(step(-y^2+(height*0.04)^2)+step(y^2-(height*0.06)^2))*step(-x^2+(width*0.01)^2)"))
            s_selection_set(params->PRESET_POTENTIAL_DROPDOWN, 3);
        if (ImGui::MenuItem( "1.0/sqrt(x^2+y^2)+1.0/sqrt((x-0.25*width)^2+(y-0.25*height)^2)"))
            s_selection_set(params->PRESET_POTENTIAL_DROPDOWN, 4);
        if (ImGui::MenuItem( "(x*cos(w*t/200) + y*sin(w*t/200))/500+0.01"))
            s_selection_set(params->PRESET_POTENTIAL_DROPDOWN, 5);
        if (ImGui::MenuItem( "0.5*(tanh(75.0*(((x/width)^2+(y/height)^2)^0.5-0.45))+1.0)"))
            s_selection_set(params->PRESET_POTENTIAL_DROPDOWN, 6);
        ImGui::EndMenu();
    }
    ImGui::Text("(Please note: to ensure stability, clamping is applied to the potential so that |V(x, y, t)| < 1.)");
    ImGui::Checkbox("Add absorbing boundaries (may incur instability!)", &params->addAbsorbingBoundaries);

}

bool outside_gui() {{
    return !global_io.WantCaptureMouse;
}}

void display_gui(void *data) {{
    global_io = ImGui::GetIO();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    bool val = true;
    ImGui::Begin("Controls", &val);
    ImGui::Text("WIP AND INCOMPLETE");
    imgui_controls(data);
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}}

#endif
