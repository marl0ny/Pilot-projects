#include "simulation.hpp"
#include "trajectories_wire_frame.hpp"
#include "metropolis.hpp"

using namespace sim_2d;


static const std::vector<float> QUAD_VERTICES = {
    -1.0, -1.0, 0.0, -1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, -1.0, 0.0};
static const std::vector<int> QUAD_ELEMENTS = {0, 1, 2, 0, 2, 3};
static WireFrame get_quad_wire_frame() {
    return WireFrame(
        {{"position", Attribute{
            3, GL_FLOAT, false,
            0, 0}}},
        QUAD_VERTICES, QUAD_ELEMENTS,
        WireFrame::TRIANGLES
    );
}

static struct IVec2 decompose(unsigned int n) {
    struct IVec2 d = {.ind={(int)n, 1}};
    int i = 1;
    for (; i*i < n; i++) {}
    for (; n % i; i--) {}
    d.ind[0] = ((n / i) > i)? (n / i): i;
    d.ind[1] = ((n / i) < i)? (n / i): i;
    return d;
}

Programs::Programs() {
    this->copy = Quad::make_program_from_path(
        "./shaders/util/copy.frag"
    );
    this->scale = Quad::make_program_from_path(
        "./shaders/util/scale.frag"
    );
    this->blur = Quad::make_program_from_path(
        "./shaders/util/blur.frag"
    );
    this->add2 = Quad::make_program_from_path(
        "./shaders/util/add2.frag"
    );
    this->add3 = Quad::make_program_from_path(
        "./shaders/util/add3.frag"
    );
    this->domain_color = Quad::make_program_from_path(
        "./shaders/util/domain-color.frag"
    );
    this->gray_scale = Quad::make_program_from_path(
        "./shaders/util/gray-scale.frag"
    );
    this->sketch_potential = Quad::make_program_from_path(
        "./shaders/potential/sketch-potential.frag"
    );
    this->modify_potential_entry = Quad::make_program_from_path(
        "./shaders/potential/modify-potential-entry.frag"
    );
    this->time_step = Quad::make_program_from_path(
        "./shaders/wave-function/time-step.frag"
    );
    this->init_wave_packet = Quad::make_program_from_path(
        "./shaders/wave-function/gaussian.frag"
    );
    this->guide = Quad::make_program_from_path(
        "./shaders/particles/guide.frag"
    );
    this->rk4 = Quad::make_program_from_path(
        "./shaders/integration/rk4.frag"
    );
    this->forward_euler = Quad::make_program_from_path(
        "./shaders/integration/forward-euler.frag"
    );
    this->display_circles = make_program_from_paths(
        "./shaders/particles/circles-display.vert",
        "./shaders/util/uniform-color.frag"
    );
    this->user_defined = 0;
}

Frames::
Frames(const TextureParams &default_tex_params, const SimParams &params):
    wave_sim_tex_params({
        .format=GL_RG32F,
        .width=(unsigned int)params.waveDiscretizationDimensions[0],
        .height=(unsigned int)params.waveDiscretizationDimensions[1],
        .generate_mipmap=1, // default_tex_params.generate_mipmap,
        .min_filter=default_tex_params.min_filter,
        .mag_filter=default_tex_params.mag_filter,
        .wrap_s=GL_REPEAT,
        .wrap_t=GL_REPEAT
    }),
    trajectories_tex_params({
        .format=GL_RG32F,
        .width=(unsigned int)decompose(params.numberOfParticles)[0],
        .height=(unsigned int)decompose(params.numberOfParticles)[1],
        .generate_mipmap=1, // default_tex_params.generate_mipmap,
        .min_filter=default_tex_params.min_filter,
        .mag_filter=default_tex_params.mag_filter,
        .wrap_s=GL_REPEAT,
        .wrap_t=GL_REPEAT
    }),
    psi{
        Quad(wave_sim_tex_params),
        Quad(wave_sim_tex_params),
        Quad(wave_sim_tex_params)
    },
    trajectories {
        .particles{Quad(trajectories_tex_params)},
        .tmp{Quad(trajectories_tex_params)},
        .rk4{
            Quad(trajectories_tex_params),
            Quad(trajectories_tex_params),
            Quad(trajectories_tex_params),
            Quad(trajectories_tex_params),
            Quad(trajectories_tex_params)}
    },
    potential(Quad(wave_sim_tex_params)),
    tmp(Quad(wave_sim_tex_params)),
    render_intermediates{
        RenderTarget(default_tex_params),
        RenderTarget(default_tex_params),
        RenderTarget(default_tex_params)},
    particles_view(RenderTarget(default_tex_params)),
    render(default_tex_params),
    quad_wire_frame(get_quad_wire_frame()),
    trajectories_wire_frame(get_trajectories_wire_frame(
        decompose(params.numberOfParticles)
    )) {
}

