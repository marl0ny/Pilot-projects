""" From the analytical squeezed wave function
in a harmonic oscillator, attempt to get the equations
of motion for a particle guided by this squeezed state.
This uses the guiding equation which can be found for example
in the Wikipedia page on De Broglie-Bohm theory:

https://en.wikipedia.org/wiki/De_Broglie%E2%80%93Bohm_theory

"""
import numpy as np
from scipy.integrate import odeint
import matplotlib.pyplot as plt
from sympy import Symbol, integrate, limit, pi, sqrt, diff
import sympy as sym


HBAR = 1.0
MASS = 1.0
SIGMA = 1.0
OMEGA = 1.0
X0, P0 = 4.0, 0.0
TOTAL_STEPS = 1000
TOTAL_TRAJECTORIES = 100
TIME_SLICES = np.linspace(0.0, 40.0, TOTAL_STEPS)


def get_analytic_relations_for_trajectories():
    t, t1, t2 \
        = Symbol('t', real=True), Symbol('t1', real=True), \
            Symbol('t2', real=True)
    hbar = Symbol('hbar', real=True)
    omega = Symbol('omega', positive=True)
    sigma = Symbol('sigma', real=True)
    factor = Symbol('f1', positive=True)
    x0, p0 = Symbol('x0', real=True), Symbol('p0', real=True)
    m = Symbol('m', real=True)
    hbar2 = hbar**2
    omega2, omega3 = omega**2, omega**3
    sigma4 = sigma**4
    c, s = sym.cos(omega*t), sym.sin(omega*t)
    c2, c3 = c**2, c**3
    s2, s3 = s**2, s**3
    m2, m3 = m**2, m**3
    p_t = -(
        4*c3*m3*omega3*sigma4/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3) 
        + c*hbar2*m*omega*s2/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3) 
        - 4*c*m3*omega3*sigma4/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3))
    a_t = (
        4*c*m2*omega2*p0*s*sigma4/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3)
        - hbar2*m*omega*s2*x0/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3))
    print(integrate(c3/(s*(factor - s2)), t))
    # print(int_factor := integrate(p_t, t, [t1, t2]))
    # print(integrate(int_factor*a_t, t, [t1, t2]))


# Did not give an answer in the desired timeframe!
# get_analytic_relations_for_trajectories()

def get_limiting_conditions():
    x = Symbol('x', real=True)
    t, t1, t2 \
        = Symbol('t', real=True), Symbol('t1', real=True), \
            Symbol('t2', real=True)
    hbar = Symbol('hbar', real=True)
    omega = Symbol('omega', real=True)
    sigma = Symbol('sigma', real=True)
    x0, p0 = Symbol('x0', real=True), Symbol('p0', real=True)
    m = Symbol('m', real=True)
    hbar2 = hbar**2
    omega2, omega3 = omega**2, omega**3
    sigma4 = sigma**4
    c, s = sym.cos(omega*t), sym.sin(omega*t)
    c2, c3 = c**2, c**3
    s2, s3 = s**2, s**3
    m2, m3 = m**2, m**3
    x_dot = (
        4*c*m2*omega2*p0*s*sigma4/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3) 
        - hbar2*m*omega*s2*x0/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3) 
        + x*(4*c3*m3*omega3*sigma4/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3) 
             + c*hbar2*m*omega*s2/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3) 
             - 4*c*m3*omega3*sigma4/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3)))
    print(x_dot.subs(sigma, sqrt(hbar/(2*m*omega))).simplify())
    # print(limit(x_dot, t, 0.0))
    # print(limit(x_dot.subs(omega, pi), t, 1))

# get_limiting_conditions()

def compare():
    x = Symbol('x', real=True)
    t, t1, t2 \
        = Symbol('t', real=True), Symbol('t1', real=True), \
            Symbol('t2', real=True)
    hbar = Symbol('hbar', real=True)
    omega = Symbol('omega', real=True)
    sigma = Symbol('sigma', real=True)
    x0, p0 = Symbol('x0', real=True), Symbol('p0', real=True)
    m = Symbol('m', real=True)
    hbar2 = hbar**2
    omega2, omega3 = omega**2, omega**3
    sigma4 = sigma**4
    c, s = sym.cos(omega*t), sym.sin(omega*t)
    c2, c3 = c**2, c**3
    s2, s3 = s**2, s**3
    m2, m3 = m**2, m**3
    x_dot = (
        4*c*m2*omega2*p0*s*sigma4/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3) 
        - hbar2*m*omega*s2*x0/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3) 
        + x*(4*c3*m3*omega3*sigma4/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3) 
             + c*hbar2*m*omega*s2/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3) 
             - 4*c*m3*omega3*sigma4/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3)))
    x_avg = x0*c + p0*s/(m*omega)
    sigma_t = sqrt(hbar2*s*s + 4.0*m2*omega2*sigma4*c*c)\
        /abs(2.0*m*omega*sigma)
    x_t = x_avg + (x - x0)*(sigma_t/sigma)
    print(diff(x_t, t))
    # print((diff(x_t, t) - x_dot.simplify()).simplify()
    #       .subs(omega, 1.0)
    #       .subs(hbar, 1.0)
    #       .subs(m, 1.0)
    #       .subs(x0, 1.0)
    #       .subs(p0, 0.0)
    #       .subs(sigma, 1.0)
    #       .subs(x, 1.0))
    # print(limit(x_dot, t, 0.0))
    # print(limit(x_dot.subs(omega, pi), t, 1))

