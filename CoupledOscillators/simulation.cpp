#include "simulation.hpp"
#include "harmonic.hpp"
#include "multidimensional_harmonic.hpp"
#include "configs_view.hpp"
#include "metropolis.hpp"
#include "histogram.hpp"
#include "parse.hpp"
#include "write_to_png.hpp"
#include "orthogonal_transforms.hpp"
#include <complex>
#include <vector>

#ifndef __EMSCRIPTEN__
#define THREAD_COUNT 8
#include <pthread.h>
#endif

using namespace::sim_2d;

using std::complex;
using std::vector;

typedef std::vector<double> Arr1D;
typedef std::vector<complex<double>> ArrC1D;
typedef std::vector<int> ArrI1D;

#define PI 3.141592653589793

#define FILTER_TYPE GL_NEAREST


static void c_sq_matrix_mul(
    double *dst, const double *m, const double *v, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = 0.0;
        for (int j = 0; j < n; j++) {
            dst[i] += m[i*n + j]*v[j];
        }
    }
}

static void c_copy(double *dst, const double *v, int n) {
    for (int i = 0; i < n; i++)
        dst[i] = v[i];
}

static WireFrame get_quad_wire_frame() {
    return WireFrame(
        {{"position", Attribute{
            3, GL_FLOAT, false,
            0, 0}}},
        {-1.0, -1.0, 0.0, -1.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0, -1.0, 0.0},
        {0, 1, 2, 0, 2, 3},
        WireFrame::TRIANGLES
    );
}

static void frequency_index_and_its_max(
    int &frequency_index, int &max_frequency, 
    int i, int n, int boundary_type) {
    enum {ZERO_ENDPOINTS=0, PERIODIC=1};
    if (boundary_type == PERIODIC) {
        frequency_index = (n % 2)? (i - n/2): (i - n/2 + 1);
        max_frequency = n/2;
    } else {
        frequency_index = i + 1;
        max_frequency = n + 1;
    }
}

static double get_omega(int i, int n, int boundary_type, int preset) {
    enum {ZERO_ENDPOINTS=0, PERIODIC=1};
    int k, size; 
    frequency_index_and_its_max(k, size, i, n, boundary_type);
    if (preset == 0)
        return 2.0*sin(0.5*PI*abs(k)/size);
    else
        return PI*abs(k)/size;
    // return sqrt(pow(PI*(i + 1)/(n + 1), 2.0) + 0.81);
    // return PI*(i + 1)/(n + 1);
    // return 2.0*sin(0.5*PI*(i + 1)/(n + 1));
}

Frames::Frames(const SimParams &sim_params, 
    int view_width, int view_height):
    view_tex_params(
        {
            .format=GL_RGBA32F,
            .width=(uint32_t)view_width,
            .height=(uint32_t)view_height,
            .wrap_s=GL_CLAMP_TO_EDGE,
            .wrap_t=GL_CLAMP_TO_EDGE,
            .min_filter=FILTER_TYPE,
            .mag_filter=FILTER_TYPE,
        }
    ),
    image_tex_params(
        {
            .format=GL_RGB8,
            .width=(uint32_t)view_width,
            .height=(uint32_t)view_height,
            .wrap_s=GL_CLAMP_TO_EDGE,
            .wrap_t=GL_CLAMP_TO_EDGE,
            .min_filter=FILTER_TYPE,
            .mag_filter=FILTER_TYPE,
        }
    ),
    configs_view_tex_params(
        {
            .format=GL_RGBA32F,
            .width=(uint32_t)view_width,
            .height=(uint32_t)view_height,
            // .width=(uint32_t)sim_params.numberOfOscillators,
            // .height=(uint32_t)sim_params.numberOfOscillators,
            .wrap_s=GL_REPEAT,
            .wrap_t=GL_REPEAT,
            .min_filter=FILTER_TYPE,
            .mag_filter=FILTER_TYPE,
        }
    ),
    hist_tex_params(
        {
            .format=GL_R32F,
            .width=(uint32_t)sim_params.numberOfOscillators,
            .height=(uint32_t)view_height/2,
            .wrap_s=GL_REPEAT,
            .wrap_t=GL_REPEAT,
            .min_filter=FILTER_TYPE,
            .mag_filter=FILTER_TYPE,
        }
    ),
    initial_values_tex_params(
        {
            .format=GL_RGBA32F,
            .width=(uint32_t)sim_params.numberOfOscillators,
            .height=(uint32_t)1,
            .wrap_s=GL_REPEAT,
            .wrap_t=GL_REPEAT,
            .min_filter=GL_NEAREST,
            .mag_filter=GL_NEAREST,
        }
    ),
    tmp(configs_view_tex_params),
    initial_values(initial_values_tex_params),
    hist(hist_tex_params),
    image(image_tex_params),
    configs_view(configs_view_tex_params),
    view(view_tex_params),
    quad_wire_frame(get_quad_wire_frame())
    {
}

