#include "gl_wrappers.hpp"
#include <complex>

#ifndef _INITIAL_NORMAL_MODE_WAVE_FUNCTION_
#define _INITIAL_NORMAL_MODE_WAVE_FUNCTION_

#define MAX_SIZE 512

/*
Struct to keep track of the initial coupled harmonic oscillator 
wave function configuration in normal coordinates.

If the wave function is configured as a purely coherent state, x is used
for the initial expectation values of each normal mode amplitude, and p
is used for their corresponding momenta; other class members are not used.
If configured as a squeezed state, the member s is used to keep track
of the initial standard deviations, where each element of s gives the
initial normal mode standard deviation divided by its ground state standard 
deviation. Like coherent, x and p are also used to keep track of the initial
expected positions and momenta. If configured as an energy eigenstate only the
member excitations is used, where this counts the number of excitations
assigned to each normal mode.*/
struct InitialNormalModeWaveFunction {
    size_t size;  // Number of normal modes
    double x[MAX_SIZE];  // Expectation value of normal mode amplitudes
    double p[MAX_SIZE];  // Expectation value of normal mode momenta
    double s[MAX_SIZE];  // Proportional to each normal mode's standard dev.
    size_t excitations[MAX_SIZE];  // Energy level of normal modes
    std::complex<double> coefficients[MAX_SIZE];
    InitialNormalModeWaveFunction(size_t size);
    void resize(size_t new_size);  // Change the number of normal modes.
    void zero_excitations();
    void zero_coefficients();
    void set_s_to_ones();
};

#endif