/*Compute stationary states and various other states
in the 1D harmonic oscillator. The main reference is

Shankar R., "The Harmonic Oscillator,"
in <i>Principles of Quantum Mechanics</i>, 2nd ed,
Springer, 1994, ch. 7., pg 185-221.

*/
#include <complex>
#include <vector>

#ifndef _HARMONIC_
#define _HARMONIC_


/* Energy eigenstates for the harmonic oscillator referenced from
Shankar, pg 195, eq 7.3.22.*/
std::complex<double> stationary_state(
    size_t n, double x, double t,
    double m, double omega, double hbar);

std::complex<double> stationary_states_combination(
    std::vector<std::complex<double>> coefficients,
    double x, double t,
    double m, double omega, double hbar
);

/* Formula for the harmonic oscillator coherent state from
Shankar, pg. 610, equation 21.1.132 in exercise 21.2.18. */
std::complex<double> coherent_state(
    double x, double t, 
    double x0, double p0,
    double m, double omega, double hbar);

std::complex<double> coherent_states_combination(
    double x, double t, 
    std::vector<std::complex<double>> coefficients,
    std::vector<double> x0, std::vector<double> p0,
    double m, double omega, double hbar);

double coherent_standard_dev(
    double m, double omega, double hbar);

/*
If the initial wave function in a harmonic oscillator is a Gaussian at t=0,
get it for all subsequent times by integrating this initial wave function 
with the harmonic oscillator propagator.

See equation 7.3.28 on pg 196 chapter 7 of Shankar for the formula for the
harmonic oscillator propagator. In order to evaluate the integral of the
initial Gaussian wave function with the propagator which also happens to take
the form of a Gaussian, equations A.2.4 - A.2.5 in page 660 of the appendix
of Shankar was consulted.
*/
std::complex<double> squeezed_state(
    double x, double t, 
    double x0, double p0, double sigma0,
    double m, double omega, double hbar, bool omit_phase=true
);

double squeezed_standard_dev(
    double t,
    double sigma0, double m, double omega, double hbar);

double squeezed_avg_x(
    double t,
    double x0, double p0,
    double m, double omega, double hbar
);

double squeezed_avg_p(
    double t,
    double x0, double p0,
    double m, double omega, double hbar
);

#endif