GLSLPrograms::GLSLPrograms() {
    this->copy = Quad::make_program_from_path("./shaders/util/copy.frag");
    this->scale = Quad::make_program_from_path("./shaders/util/scale.frag");
    this->height_map = Quad::make_program_from_path(
        "./shaders/util/height-map.frag");
    this->modes = Quad::make_program_from_path("./shaders/modes.frag");
    this->configs_view = make_program_from_paths(
        "./shaders/configs-view.vert", "./shaders/util/uniform-color.frag");
};

Simulation::Simulation(
    const SimParams &sim_params,
    int view_width, int view_height):
    m_programs(),
    m_frames(sim_params, view_width, view_height),
    m_initial_wave_func(sim_params.numberOfOscillators) {
    int n = sim_params.numberOfOscillators;
    m_configs = Arr1D(
        sim_params.numberOfMCSteps*n);
    m_configs.reserve(100000*MAX_SIZE/2);
    m_positions2normals = Arr1D(n*n);
    // m_positions2normals.reserve(MAX_SIZE*MAX_SIZE);
    m_normals2positions = Arr1D(n*n);
    // m_omega = Arr1D(MAX_SIZE, 0.0);
    // m_normals2positions.reserve(MAX_SIZE*MAX_SIZE);
    this->reset_coord_transform(sim_params);
    this->reset_omega(sim_params);
    Arr1D tmp (n);
    m_hist = {
        .dimensions=IVec2{.ind{n, (int)m_frames.hist_tex_params.height}},
        .min_val={.x=0.0, -20.0}, .range={.x=float(n), .y=40.0},
        .arr=std::vector<float>(n*m_frames.hist_tex_params.height)
    };
    for (int i = 0; i < n; i++) {
        m_initial_wave_func.s[i] = 1.0;
        m_initial_wave_func.coefficients[i]
            = 2.0*sin(PI*(i + 1)*(int(n/2))/(n + 1))/sqrt(2.0*(n + 1));
        tmp[i] = 10.0*exp(-0.5*pow((double(i) - n/2.0)/(n*0.05), 2.0));
        // tmp[i] = 10.0*sin(4.0*PI*(i + 1)/(n + 1));
    }
    c_sq_matrix_mul(
        &m_initial_wave_func.x[0], &m_positions2normals[0], &tmp[0], n);
}

void
Simulation::load_initial_values_texture(const SimParams &sim_params) {
    std::vector<float> initial_values_pixels {};
    int n = sim_params.numberOfOscillators;
    for (int i = 0; i < n; i++) {
        double sigma = coherent_standard_dev(1.0, m_omega[i], 1.0);
        if (sim_params.useStationary)
            initial_values_pixels.push_back(
                m_initial_wave_func.excitations[i]);
        else if (sim_params.useCoherentStates || sim_params.useSqueezed)
            initial_values_pixels.push_back(m_initial_wave_func.x[i]);
        initial_values_pixels.push_back(m_initial_wave_func.p[i]);
        initial_values_pixels.push_back(m_omega[i]);
        if (sim_params.useCoherentStates)
            initial_values_pixels.push_back(sigma);
        else if (sim_params.useSqueezed)
            initial_values_pixels.push_back(sigma*m_initial_wave_func.s[i]);
    }
    m_frames.initial_values.set_pixels(initial_values_pixels);
}