void Frames::reset_wave_function_dimensions(IVec2 texel_dimensions2d) {
    this->wave_sim_tex_params.width = texel_dimensions2d[0];
    this->wave_sim_tex_params.width = texel_dimensions2d[0];
    potential.reset(this->wave_sim_tex_params);
    this->tmp.reset(this->wave_sim_tex_params);
    for (int i = 0; i < 3; i++)
        this->psi[i].reset(this->wave_sim_tex_params);

}

void Frames::reset_trajectories_dimensions(int number_of_particles) {
    IVec2 dimensions = decompose(number_of_particles);
    this->trajectories_tex_params.width = dimensions[0];
    this->trajectories_tex_params.height = dimensions[1];
    this->trajectories.particles.reset(this->trajectories_tex_params);
    for (int i = 0; i <= 4; i++)
        this->trajectories.rk4[i].reset(this->trajectories_tex_params);
    this->trajectories.tmp.reset(this->trajectories_tex_params);
    trajectories_wire_frame = get_trajectories_wire_frame(dimensions);
}

Simulation::
Simulation(const TextureParams &default_tex_params, const SimParams &params
) : m_programs(Programs()), m_frames(default_tex_params, params) {
    m_psi_ptr[0] = &m_frames.psi[0];
    m_psi_ptr[1] = &m_frames.psi[1];
    m_psi_ptr[2] = &m_frames.psi[2];
    this->new_wave_function(
        params, Vec2{.x=0.25, 0.25}, Vec2{.x=10.0, .y=10.0});
    this->new_particles(params, Vec2{.x=0.25, 0.25});
}

void Simulation::new_wave_function(
    const SimParams &params, Vec2 tex_position, Vec2 wave_num
) {
    m_time_step_count = 0;
    float sigma = params.waveFuncSize;
    for (int i = 0; i < 3; i++)
        m_psi_ptr[i]->draw(
            m_programs.init_wave_packet,
            {
                {"waveNumber", wave_num},
                {"texOffset", tex_position},
                {"amplitude", 1.0F},
                {"sigmaXY", Vec2{.x=sigma, .y=sigma}}
            }
        );
}

struct Gaussian2DParams {
    Vec2 texOffset;
    double sigma;
};

static double gaussian(const std::vector<double> &r, void *void_params) {
    Gaussian2DParams *params = (Gaussian2DParams *)void_params;
    double sigma = params->sigma;
    double x0 = (double)params->texOffset[0];
    double y0 = (double)params->texOffset[1];
    double gx = std::exp(-0.5*pow((r[0] - x0)/sigma, 2.0));
    double gy = std::exp(-0.5*pow((r[1] - y0)/sigma, 2.0));
    return gx*gy;
}

