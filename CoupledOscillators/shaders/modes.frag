/*Visualization of various normal mode wave function states
of the coupled harmonic oscillator. Each normal mode acts as an independent
1D harmonic oscillator, where the formulas for the energy eigenstates and 
other relevant states of the 1D system can be found in

Shankar R., "The Harmonic Oscillator,"
in <i>Principles of Quantum Mechanics</i>, 2nd ed,
Springer, 1994, ch. 7., pg 185-221.
*/
#if (__VERSION__ >= 330) || (defined(GL_ES) && __VERSION__ >= 300)
#define texture2D texture
#else
#define texture texture2D
#endif

#if (__VERSION__ > 120) || defined(GL_ES)
precision highp float;
#endif

#if __VERSION__ <= 120
varying vec2 UV;
#define fragColor gl_FragColor
#else
in vec2 UV;
out vec4 fragColor;
#endif

uniform int numberOfOscillators;
uniform bool colorPhase;
uniform float t;
uniform float m;
uniform float omega;
uniform float hbar;
uniform float scale;
uniform float offset;
uniform float brightness;
uniform sampler2D initialValuesTex;
uniform int waveFunctionType;
const int ALL_COHERENT = 0;
const int ALL_SQUEEZED = 1;
const int ENERGY_EIGENSTATE = 2;


#define complex vec2

const float PI = 3.141592653589793;


complex expI(float angle) {
    return complex(cos(angle), sin(angle));
}

complex cExp(complex z) {
    return exp(z.r)*complex(cos(z.y), sin(z.y));
}

complex mul(complex z, complex w) {
    return complex(z.x*w.x - z.y*w.y, z.x*w.y + z.y*w.x);
}

complex conj(complex z) {
    return complex(z[0], -z[1]);
}

float getTallThinGaussianStandardDev() {
    return 0.01;
}

complex tallThinGaussian(float x, float x0) {
    float s = getTallThinGaussianStandardDev();
    return complex(
        exp(-0.25*pow((x - x0)/s, 2.0))/sqrt(s*sqrt(2.0*PI)), 0.0);
}

/* Formula for the harmonic oscillator coherent state from
Shankar, pg. 610, equation 21.1.132 in exercise 21.2.18. */
complex coherentState(
    float x, float t, 
    float x0, float p0,
    float m, float omega, float hbar
) {
    if (omega == 0.0)
        return tallThinGaussian(x, x0);
    complex z = complex(sqrt((m*omega)/(2.0*hbar))*x0,
                        sqrt(1.0/(2.0*m*omega*hbar))*p0);
    z = mul(z, expI(-omega*t));
    return pow(m*omega/(PI*hbar), 0.25)*(
        cExp(- z.r*z
             - complex(m*omega/(2.0*hbar)*x*x, 0.0)
             + sqrt(2.0*m*omega/hbar)*x*z));
}

/* Recursive Hermite polynomials.

See Shankar, pg. 195, 7.3.21 to obtain the base cases,
then 7.3.35 for the recursive relation itself.
*/
float hermite(int n, float x) {
    float prev1, prev2, res;
    for (int i = 0; i <= n; i++) {
        if (i == 0) {
            res = 1.0;
            prev2 = 1.0;
        } else if (i == 1) {
            res = 2.0*x;
            prev1 = 2.0*x;
        } else if (i > 1) {
            res = 2.0*(x*prev1 - float(i-1)*prev2);
            prev2 = prev1, prev1 = res;
        }
    }
    return res;
}

float factorial(float n) {
    float res = 1.0, prev = 1.0;
    for (int i = 0; i <= int(n); i++) {
        if (i > 1) {
            res = float(i)*prev;
            prev = res;
        }
    }
    return res;
}

/* Energy eigenstates for the harmonic oscillator referenced from
Shankar, pg 195, eq 7.3.22.*/
complex stationaryState(
    int n, float x, float t,
    float m, float omega, float hbar
) {
    if (omega == 0.0)
        return tallThinGaussian(x, 0.0);
    complex i = complex(0.0, 1.0);
    float normFactor = 
        pow(m*omega/(PI*hbar), 0.25)
        * (1.0/pow(2.0, 2.0*float(n)/4.0))
        * (1.0/pow(factorial(float(n)), 0.5));
    float absVal = (normFactor*hermite(n, x*sqrt(m*omega/hbar)))
        *exp(-0.5*m*omega*x*x/hbar);
    #if (__VERSION__ > 130)
    if (isnan(absVal))
        absVal = 0.0;
    #endif
    return absVal*cExp(-i*omega*(float(n) + 0.5)*t);
}