void
Simulation::compute_stationary_state_configurations(SimParams &sim_params) {
    int n_count = sim_params.numberOfOscillators;
    StationaryStatesProdData data = {
        .t=sim_params.t, .m=1.0, .hbar=1.0,
        .excitations=ArrI1D(n_count),
        .omega=Arr1D(n_count)
    };
    auto delta = Arr1D(n_count);
    auto x = Arr1D(n_count);
    for (int i = 0; i < n_count; i++) {
        data.excitations[i] = m_initial_wave_func.excitations[i];
        double omega = m_omega[i];
        data.omega[i] = omega;
        delta[i] = (1.0 + data.excitations[i])
            *sim_params.relativeDelta*coherent_standard_dev(1.0, omega, 1.0);
    }
    std::vector<float> initial_values_pixels {};
    for (int i = 0; i < n_count; i++) {
        initial_values_pixels.push_back(float(data.excitations[i]));
        initial_values_pixels.push_back(0.0);
        initial_values_pixels.push_back(data.omega[i]);
        initial_values_pixels.push_back(0.0);   
    }
    m_frames.initial_values.set_pixels(initial_values_pixels);
    auto info = metropolis(
        m_configs, x, delta, 
        stationary_states_prod_dist_func,
        sim_params.numberOfMCSteps, (void *)&data);
    sim_params.acceptanceRate
            = float(info.accepted_count)
            / float(info.accepted_count + info.rejection_count);
}

void 
Simulation::compute_coherent_state_configurations(SimParams &sim_params) {
    int n = sim_params.numberOfOscillators;
    CoherentStateProdData data = {
        .t=sim_params.t, .hbar=1.0, .m=1.0,
        .x0=Arr1D(n), .p0=Arr1D(n), .omega=Arr1D(n)
    };
    auto delta = Arr1D(n);
    auto x = Arr1D(n);
    std::vector<float> initial_values_pixels {};
    for (int i = 0; i < n; i++) {
        data.x0[i] = m_initial_wave_func.x[i];
        data.p0[i] = m_initial_wave_func.p[i];
        double omega = m_omega[i];
        data.omega[i] = omega;
        double sigma = coherent_standard_dev(1.0, omega, 1.0);
        delta[i] = sim_params.relativeDelta*sigma;
        x[i] = squeezed_avg_x(
            data.t, data.x0[i], data.p0[i], 1.0, data.omega[i], 1.0);
        initial_values_pixels.push_back(data.x0[i]);
        initial_values_pixels.push_back(data.p0[i]);
        initial_values_pixels.push_back(data.omega[i]);
        initial_values_pixels.push_back(sigma);
    }
    m_frames.initial_values.set_pixels(initial_values_pixels);
    auto info = metropolis(
        m_configs, x, delta, 
        coherent_state_prod_dist_func,
        sim_params.numberOfMCSteps, (void *)&data);
    sim_params.acceptanceRate
            = float(info.accepted_count)
            / float(info.accepted_count + info.rejection_count);
}

