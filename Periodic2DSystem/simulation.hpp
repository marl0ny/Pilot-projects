#include "gl_wrappers.hpp"
#include "parameters.hpp"

#ifndef _SIMULATION_
#define _SIMULATION_

using namespace sim_2d;

struct Frames {
    TextureParams wave_sim_tex_params;
    TextureParams trajectories_tex_params;
    TextureParams view_higher_res_tex_params;
    Quad psi[3];
    struct {
        Quad particles;
        Quad tmp;
        Quad rk4[5];
    } trajectories;
    Quad potential;
    Quad tmp;
    RenderTarget render_intermediates[3];
    RenderTarget particles_view;
    RenderTarget render;
    WireFrame quad_wire_frame;
    WireFrame trajectories_wire_frame;
    Frames(const TextureParams &default_tex_params, const SimParams &params);
    void reset_wave_function_dimensions(IVec2 texel_dimensions_2d);
    void reset_trajectories_dimensions(int number_of_particles);
};

struct Programs {
    unsigned int copy;
    unsigned int scale;
    unsigned int user_defined;
    unsigned int blur;
    unsigned int add2, add3;
    unsigned int domain_color;
    unsigned int gray_scale;
    unsigned int modify_potential_entry;
    unsigned int sketch_potential;
    unsigned int time_step;
    unsigned int init_wave_packet;
    unsigned int guide;
    unsigned int forward_euler;
    unsigned int rk4;
    unsigned int display_circles;
    Programs();
};

class Simulation {
    Programs m_programs;
    Frames m_frames;
    Quad *m_psi_ptr[3];
    int m_time_step_count;
    std::vector<unsigned char> m_image_data;
    void compute_guide(
        Quad &q2, const Quad *wave, const Quad &q,
        const SimParams &params);
    void compute_guide(
        Quad &q2,
        const Quad *wave,
        const Quad &q, double dt, const Quad &q_dot,
        const SimParams &params);
    void trajectories_time_step_rk4(const SimParams &params);
    public:
    Simulation(const TextureParams &default_tex_params,
               const SimParams &params);
    void new_wave_function(
        const SimParams &params, Vec2 tex_position, Vec2 wave_num);
    void new_particles(
        const SimParams &params, Vec2 tex_position);
    const RenderTarget &view(SimParams &params);
    void time_step(const SimParams &params);
    void add_user_defined_potential(
        const SimParams &params,
        unsigned int program,
        const std::map<std::string, float> &uniforms);
    void sketch_potential(
        const SimParams &params,
        const Vec2 &position, float amplitude
    );
    void set_potential_from_image(
        const SimParams &params, 
        const uint8_t *image_data,
        IVec2 image_dimensions);
    std::vector<unsigned char> &get_image_data();
};

#endif