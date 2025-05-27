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
uniform float t;
uniform float dt;
uniform float m;
uniform float hbar;
uniform sampler2D initialValuesTex;
uniform int trajectoriesTexWidth;
uniform sampler2D trajectoriesTex;

const float PI = 3.141592653589793;

float xDot(float x, float t, float x0, float p0, float sigma0, float omega) {
    float hbar2 = hbar*hbar;
    float omegaT = omega*t;
    float omega2 = omega*omega;
    float omega3 = omega2*omega;
    float sigma4 = sigma0*sigma0*sigma0*sigma0;
    float c = cos(omega*t), s = sin(omega*t);
    float c2 = c*c, c3 = c*c*c;
    float s2 = s*s, s3 = s*s*s;
    float m2 = m*m, m3 = m*m*m;
    float eps = 1e-30;
    if (abs(mod(omegaT, (2.0*PI))) < eps)
        return p0/m;
    else if (abs(mod(omegaT, (PI))) < eps)
        return -p0/m;
    float denominator = (4.0*c2*m3*omega2*s*sigma4 + hbar2*m*s3);
    float numerator = x*(
        4.0*c3*m3*omega3*sigma4 + c*hbar2*m*omega*s2 - 4.0*c*m3*omega3*sigma4) 
        + 4.0*c*m2*omega2*p0*s*sigma4 - hbar2*m*omega*s2*x0;
    return numerator/denominator;
}

float rk4(float x, float t, float x0, float p0, float sigma0, float omega) {
    float dxDt1 = xDot(x, t, x0, p0, sigma0, omega);
    float dxDt2 = xDot(x + 0.5*dt*dxDt1, t + dt/2.0, x0, p0, sigma0, omega);
    float dxDt3 = xDot(x + 0.5*dt*dxDt2, t + dt/2.0, x0, p0, sigma0, omega);
    float dxDt4 = xDot(x + dt*dxDt3, t + dt, x0, p0, sigma0, omega);
    return x + (1.0/6.0)*dt*(dxDt1 + 2.0*dxDt2 + 2.0*dxDt3 + dxDt4);
}

float squeezedTrajectory(
    float x, float t, float x0, float p0, float sigma0, float omega) {
    // if (omega == 0.0)
    //     return tallThinGaussian(x, x0);
    float c = cos(t*omega), s = sin(t*omega);
    float hbar2 = hbar*hbar;
    float m2 = m*m;
    float omega2 = omega*omega;
    float sigma4 = sigma0*sigma0*sigma0*sigma0;
    float xT = x0*c + p0*s/(m*omega);
    float sT = sqrt(hbar2*s*s + 4.0*m2*omega2*sigma4*c*c)
        /abs(2.0*m*omega*sigma0);
    return xT + (x - x0)*sT/sigma0;
}


void main() {
    float x = texture2D(trajectoriesTex, UV).x;
    float ind = mod(
        float(trajectoriesTexWidth)*UV.x-0.5, 
        float(numberOfOscillators)) + 0.5;
    vec4 initialValues = texture2D(
        initialValuesTex, vec2(ind/float(numberOfOscillators), 0.5));
    vec4 initialXPOmegaSigma = initialValues;
    float x0 = initialXPOmegaSigma[0];
    float p0 = initialXPOmegaSigma[1];
    float omega = initialXPOmegaSigma[2];
    float sigma0 = initialXPOmegaSigma[3];
    fragColor = vec4(squeezedTrajectory(x, t, x0, p0, sigma0, omega));
    // float dt2 = dt/10.0;
    // x = rk4(x, t, dt2, x0, p0, sigma0, omega);
    // float t2 = t + dt/10.0;
    // for (int i = 0; i < 10; i++) {
    //     x = rk4(x, t2, dt2, x0, p0, sigma0, omega);
    //     t2 += dt2;
    // }
    // fragColor = vec4(x);
}