#include "simulation_pilot.hpp"

#include "trajectories_wire_frame.hpp"
#include "write_to_png.hpp"
#include "orthogonal_transforms.hpp"

static struct IVec2 decompose(unsigned int n) {
    struct IVec2 d = {.ind={(int)n, 1}};
    int i = 1;
    for (; i*i < n; i++) {}
    for (; n % i; i--) {}
    d.ind[0] = ((n / i) > i)? (n / i): i;
    d.ind[1] = ((n / i) < i)? (n / i): i;
    return d;
}

static struct IVec3 decompose_3(unsigned int n) {
    struct IVec3 d = {.ind={(int)n, 1, 1}};
    int i = 1;
    for (; i*i*i < n; i++) {}
    for (; n % i; i--) {}
    IVec2 d_2d = decompose(n/i);
    d.ind[0] = d_2d[0];
    d.ind[1] = d_2d[1];
    d.ind[2] = i; 
    return d;
}

IVec2 get_dimensions(int n_large, int n_small) {
    IVec3 prod3 = decompose_3(n_large);
    printf("%d, %d, %d\n", prod3[0], prod3[1], prod3[2]);
    IVec2 d1 = {.ind{prod3[0]*n_small, prod3[1]*prod3[2]}};
    IVec2 d2 = {.ind{prod3[1]*n_small, prod3[0]*prod3[2]}};
    IVec2 d3 = {.ind{prod3[2]*n_small, prod3[0]*prod3[1]}};
    int d1_size = d1[0] + d1[1];
    int d2_size = d2[0] + d2[1];
    int d3_size = d3[0] + d3[1];
    if (d1_size <= d2_size && d1_size <= d3_size)
        return d1;
    else if (d2_size <= d1_size && d2_size <= d3_size)
        return d2;
    else
        return d3;
}

