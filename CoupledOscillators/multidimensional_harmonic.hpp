#include "harmonic.hpp"
#include <vector>
#include <complex>

#ifndef _MULTIDIMENSIONAL_HARMONIC_
#define _MULTIDIMENSIONAL_HARMONIC_

struct StationaryStatesProdData {
    double t, m, hbar;
    std::vector<int> excitations;
    std::vector<double> omega;
};

struct CoherentStateProdData {
    double t, m, hbar;
    std::vector<double> x0, p0, omega;
};

struct SqueezedStateProdData {
    double t, m, hbar;
    std::vector<double> x0, p0, sigma0, omega;
};

struct SingleExcitationsStateData {
    double t, m, hbar;
    std::vector<double> omega;
    std::vector<std::complex<double>> coeff;
};

double stationary_states_prod_dist_func(
    const std::vector<double> &x, void *data_ptr
);

double coherent_state_prod_dist_func(
    const std::vector<double> &x, void *data_ptr
);

double squeezed_state_prod_dist_func(
    const std::vector<double> &x, void *data_ptr
);

double single_excitations_sum_dist_func(
    const std::vector<double> &x, void *data_ptr
);

#endif