void 
Simulation::compute_squeezed_state_configurations(SimParams &sim_params) {
    int n = sim_params.numberOfOscillators;
    SqueezedStateProdData data = {
        .t=sim_params.t, .m=1.0, .hbar=1.0, 
        .x0=Arr1D(n), .p0=Arr1D(n), .sigma0=Arr1D(n), .omega=Arr1D(n)
    };
    auto delta = Arr1D(n);
    auto x = Arr1D(n);
    std::vector<float> initial_values_pixels {};
    for (int i = 0; i < n; i++) {
        data.x0[i] = m_initial_wave_func.x[i];
        data.p0[i] = m_initial_wave_func.p[i];
        data.omega[i] = m_omega[i];
        double sigma = coherent_standard_dev(1.0, data.omega[i], 1.0);
        data.sigma0[i] = m_initial_wave_func.s[i]*sigma;
        delta[i] = sim_params.relativeDelta*
            squeezed_standard_dev(
                data.t, data.sigma0[i], 
                data.m, data.omega[i], data.hbar);
        x[i] = squeezed_avg_x(
            data.t, data.x0[i], data.p0[i], 1.0, data.omega[i], 1.0);
        initial_values_pixels.push_back(data.x0[i]);
        initial_values_pixels.push_back(data.p0[i]);
        initial_values_pixels.push_back(data.omega[i]);
        initial_values_pixels.push_back(data.sigma0[i]);
    }
    m_frames.initial_values.set_pixels(initial_values_pixels);
    auto info = metropolis(
        m_configs, x, delta, 
        squeezed_state_prod_dist_func,
        sim_params.numberOfMCSteps, (void *)&data);
    sim_params.acceptanceRate
            = float(info.accepted_count)
            / float(info.accepted_count + info.rejection_count);
}

void Simulation::
compute_single_excitations_configurations(SimParams &sim_params) {
    int n = sim_params.numberOfOscillators;
    auto delta = Arr1D(n);
    auto x = Arr1D(n);
    SingleExcitationsStateData data {
        .t=sim_params.t, .m=1.0, .hbar=1.0,
        .omega=Arr1D(n),
        .coeff=ArrC1D(n), 
    };
    std::vector<float> initial_values_pixels {};
    for (int i = 0; i < n; i++) {
        double omega = m_omega[i];
        double sigma = coherent_standard_dev(1.0, omega, 1.0);
        data.omega[i] = omega;
        data.coeff[i] = m_initial_wave_func.coefficients[i];
        delta[i] = sim_params.relativeDelta*sigma;
    }
    m_frames.initial_values.set_pixels(initial_values_pixels);
    auto info = metropolis(
        m_configs, x, delta, 
        single_excitations_sum_dist_func,
        sim_params.numberOfMCSteps, (void *)&data);
    sim_params.acceptanceRate
            = float(info.accepted_count)
            / float(info.accepted_count + info.rejection_count);
}

void Simulation::compute_configurations(SimParams &sim_params) {
    if (sim_params.useCoherentStates)
        this->compute_coherent_state_configurations(sim_params);
    else if (sim_params.useSqueezed)
        this->compute_squeezed_state_configurations(sim_params);
    else if (sim_params.useStationary)
        this->compute_stationary_state_configurations(sim_params);
    else if (sim_params.useSingleExcitations)
        this->compute_single_excitations_configurations(sim_params);
    else
        this->compute_coherent_state_configurations(sim_params);

}

static int get_wave_func_type(const SimParams &sim_params) {
    enum {ALL_COHERENT=0, ALL_SQUEEZED, ENERGY_EIGENSTATE, 
        SINGLE_EXCITATIONS};
    if (sim_params.useCoherentStates)
        return ALL_COHERENT;
    else if (sim_params.useSqueezed)
        return ALL_SQUEEZED;
    else if (sim_params.useStationary)
        return ENERGY_EIGENSTATE;
    else if (sim_params.useSingleExcitations)
        return SINGLE_EXCITATIONS;
    return -1;
}

