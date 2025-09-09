# 1D Coupled Harmonic Oscillators (WIP)

[Visualization](https://marl0ny.github.io/Pilot-projects/CoupledOscillators/index.html) of a 1D chain of coupled quantum harmonic oscillators, in terms of its untransformed positions and in decoupled normal coordinates. This is primarily based on an [AAPT article](https://doi.org/10.1119/1.1446858) by Scott Johnson and Thomas Gutierrez.

# References:

 - Johnson S. and Gutierrez T., 
"Visualizing the phonon wave function",
<i>American Journal of Physics</i>, vol 70, 227-237 (2002),
https://doi.org/10.1119/1.1446858

### Quantum harmonic oscillator

 - Shankar R., <i>Principles of Quantum Mechanics</i>, 2nd ed,
Springer, 1994, pg 185-221.

Chapter 7 of this book gives an overview of the 1D harmonic oscillator. See pg 195 for the energy eigenstate solutions of this system, pg 610 for finding its coherent states, and pg 196 for its position space propagator. The propagator and the Gaussian integral formula on pg 660 are used to obtain the time evolution of a Gaussian wave packet with arbitrary width.

### Metropolis algorithm

An outline of this algorithm is given on pg 429-430 of:

- Gould H., Tobochnik J., Christian W., "Numerical and Monte Carlo Methods,"
in <i>An Introduction to Computer Simulation Methods</i>,
2016, ch 11., pg 406-444, Available: https://www.compadre.org/osp/items/detail.cfm?ID=7375

The wave function of N coupled 1D harmonic oscillators actually breathes in an N-dimensional space, and therefore Monte Carlo methods like the Metropolis algorithm must be used when numerically evaluating integrals of functions with such high dimensionality; see pg 233 of Johnson and Gutierrez. This needs to be only done once to get the samples that correspond to the initial wave function at time t=0: the [guiding equation](https://en.wikipedia.org/wiki/De_Broglie%E2%80%93Bohm_theory) from De Broglie–Bohm theory is used to get their configuration for all subsequent times.
