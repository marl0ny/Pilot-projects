SHELL = /bin/bash
FLAGS = -O3 -g
C_COMPILE = clang
CPP_COMPILE = clang++ -std=c++11
LINKER = /usr/bin/ld

ifeq ($(shell uname),Darwin)
INCLUDE =  -I${PWD} -I${PWD}/imgui -I${PWD}/gl_wrappers -I/opt/homebrew/include
LIBS = -ldl -L/opt/homebrew/lib -lglfw\
       -framework CoreVideo -framework OpenGL -framework IOKit\
       -framework Cocoa -framework Carbon -lpng
else
INCLUDE =  -I${PWD} -I${PWD}/imgui -I${PWD}/gl_wrappers
LIBS = -lm -lGL -lGLEW -lglfw -lpng
endif

IMGUI_DIR = ${PWD}/imgui
IMGUI_SOURCES = ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp \
                ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp \
                ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp \
                ${IMGUI_DIR}/imgui_tables.cpp \
                ${IMGUI_DIR}/imgui_widgets.cpp
IMGUI_OBJECTS = imgui.o imgui_draw.o imgui_impl_glfw.o imgui_impl_opengl3.o \
                imgui_tables.o imgui_widgets.o

# Make sure to source <emcc_location>/emsdk/emsdk_env.sh first!
WEB_TARGET = main.js

TARGET = ${PWD}/program
DATA_DEPENDENCIES = parameters.json
GENERATION_SCRIPTS = make_parameter_files.py
GENERATED_DEPENDENCIES = parameters.hpp
C_SOURCES =
CPP_SOURCES = main.cpp simulation_pilot.cpp simulation.cpp \
	gl_wrappers.cpp glfw_window.cpp \
	interactor.cpp configs_view.cpp trajectories_wire_frame.cpp \
	harmonic.cpp metropolis.cpp histogram.cpp\
	initial_normal_mode_wave_function.cpp multidimensional_harmonic.cpp \
	write_to_png.cpp parse.cpp orthogonal_transforms.cpp
SOURCES = ${C_SOURCES} ${CPP_SOURCES}
OBJECTS = main.o simulation_pilot.o simulation.o \
	gl_wrappers.o glfw_window.o \
	interactor.o configs_view.o trajectories_wire_frame.o \
	harmonic.o metropolis.o histogram.o \
	initial_normal_mode_wave_function.o multidimensional_harmonic.o \
	write_to_png.o parse.o orthogonal_transforms.o
# SHADERS = ./shaders/*


all: ${TARGET}

${TARGET}: ${OBJECTS} ${IMGUI_OBJECTS}
	${CPP_COMPILE} ${FLAGS} -o $@ ${OBJECTS} ${IMGUI_OBJECTS} ${LIBS}

${WEB_TARGET}: ${SOURCES} ${GENERATED_DEPENDENCIES}
	emcc -lembind -o $@ ${SOURCES} ${INCLUDE} -O3 -v -s WASM=2 -s USE_GLFW=3 -s FULL_ES3=1 \
	-s TOTAL_MEMORY=500MB -s LLD_REPORT_UNDEFINED --embed-file shaders

${IMGUI_SOURCES}:
	wget "https://github.com/ocornut/imgui/archive/refs/heads/master.zip"
	unzip master.zip
	mv ./imgui-master ./imgui

${IMGUI_OBJECTS}: ${IMGUI_SOURCES}
	${CPP_COMPILE} ${FLAGS} -c $^ ${INCLUDE}

${OBJECTS}: ${CPP_SOURCES} ${GENERATED_DEPENDENCIES} ${IMGUI_OBJECTS}
	${CPP_COMPILE} ${FLAGS} -c ${CPP_SOURCES} ${INCLUDE}

${GENERATED_DEPENDENCIES}: ${DATA_DEPENDENCIES} ${GENERATION_SCRIPTS}
	python3 make_parameter_files.py

clean:
	rm -f *.o ${TARGET} *.wasm *.js
