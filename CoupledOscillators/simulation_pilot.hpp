#include "simulation.hpp"

#ifndef _SIMULATION_PILOT_
#define _SIMULATION_PILOT_

namespace sim_pilot {

struct Trajectories {
    Quad x[2];
    int prev;
    int next;
};

struct Frames {
    TextureParams trajectories_tex_params;
    TextureParams transform_tex_params;
    // Quad tmp0;
    // Quad tmp1;
    // Quad tmp2;
    // Quad tmp3;
    // Quad tmp4;
    Quad position_trajectories;
    Trajectories trajectories;
    Quad discrete_sine;
    Quad discrete_sine_cosine;
    WireFrame trajectories_view_wire_frame;
    Frames(const sim_2d::SimParams &sim_params, 
        int view_width, int view_height);
    void change_simulation_dimensions(IVec2 d_2d);
};

struct GLSLPrograms {
    uint32_t time_step;
    uint32_t trajectories_view;
    uint32_t norms2pos;
    uint32_t orthogonal_transforms;
    GLSLPrograms();
};

class Simulation: sim_2d::Simulation {
    // sim_2d::Simulation m_simulation;
    GLSLPrograms m_programs;
    Frames m_frames;
    void load_config_to_texture(int number_of_oscillators);
    void normals2positions(const sim_2d::SimParams &sim_params);
    void plot_non_hist_normals(const sim_2d::SimParams &sim_params);
    void plot_non_hist_positions(const sim_2d::SimParams &sim_params);
    void make_transform_textures(int number_of_oscillators);
    public:
    Simulation(const sim_2d::SimParams &sim_params,
        int view_width, int view_height);
    void reset_initial_values_texture(const sim_2d::SimParams &sim_params);
    const RenderTarget &render_view(const sim_2d::SimParams &sim_params);
    void time_step(const sim_2d::SimParams &sim_params);
    void compute_configurations(sim_2d::SimParams &sim_params);
    void reset_oscillator_count(const sim_2d::SimParams &sim_params);
    void reset_omega(const sim_2d::SimParams &sim_params);
    void modify_boundaries(const sim_2d::SimParams &sim_params);
    void cursor_set_initial_wave_function(
        sim_2d::SimParams &sim_params, Vec2 cursor_pos);
    void set_relative_standard_deviation(float val);
    void modify_dispersion_with_user_input(
        const sim_2d::SimParams &sim_params, const std::string &s);
};

}

#endif