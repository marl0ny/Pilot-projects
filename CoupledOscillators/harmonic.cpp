/*Compute stationary states and various other states
in the 1D harmonic oscillator. The main reference is

Shankar R., "The Harmonic Oscillator,"
in <i>Principles of Quantum Mechanics</i>, 2nd ed,
Springer, 1994, ch. 7., pg 185-221.

*/
#include "harmonic.hpp"

using std::complex;
using std::vector;

#define PI 3.141592653589793


static double factorial(double n) {
    double res = 1, prev = 1;
    for (size_t i = 0; i < (n+1); i++) {
        if (i > 1) {
            res = i*prev;
            prev = res;
        }
    }
    return res;
}

/* Recursive Hermite polynomials.

See Shankar, pg. 195, 7.3.21 to obtain the base cases,
then 7.3.35 for the recursive relation itself.
*/
static double hermite(size_t n, double x) {
    double x2 = x*x;
    double x3 = x2*x;
    double x4 = x2*x2;
    double x5 = x4*x;
    double x6 = x3*x3;
    double x7 = x6*x;
    double x8 = x4*x4;
    double x9 = x8*x;
    double x10 = x5*x5;
    double x11 = x10*x;
    double x12 = x6*x6;
    double x13 = x12*x;
    double x14 = x7*x7;
    double x15 = x14*x;
    double x16 = x8*x8;
    double x17 = x16*x;
    double x18 = x9*x9;
    double x19 = x18*x;
    switch(n) {
        case 0: return 1;
        case 1: return 2*x;
        case 2: return 4*x2 - 2;
        case 3: return 8*x3 - 12*x;
        case 4: return 16*x4 - 48*x2 + 12;
        case 5: return 32*x5 - 160*x3 + 120*x;
        case 6: return 64*x6 - 480*x4 + 720*x2 - 120;
        case 7: return 128*x7 - 1344*x5 + 3360*x3 - 1680*x;
        case 8: return 256*x8 - 3584*x6 + 13440*x4 - 13440*x2 + 1680;
        case 9: return 512*x9 - 9216*x7 + 48384*x5 - 80640*x3 + 30240*x;
        case 10: 
            return 1024*x10 - 23040*x8 + 161280*x6 
                - 403200*x4 + 302400*x2 - 30240;
        case 11: 
            return 2048*x11 - 56320*x9 
                + 506880*x7 - 1774080*x5 + 2217600*x3 - 665280*x;
        case 12:
            return 4096*x12 - 135168*x10 + 1520640*x8 
                - 7096320*x6 + 13305600*x4 - 7983360*x2 + 665280;
        case 13: 
            return 8192*x13 - 319488*x11 + 4392960*x9 
            - 26357760*x7 + 69189120*x5 - 69189120*x3 + 17297280*x;
        case 14: 
            return 16384*x14 - 745472*x12 + 12300288*x10 
            - 92252160*x8 + 322882560*x6 - 484323840*x4 
            + 242161920*x2 - 17297280;
        case 15: 
            return 32768*x15 - 1720320*x13 + 33546240*x11 
            - 307507200*x9 + 1383782400*x7 - 2905943040*x5 
            + 2421619200*x3 - 518918400*x;
        case 16: 
            return 65536*x16 - 3932160*x14 + 89456640*x12
             - 984023040*x10 + 5535129600*x8 - 15498362880*x6 
             + 19372953600*x4 - 8302694400*x2 + 518918400;
        case 17: 
            return 131072*x17 - 8912896*x15 + 233963520*x13 
            - 3041525760*x11 + 20910489600*x9 - 75277762560*x7 
            + 131736084480*x5 - 94097203200*x3 + 17643225600*x;
        case 18: 
            return 262144*x18 - 20054016*x16 + 601620480*x14 
            - 9124577280*x12 + 75277762560*x10 - 338749931520*x8 
            + 790416506880*x6 - 846874828800*x4 + 317578060800*x2 
            - 17643225600;
        case 19: 
            return 524288*x19 - 44826624*x17 + 1524105216*x15 
            - 26671841280*x13 + 260050452480*x11 - 1430277488640*x9 
            + 4290832465920*x7 - 6436248698880*x5 + 4022655436800*x3 
            - 670442572800*x;
        default:
        return 2.0*x*hermite(n-1, x) - 2.0*(n-1)*hermite(n-2, x);   
    }
}

static double get_tall_thin_gaussian_standard_dev() {
    return 0.01;
}

static complex<double> tall_thin_gaussian(double x, double x0) {
    double s = get_tall_thin_gaussian_standard_dev();
    return exp(-0.25*pow((x - x0)/s, 2.0))/sqrt(s*sqrt(2.0*PI));
}

/* Energy eigenstates for the harmonic oscillator referenced from
Shankar, pg 195, eq 7.3.22.*/
complex<double> stationary_state(
    size_t n, double x, double t,
    double m, double omega, double hbar
) {
    if (omega == 0.0)
        return tall_thin_gaussian(x, 0.0);
    complex<double> i (0.0, 1.0);
    double norm_factor = pow(
        m*omega/(PI*hbar*pow(2.0, 2.0*n)*pow(factorial(n), 2.0)), 0.25);
    // 1/(4*sigma^2) = (m*omega)/(2*hbar)
    // 1/sigma^2 = 2*m*omega/hbar
    // sigma^2 = hbar/(2*m*omega)
    // sigma = sqrt(hbar/(2*m*omega))
    return norm_factor*(
        exp(-0.5*m*omega*x*x/hbar - i*omega*(n + 0.5)*t)
        *hermite(n, x*sqrt(m*omega/hbar)));
}

