
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
    if (ImGui::SliderFloat("Time elapsed per frame", &params->dt, 0.0, 10.0))
           s_sim_params_set(params->DT, params->dt);
    if (ImGui::SliderInt("Number of oscillators", &params->numberOfOscillators, 8, 256))
            s_sim_params_set(params->NUMBER_OF_OSCILLATORS, params->numberOfOscillators);
    if (ImGui::BeginMenu("Boundary type")) {
        if (ImGui::MenuItem( "Zero at endpoints"))
            s_selection_set(params->BOUNDARY_TYPE, 0);
        if (ImGui::MenuItem( "Periodic"))
            s_selection_set(params->BOUNDARY_TYPE, 1);
        ImGui::EndMenu();
    }
    ImGui::Text("--------------------------------------------------------------------------------");
    ImGui::Text("Metropolis algorithm configuration");
    if (ImGui::SliderFloat("Relative step size", &params->relativeDelta, 0.0, 1.0))
           s_sim_params_set(params->RELATIVE_DELTA, params->relativeDelta);
    ImGui::Text("Acceptance rate");
    if (ImGui::SliderInt("Requested number of Monte Carlo samples", &params->numberOfMCSteps, 10, 100000))
            s_sim_params_set(params->NUMBER_OF_M_C_STEPS, params->numberOfMCSteps);
    ImGui::Text("--------------------------------------------------------------------------------");
    ImGui::Text("Samples display options");
    if (ImGui::SliderFloat("Brightness", &params->alphaBrightness, 0.0, 0.1))
           s_sim_params_set(params->ALPHA_BRIGHTNESS, params->alphaBrightness);
    ImGui::Text("Colour 1 (r, g, b)");
    if (ImGui::SliderFloat("colorOfSamples1[0]", &params->colorOfSamples1.ind[0], 0.0, 1.0))
           s_sim_params_set(params->COLOR_OF_SAMPLES1, params->colorOfSamples1);
    if (ImGui::SliderFloat("colorOfSamples1[1]", &params->colorOfSamples1.ind[1], 0.0, 1.0))
           s_sim_params_set(params->COLOR_OF_SAMPLES1, params->colorOfSamples1);
    if (ImGui::SliderFloat("colorOfSamples1[2]", &params->colorOfSamples1.ind[2], 0.0, 1.0))
           s_sim_params_set(params->COLOR_OF_SAMPLES1, params->colorOfSamples1);
    ImGui::Text("Colour 2 (r, g, b)");
    if (ImGui::SliderFloat("colorOfSamples2[0]", &params->colorOfSamples2.ind[0], 0.0, 1.0))
           s_sim_params_set(params->COLOR_OF_SAMPLES2, params->colorOfSamples2);
    if (ImGui::SliderFloat("colorOfSamples2[1]", &params->colorOfSamples2.ind[1], 0.0, 1.0))
           s_sim_params_set(params->COLOR_OF_SAMPLES2, params->colorOfSamples2);
    if (ImGui::SliderFloat("colorOfSamples2[2]", &params->colorOfSamples2.ind[2], 0.0, 1.0))
           s_sim_params_set(params->COLOR_OF_SAMPLES2, params->colorOfSamples2);
    if (ImGui::BeginMenu("Plot type")) {
        if (ImGui::MenuItem( "Lines"))
            s_selection_set(params->DISPLAY_TYPE, 0);
        if (ImGui::MenuItem( "Scatter"))
            s_selection_set(params->DISPLAY_TYPE, 1);
        if (ImGui::MenuItem( "Multi-coloured histogram"))
            s_selection_set(params->DISPLAY_TYPE, 2);
        ImGui::EndMenu();
    }
    ImGui::Checkbox("Display samples in normal coordinates", &params->showNormalCoordSamples);
    ImGui::Text("--------------------------------------------------------------------------------");
    ImGui::Text("Normal mode analytic wave function display");
    ImGui::Checkbox("Colour phase", &params->colorPhase);
    if (ImGui::SliderFloat("Brightness (modesBrightness)", &params->modesBrightness, 0.0, 10.0))
           s_sim_params_set(params->MODES_BRIGHTNESS, params->modesBrightness);
    ImGui::Text("--------------------------------------------------------------------------------");
    ImGui::Text("Wave function modification options");
    ImGui::Checkbox("Coherent (Single product of coherent modes)", &params->useCoherentStates);
    ImGui::Checkbox("Squeezed (Single product)", &params->useSqueezed);
    ImGui::Checkbox("Energy eigenstate (Expect poor Metropolis convergence for highly excited modes. Single product of normal mode eigenstates only.)", &params->useStationary);
    ImGui::Checkbox("Superposition of singly-excited normal modes", &params->useSingleExcitations);
    ImGui::Text("(Will be difficult to differentiate any differences from the ground unless a large number of samples are used.)");
    ImGui::Text("If 'Coherent' or 'Squeezed' selected:");
    if (ImGui::BeginMenu("Behaviour when modifying a selected normal mode amplitude expectation value with the mouse cursor:")) {
        if (ImGui::MenuItem( "Change selected while setting others to zero"))
            s_selection_set(params->CLICK_ACTION_NORMAL, 0);
        if (ImGui::MenuItem( "Modify selection only"))
            s_selection_set(params->CLICK_ACTION_NORMAL, 1);
        ImGui::EndMenu();
    }
    ImGui::Text("If 'Squeezed' selected:");
    if (ImGui::SliderFloat("Global squeezing factor (compared to coherent)", &params->squeezedFactorGlobal, 0.5, 10.0))
           s_sim_params_set(params->SQUEEZED_FACTOR_GLOBAL, params->squeezedFactorGlobal);
    if (ImGui::SliderFloat("Squeeze factor for an individual normal mode", &params->squeezedFactor, 0.5, 10.0))
           s_sim_params_set(params->SQUEEZED_FACTOR, params->squeezedFactor);
    ImGui::Text("(Click on a normal mode for this slider to take effect)");
    ImGui::Text("If 'Energy eigenstate' selected:");
    ImGui::Checkbox("Click on normal mode to add energy", &params->addEnergy);
    ImGui::Checkbox("Remove energy instead", &params->removeEnergy);
    ImGui::Text("--------------------------------------------------------------------------------");
    ImGui::Text("Dispersion relation options");
    if (ImGui::BeginMenu("Preset dispersion relation ω(k)")) {
        if (ImGui::MenuItem( "2*sin((pi/2)*(abs(k)/k_max))"))
            s_selection_set(params->PRESET_DISPERSION_RELATION, 0);
        if (ImGui::MenuItem( "pi*(abs(k)/k_max)"))
            s_selection_set(params->PRESET_DISPERSION_RELATION, 1);
        ImGui::EndMenu();
    }
    ImGui::Checkbox("Take screenshots", &params->imageRecord);

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
