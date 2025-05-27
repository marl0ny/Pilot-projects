#include "multidimensional_harmonic.hpp"

using std::complex;

typedef std::vector<double> Arr1D;
typedef std::vector<complex<double>> ArrC1D;
typedef std::vector<int> ArrI1D;

static double coherent_state_prod(
    const Arr1D &x, double t, 
    const Arr1D &x0, const Arr1D &p0, double m,
    const Arr1D &omega, double hbar
) {
    std::complex<double> prod = 1.0;
    for (int i = 0; i < x.size(); i++)
        prod *= coherent_state(x[i], t, x0[i], p0[i], m, omega[i], hbar);
    return abs(prod)*abs(prod);
}


double coherent_state_prod_dist_func(
    const Arr1D &x, void *data_ptr
) {
    CoherentStateProdData *data = (CoherentStateProdData *)data_ptr;
    return coherent_state_prod(
        x, data->t, data->x0, data->p0, data->m, data->omega, data->hbar);
}

static double stationary_states_prod(
    const Arr1D &x, double t,
    const ArrI1D &excitations,
    double m, const Arr1D &omega, double hbar
) {
    std::complex<double> prod = 1.0;
    for (int i = 0; i < excitations.size(); i++)
        prod *= 
            stationary_state(excitations[i], x[i], t, m, omega[i], hbar);
    return abs(prod)*abs(prod);
}

double stationary_states_prod_dist_func(
    const Arr1D &x, void *data_ptr
) {
    StationaryStatesProdData *data = (StationaryStatesProdData *)data_ptr;
    return stationary_states_prod(x, data->t, data->excitations,
         data->m, data->omega, data->hbar);
}

static double squeezed_state_prod(
    const Arr1D &x, double t, 
    const Arr1D &x0, const Arr1D &p0, const Arr1D &sigma0,
    double m, const Arr1D &omega, double hbar
) {
    std::complex<double> prod = 1.0;
    for (int i = 0; i < x.size(); i++)
        prod *= squeezed_state(
            x[i], t, x0[i], p0[i], 
            sigma0[i], m, omega[i], hbar);
    return abs(prod)*abs(prod);
}

double squeezed_state_prod_dist_func(
    const Arr1D &x, void *data_ptr
) {
    SqueezedStateProdData *data = (SqueezedStateProdData *)data_ptr;
    return squeezed_state_prod(
        x, data->t, data->x0, data->p0, 
        data->sigma0, data->m, data->omega, data->hbar);
}

static double single_excitations_sum(
    const Arr1D &x, const ArrC1D &coeff,
    double t, double m, Arr1D &omega, double hbar
) {
    std::complex<double> sum = 0.0;
    ArrC1D ground_states (x.size());
    for (int i = 0; i < x.size(); i++)
        ground_states[i] = stationary_state(0, x[i], t, m, omega[i], hbar);
    for (int i = 0; i < x.size(); i++) {
        if (abs(coeff[i]) != 0.0) {
            std::complex<double> prod = 1.0;
            for (int k = 0; k < x.size(); k++) {
                if (k != i)
                    prod *= ground_states[k];
            }
            sum += coeff[i]*prod*stationary_state(
                1, x[i], t, m, omega[i], hbar);
        }
    }
    return abs(sum)*abs(sum);
}

double single_excitations_sum_dist_func(
    const Arr1D &x, void *data_ptr
) {
    SingleExcitationsStateData *data
         = (SingleExcitationsStateData *)data_ptr;
    return single_excitations_sum(
        x, data->coeff, 
        data->t, data->m, data->omega, data->hbar);

}