std::complex<double> stationary_states_combination(
    std::vector<std::complex<double>> coefficients,
    double x, double t,
    double m, double omega, double hbar
) {
    std::complex<double> psi_t = 0.0;
    for (size_t n = 0; n < coefficients.size(); n++)
        psi_t += coefficients[n]*stationary_state(n, x, t, m, omega, hbar);
    return psi_t;
}

/* Formula for the harmonic oscillator coherent state from
Shankar, pg. 610, equation 21.1.132 in exercise 21.2.18. */
complex<double> coherent_state(
    double x, double t, 
    double x0, double p0,
    double m, double omega, double hbar
) {
    if (omega == 0.0)
        return tall_thin_gaussian(x, x0);
    complex<double> i (0.0, 1.0);
    complex<double> z = 
        sqrt((m*omega)/(2.0*hbar))*x0
        + i*sqrt(1.0/(2.0*m*omega*hbar))*p0; 
    z *= exp(-i*omega*t);
    return std::pow(m*omega/(PI*hbar), 0.25)*(
        std::exp(-real(z)*z
            - m*omega/(2.0*hbar)*x*x 
            + sqrt(2.0*m*omega/hbar)*z*x));
}

static std::complex<double> squeezed_state_phase_factor(
    double x, double t, 
    double x0, double p0, double sigma0,
    double m, double omega, double hbar
) {
    double c = cos(omega*t);
    double s = sin(omega*t);
    double c2 = c*c;
    double c3 = c*c*c;
    double s2 = s*s;
    double hbar2 = hbar*hbar;
    double x2 = x*x;
    double x02 = x0*x0;
    double m2 = m*m;
    double m3 = m*m*m;
    double sigma4 = sigma0*sigma0*sigma0*sigma0;
    double p02 = p0*p0;
    double omega2 = omega*omega;
    double omega3 = omega*omega*omega;
    double omega_t = omega*t;
    double phase = (
        2.0*c3*m3*omega3*sigma4*x2 
        + c*m*omega*(hbar2*s2*x2 + hbar2*s2*x02 
                     - 4.0*m2*omega2*sigma4*x2 + 8.0*m*omega*p0*s*sigma4*x 
                     - 4.0*p02*s2*sigma4)/2.0
        + hbar2*s2*x0*(-m*omega*x + p0*s)
        )/(hbar*s*(4.0*c2*m2*omega2*sigma4 + hbar2*s2));
    double eps = 1e-60;
    if (abs(remainder(omega_t, (2.0*PI))) < eps)
        return exp(std::complex<double>(0.0, p0*x/hbar))
            *std::complex<double>(1.0, -1.0)/sqrt(2.0);
    else if (abs(remainder(omega_t, (PI))) < eps)
        return exp(std::complex<double>(0.0, -p0*x/hbar))
            *std::complex<double>(1.0, -1.0)/sqrt(2.0);
    return exp(std::complex<double>(0.0, phase))
        *std::complex<double>(1.0, -1.0)/sqrt(2.0);
}

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
    double m, double omega, double hbar, bool omit_phase
) {
    if (omega == 0.0)
        return tall_thin_gaussian(x, x0);
    double c = cos(t*omega), s = sin(t*omega);
    double hbar2 = hbar*hbar;
    double m2 = m*m;
    double omega2 = omega*omega;
    double sigma4 = sigma0*sigma0*sigma0*sigma0;
    double x_t = x0*c + p0*s/(m*omega);
    double s_t = sqrt(hbar2*s*s + 4.0*m2*omega2*sigma4*c*c)
        /abs(2.0*m*omega*sigma0);
    std::complex<double> phase_factor = (omit_phase)?
        std::complex<double>(1.0, 0.0):
        squeezed_state_phase_factor(
            x, t, x0, p0, sigma0, m, omega, hbar);
    return exp(-0.25*pow((x - x_t)/s_t, 2.0))/sqrt(s_t*sqrt(2.0*PI))
        *phase_factor;
}

double coherent_standard_dev(
    double m, double omega, double hbar) {
    if (omega == 0.0)
        return get_tall_thin_gaussian_standard_dev();
    return sqrt(hbar/(2.0*m*omega));
}

double squeezed_standard_dev(
    double t,
    double sigma0, double m, double omega, double hbar) {
    if (omega == 0.0)
        return get_tall_thin_gaussian_standard_dev();
    double c = cos(t*omega), s = sin(t*omega);
    double hbar2 = hbar*hbar;
    double omega2 = omega*omega;
    double sigma4 = sigma0*sigma0*sigma0*sigma0;
    double m2 = m*m;
    return sqrt(hbar2*s*s + 4.0*m2*omega2*sigma4*c*c)
        /abs(2.0*m*omega*sigma0);
}

double squeezed_avg_x(
    double t,
    double x0, double p0,
    double m, double omega, double hbar
) {
    if (omega == 0.0)
        return x0;
    return x0*cos(t*omega) + p0*sin(t*omega)/(m*omega);
}

double squeezed_avg_p(
    double t,
    double x0, double p0,
    double m, double omega, double hbar
) {
    if (omega == 0.0)
        return 0.0;
    return -omega*x0*sin(t*omega) + (p0/m)*cos(t*omega);
}