void Simulation::new_particles(
    const SimParams &params, Vec2 tex_position) {
    int old_number_of_particles 
        = m_frames.trajectories_tex_params.width
            *m_frames.trajectories_tex_params.height;
    if (old_number_of_particles != params.numberOfParticles)
        m_frames.reset_trajectories_dimensions(params.numberOfParticles);
    std::vector<double> x0 = {
    (double)tex_position.x, (double)tex_position.y};
    std::vector<double> delta = {
        1.5*params.waveFuncSize, 1.5*params.waveFuncSize};
    std::vector <double>configs 
        = std::vector<double>(params.numberOfParticles*2, 0.0);
    Gaussian2DParams gaussian_params = Gaussian2DParams {
        .texOffset=tex_position,
        .sigma=params.waveFuncSize
    };
    metropolis(
        configs, x0, delta,
        gaussian, params.numberOfParticles, 
        (void *)&gaussian_params);
    // for (int i = 0; i < params.numberOfParticles; i++)
    //     printf("%g, %g\n", configs[2*i], configs[2*i + 1]);
    std::vector<float> configs_f 
        = std::vector<float>(params.numberOfParticles*2); 
    for (int i = 0; i < params.numberOfParticles; i++) {
        float x = configs[2*i];
        float y = configs[2*i + 1];
        configs_f[2*i] = x*params.waveSimulationDimensions[0];
        configs_f[2*i + 1] = y*params.waveSimulationDimensions[1];
    }
    m_frames.trajectories.particles.set_pixels(&configs_f[0]);
    if (params.showTrails)
        m_frames.particles_view.clear();
}

const RenderTarget &Simulation
::view(SimParams &params) {
    m_frames.render_intermediates[0].draw(
        m_programs.gray_scale,
        {
            {"tex", &m_frames.potential},
            {"brightness", params.brightnessV},
            {"offset", 0.0F},
            {"maxBrightness", 1.0F}
        },
        m_frames.quad_wire_frame
    );
    m_frames.render_intermediates[1].draw(
        m_programs.domain_color,
        {
            {"tex", m_psi_ptr[1]},
            {"brightness", params.brightness},
        },
        m_frames.quad_wire_frame
    );
    if (!params.showTrails)
        m_frames.particles_view.clear();
    float particle_brightness = params.brightnessParticles;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_frames.particles_view.draw(
        m_programs.display_circles,
        {
            {"coordTex", &m_frames.trajectories.particles},
            {"circleRadius", 0.001F},
            {"dimensions2D", params.waveSimulationDimensions},
            {"color", 
                    Vec4{
                        .r=1.0,
                        .g=1.0,
                        .b=1.0,
                        .a=particle_brightness
                    }
                }
        },
        m_frames.trajectories_wire_frame
    );
    glDisable(GL_BLEND);
    m_frames.render.draw(
        m_programs.add3,
        {
            {"tex1", &m_frames.render_intermediates[0]},
            {"tex2", &m_frames.render_intermediates[1]},
            {"tex3", &m_frames.particles_view},
        }, 
        m_frames.quad_wire_frame);
    if (params.showTrails) {
        m_frames.render_intermediates[2].draw(
            m_programs.scale,
            {
                {"tex", &m_frames.particles_view},
                {"scale", 0.95F}
            },
            m_frames.quad_wire_frame
        );
        m_frames.particles_view.draw(
            m_programs.copy,
            {
                {"tex", &m_frames.render_intermediates[2]}
            },
            m_frames.quad_wire_frame
        );
    }
    return m_frames.render;
}

void Simulation::compute_guide(
    Quad &q2, const Quad *wave, const Quad &q,
        const SimParams &params) {
    bool use_nearest_sampling = (
        m_frames.wave_sim_tex_params.min_filter != GL_LINEAR ||
        m_frames.wave_sim_tex_params.mag_filter != GL_LINEAR);
    q2.draw(
        m_programs.guide,
        {
            {"hbar", params.hbar},
            {"m", params.m},
            {"psiTex", wave},
            {"qTex", &q},
            {"dimensions2D", params.waveSimulationDimensions},
            {"textureDimensions2D", params.waveDiscretizationDimensions},
            {"nearestSamplingOnly", int(use_nearest_sampling)},
            // {"potentialTex",&m_frames.potential},
            {"imposeAbsorbingBoundaries", int(params.addAbsorbingBoundaries)}
        }
    );
}