def get_x_avg(t):
    return X0*np.cos(OMEGA*t) + P0*np.sin(OMEGA*t)/(MASS*OMEGA)

def get_sigma_t(t):
    return np.sqrt(HBAR**2*np.sin(OMEGA*t)**2 
                   + 4.0*MASS**2*OMEGA**2*SIGMA**4*np.cos(OMEGA*t)**2
                   )/np.abs(2.0*MASS*OMEGA*SIGMA)

# compare()

# # func()
# import sys; sys.exit()

def eom(x, t):
    hbar2 = HBAR**2
    omega, omega2, omega3 = OMEGA, OMEGA**2, OMEGA**3
    omega_t = omega*t
    sigma4 = SIGMA**4
    x0, p0 = X0, P0
    c, s = np.cos(omega*t), np.sin(omega*t)
    c2, c3 = c**2, c**3
    s2, s3 = s**2, s**3
    m, m2, m3 = MASS, MASS**2, MASS**3
    eps = 1e-60
    if abs(omega_t % (2.0*pi)) < eps:
        return p0/m*np.ones(x.shape)
    elif abs(omega_t % pi) < eps:
        return -p0/m*np.ones(x.shape)
    return (
        4*c*m2*omega2*p0*s*sigma4/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3) 
        - hbar2*m*omega*s2*x0/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3) 
        + x*(4*c3*m3*omega3*sigma4/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3) 
             + c*hbar2*m*omega*s2/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3) 
             - 4*c*m3*omega3*sigma4/(4*c2*m3*omega2*s*sigma4 + hbar2*m*s3))
    )

def eom2(x, t):
    hbar = HBAR
    sigma = SIGMA
    omega = OMEGA
    m = MASS
    x0, p0 = X0, P0
    return -omega*x0*np.sin(omega*t) \
            + 1.0*(x - x0)*(0.25*hbar**2*omega*np.sin(omega*t)*np.cos(omega*t) 
                            - m**2*omega**3*sigma**4*np.sin(omega*t)*np.cos(omega*t)
                            )/(sigma*np.sqrt(0.25*hbar**2*np.sin(omega*t)**2 
                                             + m**2*omega**2*sigma**4*np.cos(omega*t)**2)*
                                             np.abs(m*omega*sigma)) + p0*np.cos(omega*t)/m

# def stuff(t):
#     hbar2 = HBAR**2
#     omega, omega2, omega3 = OMEGA, OMEGA**2, OMEGA**3
#     omega_t = omega*t
#     sigma4 = SIGMA**4
#     x0, p0 = X0, P0
#     c, s = np.cos(omega*t), np.sin(omega*t)
#     c2, c3 = c**2, c**3
#     s2, s3 = s**2, s**3
#     m, m2, m3 = MASS, MASS**2, MASS**3
#     eps = 1e-60
#     return (4.0*c2*m3*omega2*s*sigma4 + hbar2*m*s3) 

# plt.plot(TIME_SLICES/(np.pi), stuff(TIME_SLICES))
# plt.show()
# plt.close()


def avg_x(t):
    omega = OMEGA
    m = MASS
    x0, p0 = X0, P0
    return x0*np.cos(t*omega) + p0*np.sin(t*omega)/(m*omega)


def standard_dev(t):
    omega = OMEGA
    m = MASS
    hbar = HBAR
    sigma = SIGMA
    c = np.cos(t*omega)
    s = np.sin(t*omega)
    hbar2 = hbar**2
    omega2 = omega**2
    sigma4 = SIGMA**4
    m2 = m*m
    return np.sqrt(hbar2*s*s + 4.0*m2*omega2*sigma4*c*c)\
        /abs(2.0*m*omega*sigma)


# x0 = np.array([X0 + 3.0*SIGMA/4.0, X0 - 3.0*SIGMA/4.0])
x0 = np.linspace(X0, X0 + SIGMA/2.0, TOTAL_TRAJECTORIES//10)
x_t = odeint(eom, x0, TIME_SLICES)
x_t2 = np.array([
    (get_x_avg(TIME_SLICES) 
     + (e - X0)*get_sigma_t(TIME_SLICES)/SIGMA) for e in x0]).T


plt.plot(TIME_SLICES, x_t, alpha=1.0, color='blue')
plt.plot(TIME_SLICES, x_t2, alpha=1.0, color='orange', linestyle='--')
# plt.plot(TIME_SLICES, x_t, alpha=0.01, color='blue')
plt.plot(TIME_SLICES, avg_x(TIME_SLICES), color='black')
plt.plot(TIME_SLICES, 
         avg_x(TIME_SLICES) + standard_dev(TIME_SLICES), 
         color='black', linewidth=0.5, linestyle='--')
plt.plot(TIME_SLICES, 
         avg_x(TIME_SLICES) - standard_dev(TIME_SLICES),
         color='black', linewidth=0.5, linestyle='--')
plt.xlim(TIME_SLICES[0], TIME_SLICES[-1])
plt.show()
plt.close()