void Simulation::fill_plot_color_hist(const SimParams &sim_params) {
    double hist_amp 
        = sim_params.alphaBrightness*
            (25000.0/sim_params.numberOfMCSteps);
    for (int i = 0; i < m_hist.arr.size(); i++)
        m_hist.arr[i] = 0.0;
    for (int k = 0; k < sim_params.numberOfMCSteps; k++) {
        int n = sim_params.numberOfOscillators;
        m_hist.min_val.y = -40.0;
        m_hist.range.y = 80.0;
        if (sim_params.showNormalCoordSamples)
            for (int j = 0; j < n; j++)
                histogram::add_data_point(
                    m_hist, 
                    double(j), m_configs[n*k + j] - 20.0,
                    hist_amp);
        m_hist.min_val.y = -20.0;
        m_hist.range.y = 40.0;
        Arr1D res = Arr1D(n);
        c_sq_matrix_mul(
            &res[0], &m_normals2positions[0], 
            &m_configs[k*n], n);
        c_copy(&m_configs[k*n], &res[0], n);
        for (int j = 0; j < n; j++)
            histogram::add_data_point(
                m_hist, 
                double(j), m_configs[n*k + j] + 10.0,
                hist_amp);
    }
    m_frames.hist.set_pixels(&m_hist.arr[0]);
    m_frames.configs_view.draw(
        m_programs.height_map, 
        {{"tex", &m_frames.hist}}, 
        m_frames.quad_wire_frame
    );
}

void Simulation::plot_non_hist_normals(const SimParams &sim_params) {
    WireFrame wire_frame = configs_view::get_configs_view_wire_frame(
        m_configs, 
        sim_params.numberOfMCSteps, 
        (sim_params.displayType.selected == 0)?
            configs_view::LINES_NO_ENDPOINTS:
            configs_view::DISCONNECTED_LINES);
    Vec3 c = sim_params.colorOfSamples2;
    m_frames.configs_view.draw(
        m_programs.configs_view,
        {
            {"scaleY", float(1.0F/40.0F)},
            {"color", 
                    Vec4{.r=c.r, c.g, c.b, 
                        sim_params.alphaBrightness}},
            {"yOffset", float(-0.5)},
        },
        wire_frame
    );
}

void Simulation::plot_exact_normals(const SimParams &sim_params) {
    m_frames.configs_view.draw(
        m_programs.modes,
        {
            {"t", sim_params.t},
            {"m", 1.0F},
            {"omega", 1.0F},
            {"hbar", 1.0F},
            {"scale", 40.0F},
            {"offset", float(-0.25F)},
            {"initialValuesTex", &m_frames.initial_values},
            {"brightness", sim_params.modesBrightness},
            {"colorPhase", int(sim_params.colorPhase)},
            {"waveFunctionType", get_wave_func_type(sim_params)}
        },
        m_frames.quad_wire_frame
    );
}

void Simulation::plot_non_hist_positions(const SimParams &sim_params) {
    int continuous_line_type;
    enum {ZERO_ENDPOINTS=0, PERIODIC=1};
    if (sim_params.boundaryType.selected == ZERO_ENDPOINTS)
        continuous_line_type = configs_view::LINES_WITH_ZERO_ENDPOINTS;
    else if (sim_params.boundaryType.selected == PERIODIC)
        continuous_line_type= configs_view::LINES_PERIODIC;
    WireFrame wire_frame = configs_view::get_configs_view_wire_frame(
        m_configs, sim_params.numberOfMCSteps,
        (sim_params.displayType.selected == 0)?
            continuous_line_type:
            configs_view::DISCONNECTED_LINES);
    Vec3 c = sim_params.colorOfSamples1;
    m_frames.configs_view.draw(
        m_programs.configs_view,
        {
            {"scaleY", float(1.0F/20.0F)},
            {"color", Vec4{.r=c.r, c.g, c.b, sim_params.alphaBrightness}},
            {"yOffset", float(0.5)},
        },
        wire_frame
    );
}

#ifdef THREAD_COUNT

struct Normals2PositionsThreadData {
    double *configs;
    const double *transform_mat;
    int num_oscillators;
    int count;
};

static void *normals2positions_mt(void *void_data) {
    struct Normals2PositionsThreadData *data 
        = (Normals2PositionsThreadData *)void_data;
    double *configs = data->configs;
    const double *transform_mat = data->transform_mat;
    int count = data->count;
    int num_oscillators = data->num_oscillators;
    double res[512] = {0.0,};
    for (int k = 0; k < count; k++) {
        c_sq_matrix_mul(
            res, transform_mat, 
            &configs[k*num_oscillators], num_oscillators);
        c_copy(&configs[k*num_oscillators], res, num_oscillators);
    }
    return NULL;
}

