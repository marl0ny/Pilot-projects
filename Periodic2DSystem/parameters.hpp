#include "gl_wrappers.hpp"

namespace sim_2d {

#ifndef _PARAMETERS_
#define _PARAMETERS_

struct Button {};

struct UploadImage {};

typedef std::string Label;

typedef bool BoolRecord;

struct BMPRecord {
    bool is_recording;
    int width, height;
};

typedef std::vector<std::string> EntryBoxes;

struct SelectionList {
    int selected;
    std::vector<std::string> options;
};

struct LineDivider {};

struct NotUsed {};

struct SimParams {
    float t = (float)(0.0F);
    int stepsPerFrame = (int)(6);
    float brightness = (float)(1.0F);
    float brightnessV = (float)(1.0F);
    float brightnessParticles = (float)(1.0F);
    float hbar = (float)(1.0F);
    float waveFuncSize = (float)(0.025F);
    float m = (float)(1.0F);
    float dt = (float)(0.3F);
    SelectionList mouseUsageEntry = SelectionList{0, {"Create new wave function", "Draw potential barrier", "Erase potential barrier"}};
    int numberOfParticles = (int)(65536);
    bool showTrails = (bool)(false);
    LineDivider lineDiv = LineDivider{};
    Label sliderSetWaveFuncTitle = Label{};
    Vec2 sliderNewWaveFuncMomentum = (Vec2)(Vec2 {.ind={0.0, 40.0}});
    Vec2 sliderNewWaveFuncPosition = (Vec2)(Vec2 {.ind={128.0, 128.0}});
    Button enterWaveFunc = Button{};
    LineDivider lineDiv2 = LineDivider{};
    IVec2 waveDiscretizationDimensions = (IVec2)(IVec2 {.ind={512, 512}});
    int potentialGridWidth = (int)(512);
    int potentialGridHeight = (int)(512);
    Vec2 waveSimulationDimensions = (Vec2)(Vec2 {.ind={512.0, 512.0}});
    SelectionList presetPotentialDropdown = SelectionList{0, {"((x/width)^2 + (y/height)^2)", "0", "amp*((x/width)^2 + (y/height)^2)", "0.4*(step(-y^2+(height*0.04)^2)+step(y^2-(height*0.06)^2))*step(-x^2+(width*0.01)^2)", "1.0/sqrt(x^2+y^2)+1.0/sqrt((x-0.25*width)^2+(y-0.25*height)^2)", "(x*cos(w*t/200) + y*sin(w*t/200))/500+0.01", "0.5*(tanh(75.0*(((x/width)^2+(y/height)^2)^0.5-0.45))+1.0)"}};
    EntryBoxes userTextEntry = EntryBoxes{"0"};
    Label userWarningLabel = Label{};
    bool addAbsorbingBoundaries = (bool)(false);
    UploadImage imagePotential = UploadImage{};
    BMPRecord takeScreenshots = BMPRecord{false, 2160, 2160};
    int dummyValue = (int)(0);
    enum {
        T=0,
        STEPS_PER_FRAME=1,
        BRIGHTNESS=2,
        BRIGHTNESS_V=3,
        BRIGHTNESS_PARTICLES=4,
        HBAR=5,
        WAVE_FUNC_SIZE=6,
        M=7,
        DT=8,
        MOUSE_USAGE_ENTRY=9,
        NUMBER_OF_PARTICLES=10,
        SHOW_TRAILS=11,
        LINE_DIV=12,
        SLIDER_SET_WAVE_FUNC_TITLE=13,
        SLIDER_NEW_WAVE_FUNC_MOMENTUM=14,
        SLIDER_NEW_WAVE_FUNC_POSITION=15,
        ENTER_WAVE_FUNC=16,
        LINE_DIV2=17,
        WAVE_DISCRETIZATION_DIMENSIONS=18,
        POTENTIAL_GRID_WIDTH=19,
        POTENTIAL_GRID_HEIGHT=20,
        WAVE_SIMULATION_DIMENSIONS=21,
        PRESET_POTENTIAL_DROPDOWN=22,
        USER_TEXT_ENTRY=23,
        USER_WARNING_LABEL=24,
        ADD_ABSORBING_BOUNDARIES=25,
        IMAGE_POTENTIAL=26,
        TAKE_SCREENSHOTS=27,
        DUMMY_VALUE=28,
    };
    void set(int enum_val, Uniform val) {
        switch(enum_val) {
            case T:
            t = val.f32;
            break;
            case STEPS_PER_FRAME:
            stepsPerFrame = val.i32;
            break;
            case BRIGHTNESS:
            brightness = val.f32;
            break;
            case BRIGHTNESS_V:
            brightnessV = val.f32;
            break;
            case BRIGHTNESS_PARTICLES:
            brightnessParticles = val.f32;
            break;
            case HBAR:
            hbar = val.f32;
            break;
            case WAVE_FUNC_SIZE:
            waveFuncSize = val.f32;
            break;
            case M:
            m = val.f32;
            break;
            case DT:
            dt = val.f32;
            break;
            case NUMBER_OF_PARTICLES:
            numberOfParticles = val.i32;
            break;
            case SHOW_TRAILS:
            showTrails = val.b32;
            break;
            case SLIDER_NEW_WAVE_FUNC_MOMENTUM:
            sliderNewWaveFuncMomentum = val.vec2;
            break;
            case SLIDER_NEW_WAVE_FUNC_POSITION:
            sliderNewWaveFuncPosition = val.vec2;
            break;
            case WAVE_DISCRETIZATION_DIMENSIONS:
            waveDiscretizationDimensions = val.ivec2;
            break;
            case POTENTIAL_GRID_WIDTH:
            potentialGridWidth = val.i32;
            break;
            case POTENTIAL_GRID_HEIGHT:
            potentialGridHeight = val.i32;
            break;
            case WAVE_SIMULATION_DIMENSIONS:
            waveSimulationDimensions = val.vec2;
            break;
            case ADD_ABSORBING_BOUNDARIES:
            addAbsorbingBoundaries = val.b32;
            break;
            case DUMMY_VALUE:
            dummyValue = val.i32;
            break;
        }
    }
    Uniform get(int enum_val) const {
        switch(enum_val) {
            case T:
            return {(float)t};
            case STEPS_PER_FRAME:
            return {(int)stepsPerFrame};
            case BRIGHTNESS:
            return {(float)brightness};
            case BRIGHTNESS_V:
            return {(float)brightnessV};
            case BRIGHTNESS_PARTICLES:
            return {(float)brightnessParticles};
            case HBAR:
            return {(float)hbar};
            case WAVE_FUNC_SIZE:
            return {(float)waveFuncSize};
            case M:
            return {(float)m};
            case DT:
            return {(float)dt};
            case NUMBER_OF_PARTICLES:
            return {(int)numberOfParticles};
            case SHOW_TRAILS:
            return {(bool)showTrails};
            case SLIDER_NEW_WAVE_FUNC_MOMENTUM:
            return {(Vec2)sliderNewWaveFuncMomentum};
            case SLIDER_NEW_WAVE_FUNC_POSITION:
            return {(Vec2)sliderNewWaveFuncPosition};
            case WAVE_DISCRETIZATION_DIMENSIONS:
            return {(IVec2)waveDiscretizationDimensions};
            case POTENTIAL_GRID_WIDTH:
            return {(int)potentialGridWidth};
            case POTENTIAL_GRID_HEIGHT:
            return {(int)potentialGridHeight};
            case WAVE_SIMULATION_DIMENSIONS:
            return {(Vec2)waveSimulationDimensions};
            case ADD_ABSORBING_BOUNDARIES:
            return {(bool)addAbsorbingBoundaries};
            case DUMMY_VALUE:
            return {(int)dummyValue};
        }
        return Uniform(0);
    }
    void set(int enum_val, int index, std::string val) {
        switch(enum_val) {
            case SLIDER_SET_WAVE_FUNC_TITLE:
            sliderSetWaveFuncTitle = val;
            break;
            case USER_TEXT_ENTRY:
            userTextEntry[index] = val;
            break;
            case USER_WARNING_LABEL:
            userWarningLabel = val;
            break;
        }
    }
};
#endif
}
