#include "gl_wrappers.hpp"
#include "parameters.hpp"
#include "histogram.hpp"
#include "initial_normal_mode_wave_function.hpp"

#ifndef _SIM_2D_
#define _SIM_2D_

namespace sim_2d {

struct Frames {
    TextureParams view_tex_params;
    TextureParams image_tex_params;
    TextureParams configs_view_tex_params;
    TextureParams hist_tex_params;
    TextureParams initial_values_tex_params;
    Quad tmp;
    Quad initial_values;
    Quad hist;
    Quad image;
    RenderTarget configs_view;
    RenderTarget view;
    WireFrame quad_wire_frame;
    Frames(
        const SimParams &sim_params,
        int view_width, int view_height);
};

struct GLSLPrograms {
    uint32_t scale;
    uint32_t copy;
    uint32_t height_map;
    uint32_t configs_view;
    uint32_t modes;
    GLSLPrograms();
};


class Simulation {
    GLSLPrograms m_programs;
    Frames m_frames;
    histogram::Histogram2D m_hist;
    std::vector<double> m_configs;  // Stores the Monte Carlo samples
    // Stores the transform matrix for going from 
    // position to normal coordinates
    std::vector<double> m_positions2normals;
    // Transform matrix from going from normal to position coordinates
    std::vector<double> m_normals2positions;
    // Stores the angular frequencies
    double m_omega[MAX_SIZE]; 
    InitialNormalModeWaveFunction m_initial_wave_func;
    void reset_coord_transform(const SimParams &sim_params);
    void compute_coherent_state_configurations(SimParams &sim_params);
    void compute_squeezed_state_configurations(SimParams &sim_params);
    void compute_stationary_state_configurations(SimParams &sim_params);
    void compute_single_excitations_configurations(SimParams &sim_params);
    protected:
    void load_initial_values_texture(const SimParams &sim_params);
    void fill_plot_color_hist(const SimParams &sim_params);
    void plot_non_hist_normals(const SimParams &sim_params);
    void plot_non_hist_positions(const SimParams &sim_params);
    void plot_exact_normals(const SimParams &sim_params);
    void normals2positions(const SimParams &sim_params);
    const GLSLPrograms& get_programs();
    Frames& get_frames();
    const std::vector<double> &get_configs();
    public:
    Simulation(const SimParams &sim_params,
        int view_width, int view_height);
    void compute_configurations(SimParams &sim_params);
    const RenderTarget &render_view(const SimParams &sim_params);
    void reset_oscillator_count(const SimParams &sim_params);
    void reset_omega(const SimParams &sim_params);
    void modify_boundaries(const SimParams &sim_params);
    void cursor_set_initial_wave_function(
        SimParams &sim_params, Vec2 cursor_pos);
    void set_relative_standard_deviation(float val);
    void modify_dispersion_with_user_input(
        const SimParams &sim_params, const std::string &s);
};

};

#endif