pthread_t s_threads[THREAD_COUNT] = {};
Normals2PositionsThreadData s_thread_data[THREAD_COUNT] = {};

#endif

void Simulation::normals2positions(const SimParams &sim_params) {
    #ifndef THREAD_COUNT
    for (int k = 0; k < sim_params.numberOfMCSteps; k++) {
        int n = sim_params.numberOfOscillators;
        Arr1D res = Arr1D(n);
        c_sq_matrix_mul(
            &res[0], &m_normals2positions[0], 
            &m_configs[k*n], n);
        c_copy(&m_configs[k*n], &res[0], n);
    }
    #else
    int num_oscillators = sim_params.numberOfOscillators;
    int ac_thread_count = THREAD_COUNT;
    if ((m_configs.size()/num_oscillators) < 100)
        ac_thread_count = 2;
    int count_per_thread = (m_configs.size()/num_oscillators)/ac_thread_count;
    for (int i = 0; i < ac_thread_count; i++) {
        s_thread_data[i].configs
            = &((double *)&m_configs[0])[
                i*(count_per_thread*num_oscillators)];
        s_thread_data[i].count = (i == ac_thread_count - 1)?
            (m_configs.size()/num_oscillators
             - (ac_thread_count - 1)*(count_per_thread)):
            count_per_thread;
        s_thread_data[i].num_oscillators = sim_params.numberOfOscillators;
        s_thread_data[i].transform_mat = &m_normals2positions[0];
        pthread_create(
            &s_threads[i], NULL, normals2positions_mt,
            (void *)&s_thread_data[i]
        );
    }
    for (int i = 0; i < ac_thread_count; i++)
        pthread_join(s_threads[i], NULL);
    #endif
}