/* If the initial wave function in a harmonic oscillator is a Gaussian at t=0,
get it for all subsequent times by integrating this initial wave function 
with the harmonic oscillator propagator.

See equation 7.3.28 on pg 196 chapter 7 of Shankar for the formula for the
harmonic oscillator propagator. In order to evaluate the integral of the
initial Gaussian wave function with the propagator which also happens to take
the form of a Gaussian, equations A.2.4 - A.2.5 in page 660 of the appendix
of Shankar was consulted.
*/
complex squeezedStatePhaseFactor(
    float x, float t, 
    float x0, float p0, float sigma0,
    float m, float omega, float hbar
) {
    float c = cos(omega*t);
    float s = sin(omega*t);
    float c2 = c*c;
    float c3 = c*c*c;
    float s2 = s*s;
    float hbar2 = hbar*hbar;
    float x2 = x*x;
    float x02 = x0*x0;
    float m2 = m*m;
    float m3 = m*m*m;
    float sigma4 = sigma0*sigma0*sigma0*sigma0;
    float p02 = p0*p0;
    float omega2 = omega*omega;
    float omega3 = omega*omega*omega;
    float omega_t = omega*t;
    float phase = (
        2.0*c3*m3*omega3*sigma4*x2 
        + c*m*omega*(hbar2*s2*x2 + hbar2*s2*x02 
                     - 4.0*m2*omega2*sigma4*x2 + 8.0*m*omega*p0*s*sigma4*x 
                     - 4.0*p02*s2*sigma4)/2.0
        + hbar2*s2*x0*(-m*omega*x + p0*s)
        )/(hbar*s*(4.0*c2*m2*omega2*sigma4 + hbar2*s2));
    float eps = 1e-30;
    if (abs(mod(omega_t, (2.0*PI))) < eps)
        return mul(cExp(complex(0.0, p0*x/hbar)), 
                   complex(1.0, -1.0)/sqrt(2.0));
    else if (abs(mod(omega_t, (PI))) < eps)
        mul(cExp(complex(0.0, -p0*x/hbar)), 
            complex(1.0, -1.0)/sqrt(2.0));
    return mul(cExp(complex(0.0, phase)),
               complex(1.0, -1.0)/sqrt(2.0));
}

complex squeezedState(
    float x, float t, 
    float x0, float p0, float sigma0,
    float m, float omega, float hbar
) {
    if (omega == 0.0)
        return tallThinGaussian(x, x0);
    float c = cos(t*omega), s = sin(t*omega);
    float hbar2 = hbar*hbar;
    float m2 = m*m;
    float omega2 = omega*omega;
    float sigma4 = sigma0*sigma0*sigma0*sigma0;
    float xT = x0*c + p0*s/(m*omega);
    float sT = sqrt(hbar2*s*s + 4.0*m2*omega2*sigma4*c*c)
        /abs(2.0*m*omega*sigma0);
    complex phaseFactor = (!colorPhase)?
        complex(1.0, 0.0):
        squeezedStatePhaseFactor(
            x, t, x0, p0, sigma0, m, omega, hbar);
    return exp(-0.25*pow((x - xT)/sT, 2.0))/sqrt(sT*sqrt(2.0*PI))
        *phaseFactor;
}


vec3 argumentToColor(float argVal) {
    float maxCol = 1.0;
    float minCol = 50.0/255.0;
    float colRange = maxCol - minCol;
    if (argVal <= PI/3.0 && argVal >= 0.0) {
        return vec3(maxCol,
                    minCol + colRange*argVal/(PI/3.0), minCol);
    } else if (argVal > PI/3.0 && argVal <= 2.0*PI/3.0){
        return vec3(maxCol - colRange*(argVal - PI/3.0)/(PI/3.0),
                    maxCol, minCol);
    } else if (argVal > 2.0*PI/3.0 && argVal <= PI){
        return vec3(minCol, maxCol,
                    minCol + colRange*(argVal - 2.0*PI/3.0)/(PI/3.0));
    } else if (argVal < 0.0 && argVal > -PI/3.0){
        return vec3(maxCol, minCol,
                    minCol - colRange*argVal/(PI/3.0));
    } else if (argVal <= -PI/3.0 && argVal > -2.0*PI/3.0){
        return vec3(maxCol + (colRange*(argVal + PI/3.0)/(PI/3.0)),
                    minCol, maxCol);
    } else if (argVal <= -2.0*PI/3.0 && argVal >= -PI){
        return vec3(minCol,
                    minCol - (colRange*(argVal + 2.0*PI/3.0)/(PI/3.0)), 
                    maxCol);
    }
    else {
        return vec3(minCol, maxCol, maxCol);
    }
}

void main() {
    float x = (2.0*(UV[1] - 0.5 - offset))*scale;
    vec4 initialValues = texture2D(initialValuesTex, UV);
    vec4 initialXPOmegaSigma = initialValues;
    vec4 initialNOmega = initialValues;
    float x0 = initialXPOmegaSigma[0];
    float p0 = initialXPOmegaSigma[1];
    float omega = initialXPOmegaSigma[2];
    float sigma = initialXPOmegaSigma[3];
    int n = int(initialNOmega[0]);
    complex amplitude;
    if (waveFunctionType == ALL_COHERENT)
        amplitude = coherentState(x, t, x0, p0, m, omega, hbar);
    else if (waveFunctionType == ALL_SQUEEZED)
        amplitude = squeezedState(x, t, x0, p0, sigma, m, omega, hbar);
    else if (waveFunctionType == ENERGY_EIGENSTATE)
        amplitude = stationaryState(n, x, t, m, omega, hbar);
    if (dot(amplitude, amplitude) == 1.0/0.0)
        amplitude = complex(0.0, 0.0);
    if (colorPhase)
        fragColor = brightness*vec4(
            argumentToColor(atan(amplitude.y, amplitude.x)),
            dot(amplitude, amplitude)
        );
    else
        fragColor = brightness*vec4(vec3(1.0), dot(amplitude, amplitude));
}