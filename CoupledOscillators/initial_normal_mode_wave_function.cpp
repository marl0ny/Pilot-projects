#include "initial_normal_mode_wave_function.hpp"


InitialNormalModeWaveFunction
::InitialNormalModeWaveFunction(size_t size) {
    this->size = size;
    for (int i = 0; i < MAX_SIZE; i++) {
        this->x[i] = 0.0;
        this->p[i] = 0.0;
        this->excitations[i] = 0;
        this->s[i] = 1.0;
        this->coefficients[i] = 0.0;
    }
}

void InitialNormalModeWaveFunction::resize(size_t new_size) {
    this->size = new_size;
}

void InitialNormalModeWaveFunction::zero_excitations() {
    for (int i = 0; i < MAX_SIZE; i++)
        this->excitations[i] = 0;
}

void InitialNormalModeWaveFunction::zero_coefficients() {
    for (int i = 0; i < MAX_SIZE; i++)
        this->coefficients[i] = 0.0;
}

void InitialNormalModeWaveFunction::set_s_to_ones() {
    for (int i = 0; i < MAX_SIZE; i++)
        this->s[i] = 1.0;
}