void Simulation::compute_guide(
    Quad &q2, const Quad *wave,
    const Quad &q, double dt, const Quad &q_dot,
    const SimParams &params) {
    bool use_nearest_sampling = (
        m_frames.wave_sim_tex_params.min_filter != GL_LINEAR ||
        m_frames.wave_sim_tex_params.mag_filter != GL_LINEAR);
    printf("Use nearest sampling: %d\n", use_nearest_sampling);
    q2.draw(
        m_programs.guide,
        {
            {"hbar", params.hbar},
            {"m", params.m},
            {"psiTex", wave},
            {"qTex", &q},
            {"dt", dt},
            {"qDotTex", &q_dot},
            {"nearestSamplingOnly", int(use_nearest_sampling)},
            {"dimensions2D", params.waveSimulationDimensions},
            {"textureDimensions2D", params.waveDiscretizationDimensions}
        }
    );
}

void Simulation::trajectories_time_step_rk4(const SimParams &params) {
    m_frames.trajectories.rk4[0].draw(
        m_programs.copy,
        {
            {"tex", &m_frames.trajectories.particles}
        });
    float dt = params.dt;
    // q1
    this->compute_guide(
        m_frames.trajectories.rk4[1],
        m_psi_ptr[0],
        m_frames.trajectories.particles, 
        params);
    // q2
    this->compute_guide(
        m_frames.trajectories.rk4[2], 
        m_psi_ptr[1],
        m_frames.trajectories.particles, dt/2.0, m_frames.trajectories.rk4[1],
        params);
    // q3
    this->compute_guide(
        m_frames.trajectories.rk4[3], 
        m_psi_ptr[1],
        m_frames.trajectories.particles, dt/2.0, m_frames.trajectories.rk4[2],
        params);
    // q4
    this->compute_guide(
        m_frames.trajectories.rk4[4], 
        m_psi_ptr[2],
        m_frames.trajectories.particles, dt, m_frames.trajectories.rk4[3],
        params);
    m_frames.trajectories.particles.draw(
        m_programs.rk4,
        {
            {"qTex", &m_frames.trajectories.rk4[0]},
            {"qDotTex1", &m_frames.trajectories.rk4[1]},
            {"qDotTex2", &m_frames.trajectories.rk4[2]},
            {"qDotTex3", &m_frames.trajectories.rk4[3]},
            {"qDotTex4", &m_frames.trajectories.rk4[4]},
            {"dt", params.dt},
            {"periodicizeResult", int(true)},
            {"minBoundaryVal", Vec4{.ind={0.0}}},
            {"domainDimensions", Vec4{.ind{
                   params.waveSimulationDimensions[0],
                   params.waveSimulationDimensions[1],
                   params.waveSimulationDimensions[0],
                   params.waveSimulationDimensions[1]}}},
        }
    );
}

void Simulation::time_step(const SimParams &params) {
    if (m_time_step_count == 0) {
        // Initial forward Euler step.
        m_psi_ptr[1]->draw(
            m_programs.time_step,
            {
                {"m", params.m},
                {"hbar", params.hbar},
                {"dt", float(params.dt/2.0)},
                {"psi0Tex", m_psi_ptr[0]},
                {"psi1Tex", m_psi_ptr[0]},
                {"potentialTex", &m_frames.potential},
                {"dimensions2D", params.waveSimulationDimensions},
                {"textureDimensions2D", params.waveDiscretizationDimensions}
            }
        );
        m_time_step_count++;
    }
    m_psi_ptr[2]->draw(
        m_programs.time_step,
        {
            {"m", params.m},
            {"hbar", params.hbar},
            {"dt", params.dt},
            {"psi0Tex", m_psi_ptr[0]},
            {"psi1Tex", m_psi_ptr[1]},
            {"potentialTex", &m_frames.potential},
            {"dimensions2D", params.waveSimulationDimensions},
            {"textureDimensions2D", params.waveDiscretizationDimensions}
        }
    );
    if (m_time_step_count % 2 && m_time_step_count != 0) {
        trajectories_time_step_rk4(params);
    }
    Quad *psi_ptr[3] = {m_psi_ptr[1], m_psi_ptr[2], m_psi_ptr[0]};
    for (int i = 0; i < 3; i++)
        m_psi_ptr[i] = psi_ptr[i];
    m_time_step_count++;
}