const RenderTarget &Simulation::render_view(
    const SimParams &sim_params) {
    m_frames.configs_view.clear();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    enum DisplayType {LINES=0, SCATTER=1, COLOR_HIST=2};
    if (sim_params.displayType.selected == DisplayType::COLOR_HIST) {
        this->fill_plot_color_hist(sim_params);
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
    m_frames.view.draw(
        m_programs.scale,
        {
            {"tex", &m_frames.configs_view},
                {"scale", 1.0F}},
        m_frames.quad_wire_frame
    );
    #ifndef __EMSCRIPTEN__
    if (sim_params.imageRecord) {
        m_frames.image.draw(
            m_programs.copy, {{"tex", &m_frames.view}});
        auto pixels = m_frames.image.get_byte_pixels();
        std::string fname = std::to_string(sim_params.stepCount) + ".png";
        write_rgb8_png(&fname[0],
            &pixels[0], m_frames.image.width(), m_frames.image.height());
    }
    #endif
    return m_frames.view;
}

void Simulation::reset_oscillator_count(const SimParams &params) {
    int n = params.numberOfOscillators;
    m_frames.initial_values_tex_params = {
        .format=GL_RGBA32F,
        .width=(uint32_t)n,
        .height=(uint32_t)1,
        .wrap_s=GL_REPEAT,
        .wrap_t=GL_REPEAT,
        .min_filter=GL_NEAREST,
        .mag_filter=GL_NEAREST,
    };
    m_frames.initial_values.reset(m_frames.initial_values_tex_params);
    m_normals2positions = Arr1D(n*n);
    m_positions2normals = Arr1D(n*n);
    m_initial_wave_func.resize(n);
    this->reset_coord_transform(params);
    this->reset_omega(params);
    int view_height = m_frames.configs_view.texture_dimensions()[1];
    m_frames.hist_tex_params = {
        .format=GL_R32F,
        .width=(uint32_t)n,
        .height=(uint32_t)view_height/2,
        .wrap_s=GL_REPEAT,
        .wrap_t=GL_REPEAT,
        .min_filter=FILTER_TYPE,
        .mag_filter=FILTER_TYPE,
    };
    m_frames.hist.reset(m_frames.hist_tex_params);
    m_hist = {
        .dimensions=IVec2{.ind{n, (int)m_frames.hist_tex_params.height}},
        .min_val={.x=0.0, -20.0}, .range={.x=float(n), .y=40.0},
        .arr=std::vector<float>(n*m_frames.hist_tex_params.height)
    };
}

void Simulation::reset_omega(const SimParams &sim_params) {
    for (int i = 0; i < sim_params.numberOfOscillators; i++)
        m_omega[i] = get_omega(
            i, sim_params.numberOfOscillators, 
            sim_params.boundaryType.selected, 
            sim_params.presetDispersionRelation.selected);
}

void Simulation::reset_coord_transform(const SimParams &sim_params) {
    enum {ZERO_ENDPOINTS=0, PERIODIC=1};
    if (sim_params.boundaryType.selected == ZERO_ENDPOINTS) {
        make_dst(
            m_positions2normals, m_normals2positions,
            sim_params.numberOfOscillators);
    } else if (sim_params.boundaryType.selected == PERIODIC) {
        make_dsct(
            m_positions2normals, m_normals2positions,
            sim_params.numberOfOscillators);
    }
}

void Simulation::modify_boundaries(const SimParams &sim_params) {
    int n = sim_params.numberOfOscillators;
    std::vector<double> x0(n), p0(n);
    if (sim_params.useCoherentStates || sim_params.useSqueezed) {
        for (int i = 0; i < n; i++) {
            x0[i] = m_initial_wave_func.x[i];
            p0[i] = m_initial_wave_func.p[i];
            m_initial_wave_func.x[i] = squeezed_avg_x(
                sim_params.t, x0[i], p0[i],
                1.0, m_omega[i], 1.0);
            m_initial_wave_func.p[i] = squeezed_avg_p(
                sim_params.t, x0[i], p0[i],
                1.0, m_omega[i], 1.0);
        }
        c_sq_matrix_mul(
            &x0[0], 
            &m_normals2positions[0], &m_initial_wave_func.x[0], n);
        c_sq_matrix_mul(
            &p0[0], 
            &m_normals2positions[0], &m_initial_wave_func.p[0], n);
        m_initial_wave_func.set_s_to_ones();
    } else if (sim_params.useSingleExcitations) {
        m_initial_wave_func.zero_coefficients();
    } else if (sim_params.useStationary) {
        m_initial_wave_func.zero_excitations();
    }
    this->reset_omega(sim_params);
    this->reset_coord_transform(sim_params);
    if (sim_params.useCoherentStates || sim_params.useSqueezed) {
        c_sq_matrix_mul(
            &m_initial_wave_func.x[0], 
            &m_positions2normals[0], &x0[0], n);
        c_sq_matrix_mul(
            &m_initial_wave_func.p[0], 
            &m_positions2normals[0], &p0[0], n);
    }
}

void Simulation::cursor_set_initial_wave_function(
    SimParams &sim_params, Vec2 cursor_pos) {
    enum {REPLACE_AMPLITUDE=0, ADD_AMPLITUDE=1};
    float t = sim_params.t;
    sim_params.t = 0.0;
    printf("%g, %g\n", cursor_pos.x, cursor_pos.y);
    int oscillator_pos = int(cursor_pos.x * sim_params.numberOfOscillators);
    if (cursor_pos.y < 0.5) {
        for (int i = 0; i < sim_params.numberOfOscillators; i++) {
            if (sim_params.useStationary) {
                if (i == oscillator_pos) {
                    if (sim_params.addEnergy) {
                        if (m_initial_wave_func.excitations[i] < 30)
                            m_initial_wave_func.excitations[i]++;
                    } else if (m_initial_wave_func.excitations[i] > 0) {
                        m_initial_wave_func.excitations[i]--;
                    }
                }
            } else if (sim_params.useCoherentStates ||
                         sim_params.useSqueezed) {
                if (sim_params.clickActionNormal.selected 
                    == ADD_AMPLITUDE) {
                    double omega = m_omega[i];
                    m_initial_wave_func.x[i] = squeezed_avg_x(
                        t, m_initial_wave_func.x[i], m_initial_wave_func.p[i],
                        1.0, omega, 1.0);
                    // m_initial_wave_func.p[i] = squeezed_avg_p(
                    //     t, m_initial_wave_func.x[i], m_initial_wave_func.p[i],
                    //     1.0, omega, 1.0);
                }
                if (i == oscillator_pos) {
                    m_initial_wave_func.x[i] = 80.0*(cursor_pos.y - 0.25);
                    if (sim_params.useSqueezed)
                        m_initial_wave_func.s[i] 
                            = 1.0/sim_params.squeezedFactor;
                } else {
                    if (sim_params.clickActionNormal.selected 
                        == REPLACE_AMPLITUDE) {
                        m_initial_wave_func.x[i] = 0.0;
                        m_initial_wave_func.p[i] = 0.0;
                    }
                }
            } else if (sim_params.useSingleExcitations) {
                m_initial_wave_func.coefficients[i] =
                    (i == oscillator_pos)? 1.0: 0.0;
            }
        }
    } else {
        if (sim_params.useCoherentStates || sim_params.useSqueezed) {
            Arr1D tmp(sim_params.numberOfOscillators);
            for (int i = 0; i < sim_params.numberOfOscillators; i++) {
                int n = sim_params.numberOfOscillators;
                m_initial_wave_func.p[i] = 0.0;
                tmp[i] = 40.0*(cursor_pos.y - 0.75)*
                    exp(-0.5*pow((double(i) - oscillator_pos)/(n*0.05), 2.0));
            }
            c_sq_matrix_mul(
                &(m_initial_wave_func.x[0]), &m_positions2normals[0], &tmp[0],
                sim_params.numberOfOscillators);
        } else if (sim_params.useSingleExcitations) {
            int n = sim_params.numberOfOscillators;
            int oscillator_pos = int(cursor_pos.x * n);
            for (int i = 0; i < sim_params.numberOfOscillators; i++)
                m_initial_wave_func.coefficients[i]
                    = 2.0*sin(
                        PI*(i + 1)*(double(oscillator_pos) + 1.0)/(n + 1)
                    )/sqrt(2.0*(n + 1));


        }
    }

}

void Simulation::set_relative_standard_deviation(float val) {
    for (int i = 0; i < 512; i++)
        m_initial_wave_func.s[i] = val;
    // printf("%g\n", m_initial_wave_func.s[0]);
}

const GLSLPrograms &Simulation::get_programs() {
    return m_programs;
}

Frames &Simulation::get_frames() {
    return m_frames;
}

const std::vector<double> &Simulation::get_configs() {
    return m_configs;
}

// #include <iostream>

// void Simulation::modify_dispersion_with_user_input(
//     const SimParams &sim_params, const std::string &s) {
//     std::vector<std::string> expr_stack, rpn_list;
//     try {
//         expr_stack = get_expression_stack(s);
//         rpn_list = shunting_yard(expr_stack);
//     } catch(...) {
//         return;
//     }
//     Arr1D omega(sim_params.numberOfOscillators);
//     for (auto &e: rpn_list)
//         std::cout << e << std::endl;
//     for (int i = 0; i < sim_params.numberOfOscillators; i++) {
//         int k, size; 
//         frequency_index_and_its_max(
//             k, size, i, 
//             sim_params.numberOfOscillators, 
//             sim_params.boundaryType.selected);
//         try {
//             omega[i] = compute_expression(rpn_list, {
//                 {{"k", k}, {"k_max", size}}});
//         } catch (...) {
//             return;
//         }
//     }
//     for (int i = 0; i < sim_params.numberOfOscillators; i++)
//         m_omega[i] = omega[i];
// }