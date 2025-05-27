#include "gl_wrappers.hpp"

namespace sim_2d {

#ifndef _PARAMETERS_
#define _PARAMETERS_

struct Button {};

typedef std::string Label;

typedef bool BoolRecord;

typedef std::vector<std::string> EntryBoxes;

struct SelectionList {
    int selected;
    std::vector<std::string> options;
};

struct SimParams {

struct LineDivider {};

struct NotUsed {};
    int stepsPerFrame = (int)(1);
    float dt = (float)(0.1F);
    float t = (float)(0.0F);
    int stepCount = (int)(0);
    int numberOfOscillators = (int)(64);
    SelectionList boundaryType = SelectionList{0, {"Zero at endpoints", "Periodic"}};
    LineDivider lineDivMonteCarlo = LineDivider{};
    Label labelMonteCarlo = Label{};
    float relativeDelta = (float)(0.66F);
    Label acceptanceRateLabel = Label{};
    float acceptanceRate = (float)(0.0F);
    int numberOfMCSteps = (int)(20000);
    LineDivider lineDivSampleColor = LineDivider{};
    Label labelSamples = Label{};
    float alphaBrightness = (float)(0.01F);
    Vec3 colorOfSamples1 = (Vec3)(Vec3 {.ind={0.0, 0.85, 1.0}});
    Vec3 colorOfSamples2 = (Vec3)(Vec3 {.ind={0.0, 1.0, 0.0}});
    SelectionList displayType = SelectionList{1, {"Lines", "Scatter", "Multi-coloured histogram"}};
    bool showNormalCoordSamples = (bool)(true);
    LineDivider lineDivNormalModeWaveFunc = LineDivider{};
    Label labelNormalModeWaveFunc = Label{};
    bool colorPhase = (bool)(false);
    float modesBrightness = (float)(1.25F);
    LineDivider lineDivWaveFuncOptions = LineDivider{};
    Label waveFuncConfigLabel = Label{};
    bool useCoherentStates = (bool)(true);
    bool useSqueezed = (bool)(false);
    bool useStationary = (bool)(false);
    bool useSingleExcitations = (bool)(false);
    Label noteForUseSingleExcitations = Label{};
    Label coherentOrSqueezedSelectedLabel = Label{};
    SelectionList clickActionNormal = SelectionList{0, {"Change selected while setting others to zero", "Modify selection only"}};
    Label squeezedSelectedLabel = Label{};
    float squeezedFactorGlobal = (float)(1.0F);
    float squeezedFactor = (float)(1.0F);
    Label squeezedStateRelStDevLabel = Label{};
    Label energyEigenstatesSelectedLabel = Label{};
    bool addEnergy = (bool)(true);
    bool removeEnergy = (bool)(false);
    LineDivider lineDivAdditionalOptions = LineDivider{};
    Label DispersionOptionsLabel = Label{};
    SelectionList presetDispersionRelation = SelectionList{0, {"2*sin((pi/2)*(abs(k)/k_max))", "pi*(abs(k)/k_max)"}};
    BoolRecord imageRecord = BoolRecord{false};
    enum {
        STEPS_PER_FRAME=0,
        DT=1,
        T=2,
        STEP_COUNT=3,
        NUMBER_OF_OSCILLATORS=4,
        BOUNDARY_TYPE=5,
        LINE_DIV_MONTE_CARLO=6,
        LABEL_MONTE_CARLO=7,
        RELATIVE_DELTA=8,
        ACCEPTANCE_RATE_LABEL=9,
        ACCEPTANCE_RATE=10,
        NUMBER_OF_M_C_STEPS=11,
        LINE_DIV_SAMPLE_COLOR=12,
        LABEL_SAMPLES=13,
        ALPHA_BRIGHTNESS=14,
        COLOR_OF_SAMPLES1=15,
        COLOR_OF_SAMPLES2=16,
        DISPLAY_TYPE=17,
        SHOW_NORMAL_COORD_SAMPLES=18,
        LINE_DIV_NORMAL_MODE_WAVE_FUNC=19,
        LABEL_NORMAL_MODE_WAVE_FUNC=20,
        COLOR_PHASE=21,
        MODES_BRIGHTNESS=22,
        LINE_DIV_WAVE_FUNC_OPTIONS=23,
        WAVE_FUNC_CONFIG_LABEL=24,
        USE_COHERENT_STATES=25,
        USE_SQUEEZED=26,
        USE_STATIONARY=27,
        USE_SINGLE_EXCITATIONS=28,
        NOTE_FOR_USE_SINGLE_EXCITATIONS=29,
        COHERENT_OR_SQUEEZED_SELECTED_LABEL=30,
        CLICK_ACTION_NORMAL=31,
        SQUEEZED_SELECTED_LABEL=32,
        SQUEEZED_FACTOR_GLOBAL=33,
        SQUEEZED_FACTOR=34,
        SQUEEZED_STATE_REL_ST_DEV_LABEL=35,
        ENERGY_EIGENSTATES_SELECTED_LABEL=36,
        ADD_ENERGY=37,
        REMOVE_ENERGY=38,
        LINE_DIV_ADDITIONAL_OPTIONS=39,
        DISPERSION_OPTIONS_LABEL=40,
        PRESET_DISPERSION_RELATION=41,
        IMAGE_RECORD=42,
    };
    void set(int enum_val, Uniform val) {
        switch(enum_val) {
            case STEPS_PER_FRAME:
            stepsPerFrame = val.i32;
            break;
            case DT:
            dt = val.f32;
            break;
            case T:
            t = val.f32;
            break;
            case STEP_COUNT:
            stepCount = val.i32;
            break;
            case NUMBER_OF_OSCILLATORS:
            numberOfOscillators = val.i32;
            break;
            case RELATIVE_DELTA:
            relativeDelta = val.f32;
            break;
            case ACCEPTANCE_RATE:
            acceptanceRate = val.f32;
            break;
            case NUMBER_OF_M_C_STEPS:
            numberOfMCSteps = val.i32;
            break;
            case ALPHA_BRIGHTNESS:
            alphaBrightness = val.f32;
            break;
            case COLOR_OF_SAMPLES1:
            colorOfSamples1 = val.vec3;
            break;
            case COLOR_OF_SAMPLES2:
            colorOfSamples2 = val.vec3;
            break;
            case SHOW_NORMAL_COORD_SAMPLES:
            showNormalCoordSamples = val.b32;
            break;
            case COLOR_PHASE:
            colorPhase = val.b32;
            break;
            case MODES_BRIGHTNESS:
            modesBrightness = val.f32;
            break;
            case USE_COHERENT_STATES:
            useCoherentStates = val.b32;
            break;
            case USE_SQUEEZED:
            useSqueezed = val.b32;
            break;
            case USE_STATIONARY:
            useStationary = val.b32;
            break;
            case USE_SINGLE_EXCITATIONS:
            useSingleExcitations = val.b32;
            break;
            case SQUEEZED_FACTOR_GLOBAL:
            squeezedFactorGlobal = val.f32;
            break;
            case SQUEEZED_FACTOR:
            squeezedFactor = val.f32;
            break;
            case ADD_ENERGY:
            addEnergy = val.b32;
            break;
            case REMOVE_ENERGY:
            removeEnergy = val.b32;
            break;
        }
    }
    Uniform get(int enum_val) const {
        switch(enum_val) {
            case STEPS_PER_FRAME:
            return {(int)stepsPerFrame};
            case DT:
            return {(float)dt};
            case T:
            return {(float)t};
            case STEP_COUNT:
            return {(int)stepCount};
            case NUMBER_OF_OSCILLATORS:
            return {(int)numberOfOscillators};
            case RELATIVE_DELTA:
            return {(float)relativeDelta};
            case ACCEPTANCE_RATE:
            return {(float)acceptanceRate};
            case NUMBER_OF_M_C_STEPS:
            return {(int)numberOfMCSteps};
            case ALPHA_BRIGHTNESS:
            return {(float)alphaBrightness};
            case COLOR_OF_SAMPLES1:
            return {(Vec3)colorOfSamples1};
            case COLOR_OF_SAMPLES2:
            return {(Vec3)colorOfSamples2};
            case SHOW_NORMAL_COORD_SAMPLES:
            return {(bool)showNormalCoordSamples};
            case COLOR_PHASE:
            return {(bool)colorPhase};
            case MODES_BRIGHTNESS:
            return {(float)modesBrightness};
            case USE_COHERENT_STATES:
            return {(bool)useCoherentStates};
            case USE_SQUEEZED:
            return {(bool)useSqueezed};
            case USE_STATIONARY:
            return {(bool)useStationary};
            case USE_SINGLE_EXCITATIONS:
            return {(bool)useSingleExcitations};
            case SQUEEZED_FACTOR_GLOBAL:
            return {(float)squeezedFactorGlobal};
            case SQUEEZED_FACTOR:
            return {(float)squeezedFactor};
            case ADD_ENERGY:
            return {(bool)addEnergy};
            case REMOVE_ENERGY:
            return {(bool)removeEnergy};
        }
        return Uniform(0);
    }
    void set(int enum_val, int index, std::string val) {
        switch(enum_val) {
            case LABEL_MONTE_CARLO:
            labelMonteCarlo = val;
            break;
            case ACCEPTANCE_RATE_LABEL:
            acceptanceRateLabel = val;
            break;
            case LABEL_SAMPLES:
            labelSamples = val;
            break;
            case LABEL_NORMAL_MODE_WAVE_FUNC:
            labelNormalModeWaveFunc = val;
            break;
            case WAVE_FUNC_CONFIG_LABEL:
            waveFuncConfigLabel = val;
            break;
            case NOTE_FOR_USE_SINGLE_EXCITATIONS:
            noteForUseSingleExcitations = val;
            break;
            case COHERENT_OR_SQUEEZED_SELECTED_LABEL:
            coherentOrSqueezedSelectedLabel = val;
            break;
            case SQUEEZED_SELECTED_LABEL:
            squeezedSelectedLabel = val;
            break;
            case SQUEEZED_STATE_REL_ST_DEV_LABEL:
            squeezedStateRelStDevLabel = val;
            break;
            case ENERGY_EIGENSTATES_SELECTED_LABEL:
            energyEigenstatesSelectedLabel = val;
            break;
            case DISPERSION_OPTIONS_LABEL:
            DispersionOptionsLabel = val;
            break;
        }
    }
};
#endif
}