void Simulation::add_user_defined_potential(
    const SimParams &sim_params,
    unsigned int program, const std::map<std::string, float> &input_uniforms) {
    this->m_programs.user_defined = program;
    Uniforms uniforms;
    for (const auto &e: input_uniforms)
        uniforms.insert({e.first, Vec2{.ind{e.second, 0.0}}});
    if (input_uniforms.count("t") > 0)
        uniforms.at("t").vec2.x = sim_params.t;
    else
        uniforms.insert({"t", Vec2{.ind{sim_params.t, 0.0}}});
    uniforms.insert(
        {"width",
            Vec2{.ind{sim_params.waveSimulationDimensions[0], 0.0}}});
    uniforms.insert(
        {"height",
            Vec2{.ind{sim_params.waveSimulationDimensions[1], 0.0}}});
    enum outputModeSelect {
        MODE_4VECTOR_REAL_OR_COMPLEX=0,
        MODE_COMPLEX4=4
    };
    uniforms.insert(
        {"outputModeSelect",int(MODE_COMPLEX4)}
    );
    uniforms.insert(
        {"useRealPartOfExpression", int(1)});
    m_frames.tmp.draw(program, uniforms);
    m_frames.potential.draw(
        m_programs.modify_potential_entry,
        {
            {"minVal", Vec2{.x=-1.0, .y=-1.0}},
            {"maxVal", Vec2{.x=1.0, .y=1.0}},
            {"addAbsorbingBoundaries", 
                    int(sim_params.addAbsorbingBoundaries)},
            {"rawPotentialTex", &m_frames.tmp}
        }
    );
}

void Simulation::sketch_potential(
    const SimParams &params,
    const Vec2 &position, float amplitude
) {
    m_frames.tmp.draw(
        m_programs.sketch_potential,
        {
            {"tex", &m_frames.potential},
            {"offsetTexCoord", position},
            {"sigmaTexCoord", Vec2{.x=0.02, .y=0.02}},
            {"amplitude", amplitude},
            {"maxScalarValue", 1.0F},
        }
    );
    m_frames.potential.draw(
        m_programs.copy,
        {{"tex", &m_frames.tmp}}
    );
}

void Simulation::set_potential_from_image(
    const SimParams &params,
    const uint8_t *image_data,
    IVec2 image_dimensions) {
    int w = image_dimensions[0];
    int h = image_dimensions[1];
    std::vector<Vec2> potential_tmp (
        params.waveDiscretizationDimensions[0]
        *params.waveDiscretizationDimensions[1],
        {.x=0.0, .y=0.0}
    );
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            int pix_ind = i*w + j;
            double r = (double)image_data[4*pix_ind]/255.0;
            double g = (double)image_data[4*pix_ind + 1]/255.0;
            double b = (double)image_data[4*pix_ind + 2]/255.0;
            double mono_val = sqrt(r*r + g*g + b*b);
            if (i < params.waveDiscretizationDimensions[0] 
                && j < params.waveDiscretizationDimensions[1])
                potential_tmp[
                    params.waveDiscretizationDimensions[0]
                    *(params.waveDiscretizationDimensions[0] - i - 1)
                     + j] = {.x=0.5F*(float)mono_val, .y=0.0};
        }
    }
    this->m_frames.tmp.set_pixels((float *)&potential_tmp[0]);
    m_frames.potential.draw(
        m_programs.modify_potential_entry,
        {
            {"minVal", Vec2{.x=-1.0, .y=-1.0}},
            {"maxVal", Vec2{.x=1.0, .y=1.0}},
            {"addAbsorbingBoundaries", 
                    int(params.addAbsorbingBoundaries)},
            {"rawPotentialTex", &m_frames.tmp}
        }
    );
}