namespace sim_pilot {

Frames::Frames(const sim_2d::SimParams &sim_params, 
    int view_width, int view_height): 
    trajectories_tex_params {
        .format=GL_R32F,
            .width=(uint32_t)get_dimensions(
                sim_params.numberOfMCSteps,
                sim_params.numberOfOscillators
            ).ind[0],
            .height=(uint32_t)get_dimensions(
                sim_params.numberOfMCSteps,
                sim_params.numberOfOscillators
            ).ind[1],
            .wrap_s=GL_REPEAT,
            .wrap_t=GL_REPEAT,
            .min_filter=GL_NEAREST,
            .mag_filter=GL_NEAREST,
    },
    transform_tex_params {
        .format=GL_R32F,
            .width=(uint32_t)sim_params.numberOfOscillators,
            .height=(uint32_t)sim_params.numberOfOscillators,
            .wrap_s=GL_REPEAT,
            .wrap_t=GL_REPEAT,
            .min_filter=GL_NEAREST,
            .mag_filter=GL_NEAREST,
    },
    // tmp0{Quad(trajectories_tex_params)},
    // tmp1{Quad(trajectories_tex_params)},
    // tmp2{Quad(trajectories_tex_params)},
    // tmp3{Quad(trajectories_tex_params)},
    // tmp4{Quad(trajectories_tex_params)},
    position_trajectories{Quad(trajectories_tex_params)},
    trajectories {.x={
        Quad{trajectories_tex_params},
        Quad{trajectories_tex_params}},
        .next=1, .prev=0},
    discrete_sine{Quad(transform_tex_params)},
    discrete_sine_cosine{Quad(transform_tex_params)},
    trajectories_view_wire_frame {trajectories_wire_frame::get(
        sim_params.numberOfMCSteps, sim_params.numberOfOscillators,
        trajectories_wire_frame::DISCONNECTED_LINES
    )} {

}

GLSLPrograms::GLSLPrograms() {
    this->time_step 
        = Quad::make_program_from_path(
            "./shaders/trajectories/time-step.frag");
    this->trajectories_view
        = make_program_from_paths(
            "./shaders/trajectories/view.vert", 
            "./shaders/util/uniform-color.frag");
    this->norms2pos
        = Quad::make_program_from_path(
            "./shaders/trajectories/norms2pos.frag");
    this->orthogonal_transforms
        = Quad::make_program_from_path(
            "./shaders/util/orthogonal-transforms.frag");

};

Simulation::Simulation(const sim_2d::SimParams &sim_params,
        int view_width, int view_height):
        sim_2d::Simulation(sim_params, view_width, view_height),
        m_programs(), m_frames(sim_params, view_width, view_height) {
    this->make_transform_textures(sim_params.numberOfOscillators);
}

void Simulation::make_transform_textures(int number_of_oscillators) {
    int n = number_of_oscillators;
    std::vector<float> transform (n*n, 0.0);
    std::vector<float> i_transform ( n*n, 0.0);
    make_dst(transform, i_transform, n);
    m_frames.discrete_sine.set_pixels(
        transform, IVec4{.ind{0, 0, n, n}});
    make_dsct(transform, i_transform, n);
    m_frames.discrete_sine_cosine.set_pixels(
        transform, IVec4{.ind{0, 0, n, n}});
}

void Simulation::load_config_to_texture(int number_of_oscillators) {
    int w = m_frames.trajectories_tex_params.width;
    int h = m_frames.trajectories_tex_params.height;
    printf("Dimensions: %d, %d\n", w, h);
    int stack_w = w/number_of_oscillators;
    const std::vector<double> configs = get_configs();
    for (int i = 0; i < stack_w; i++) {
        int sub_size = number_of_oscillators*h;
        int offset = i*sub_size;
        std::vector<float> sub_configs (sub_size, 0.0);
        for (int k = 0; k < sub_size; k++)
            sub_configs[k] = configs[k + offset];
        int tex_x_offset = number_of_oscillators*i;
        m_frames.trajectories.x[0].set_pixels(
            sub_configs, IVec4{.ind{
                tex_x_offset, 0,
                number_of_oscillators, h}});
    }
    m_frames.trajectories.prev = 0;
    m_frames.trajectories.next = 1;
}

void Simulation::time_step(const sim_2d::SimParams &sim_params) {
    int next = m_frames.trajectories.next, prev = m_frames.trajectories.prev;
    m_frames.trajectories.x[next].draw(
        m_programs.time_step,
        {
            {"numberOfOscillators", int(sim_params.numberOfOscillators)},
            {"t", sim_params.t},
            {"dt", sim_params.dt},
            {"m", 1.0F},
            {"hbar", 1.0F},
            {"initialValuesTex",
                &sim_2d::Simulation::get_frames().initial_values},
            {"trajectoriesTexWidth",
                int(m_frames.trajectories.x[prev].width())},
            {"trajectoriesTex",
                &m_frames.trajectories.x[prev]}
        }
    );
    // m_frames.trajectories.next = prev;
    // m_frames.trajectories.prev = next;
}

void Simulation::plot_non_hist_normals(const sim_2d::SimParams &sim_params) {
    sim_2d::Frames &super_frames = sim_2d::Simulation::get_frames();
    Vec3 c = sim_params.colorOfSamples2;
    IVec2 trajectories_dimensions = IVec2{.ind{
        (int)m_frames.trajectories_tex_params.width,
        (int)m_frames.trajectories_tex_params.height}};
    super_frames.configs_view.draw(
        m_programs.trajectories_view,
        {
            {"scaleY", float(1.0F/40.0F)},
            {"color", Vec4{.r=c.r, c.g, c.b, sim_params.alphaBrightness}},
            {"yOffset", float(-0.5)},
            {"numberOfOscillators", sim_params.numberOfOscillators},
            {"trajectoryTexDimensions", trajectories_dimensions},
            {"trajectoriesTex", &m_frames.trajectories.x[1]},
            {"boundaryCond", sim_params.boundaryType.selected}
        },
        m_frames.trajectories_view_wire_frame
    );
}

void Simulation::normals2positions(const sim_2d::SimParams &sim_params) {
    IVec2 trajectories_dimensions = IVec2{.ind{
        (int)m_frames.trajectories_tex_params.width,
        (int)m_frames.trajectories_tex_params.height}};
    Quad *transform_quad;
    enum {ZERO_AT_ENDPOINTS=0, PERIODIC=1};
    if (sim_params.boundaryType.selected == ZERO_AT_ENDPOINTS)
        transform_quad = &m_frames.discrete_sine;
    else if (sim_params.boundaryType.selected == PERIODIC)
        transform_quad = &m_frames.discrete_sine_cosine;
    m_frames.position_trajectories.draw(
        m_programs.norms2pos,
        {
            {"numberOfOscillators", sim_params.numberOfOscillators},
            {"trajectoriesTexDimensions", trajectories_dimensions},
            {"trajectoriesTex", &m_frames.trajectories.x[1]},
            {"transformTex", transform_quad},
            {"useTransformTex", int(1)},
            {"boundaryType", sim_params.boundaryType.selected},
        }
    );
}

void Simulation::plot_non_hist_positions(const sim_2d::SimParams &sim_params) {
    sim_2d::Frames &super_frames = sim_2d::Simulation::get_frames();
    Vec3 c = sim_params.colorOfSamples1;
    IVec2 trajectories_dimensions = IVec2{.ind{
        (int)m_frames.trajectories_tex_params.width,
        (int)m_frames.trajectories_tex_params.height}};
    Quad *position_trajectories = &m_frames.position_trajectories;
    super_frames.configs_view.draw(
        m_programs.trajectories_view,
        {
            {"scaleY", float(1.0F/20.0F)},
            {"color", Vec4{.r=c.r, c.g, c.b, sim_params.alphaBrightness}},
            {"yOffset", float(0.5)},
            {"numberOfOscillators", sim_params.numberOfOscillators},
            {"trajectoriesTexDimensions", trajectories_dimensions},
            {"trajectoriesTex", position_trajectories},
            {"boundaryCond", sim_params.boundaryType.selected}
        },
        m_frames.trajectories_view_wire_frame
    );
}

const RenderTarget &Simulation::render_view(
    const sim_2d::SimParams &sim_params) {
    sim_2d::Frames &super_frames = sim_2d::Simulation::get_frames();
    const sim_2d::GLSLPrograms &super_programs 
        = sim_2d::Simulation::get_programs();
    super_frames.configs_view.clear();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    enum DisplayType {LINES=0, SCATTER=1, COLOR_HIST=2};
    if (sim_params.displayType.selected == DisplayType::COLOR_HIST) {
        // this->fill_plot_color_hist(sim_params);
    } else {
        if (sim_params.showNormalCoordSamples)
            this->plot_non_hist_normals(sim_params);
        this->normals2positions(sim_params);
    }
    if (sim_params.modesBrightness > 0.0)
        this->plot_exact_normals(sim_params);
    if (sim_params.displayType.selected == DisplayType::LINES ||
        sim_params.displayType.selected == DisplayType::SCATTER) {
        this->plot_non_hist_positions(sim_params);
    }
    glDisable(GL_BLEND);
    super_frames.view.draw(
        super_programs.scale,
        {
            {"tex", &super_frames.configs_view},
                {"scale", 1.0F}},
        super_frames.quad_wire_frame
    );
    #ifndef __EMSCRIPTEN__
    if (sim_params.imageRecord) {
        super_frames.image.draw(
            super_programs.copy, {{"tex", &super_frames.view}});
        auto pixels = super_frames.image.get_byte_pixels();
        std::string fname = std::to_string(sim_params.stepCount) + ".png";
        write_rgb8_png(&fname[0],
            &pixels[0], 
            super_frames.image.width(), super_frames.image.height());
    }
    #endif
    return super_frames.view;
}

void Simulation::compute_configurations(sim_2d::SimParams &sim_params) {
    sim_2d::Simulation::compute_configurations(sim_params);
    this->load_config_to_texture(sim_params.numberOfOscillators);
}

void Simulation::reset_oscillator_count(const sim_2d::SimParams &sim_params) {
    sim_2d::Simulation::reset_oscillator_count(sim_params);
    this->make_transform_textures(sim_params.numberOfOscillators);
}

void Simulation::reset_omega(const sim_2d::SimParams &sim_params) {
    sim_2d::Simulation::reset_omega(sim_params);
    this->reset_initial_values_texture(sim_params);
}

void Simulation::modify_boundaries(const sim_2d::SimParams &sim_params) {
    sim_2d::Simulation::modify_boundaries(sim_params);
    sim_2d::SimParams sim_params2 = sim_params;
    this->compute_configurations(sim_params2);
}

void Simulation::cursor_set_initial_wave_function(
    sim_2d::SimParams &sim_params, Vec2 cursor_pos) {
    sim_2d::Simulation::cursor_set_initial_wave_function(sim_params, cursor_pos);
    this->compute_configurations(sim_params);
}

void Simulation::set_relative_standard_deviation(float val) {
    sim_2d::Simulation::set_relative_standard_deviation(val);
}

void Simulation::reset_initial_values_texture(const sim_2d::SimParams &params) {
    sim_2d::Simulation::load_initial_values_texture(params);
}

// void Simulation::modify_dispersion_with_user_input(
//     const sim_2d::SimParams &sim_params, const std::string &s) {
//     sim_2d::Simulation::modify_dispersion_with_user_input(sim_params, s);
// }

}