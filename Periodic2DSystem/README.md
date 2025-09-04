# 2D Pilot Wave System

For solving the 2D Schrödinger equation, a [2nd-order accurate centered finite difference numerical integration scheme in time](https://doi.org/10.1063/1.168415) is used, and a 4th order accurate stencil in space is constructed for discretizing the Laplacian (have a look at this [article](https://doi.org/10.1090/S0025-5718-1988-0935077-0 ) for a convenient chart of various higher-order finite difference stencils). Using these numerical solutions of the wave function, the [4th order Runge-Kutta method](https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods) is then used for integrating the trajectories of the Bohmian particles.


