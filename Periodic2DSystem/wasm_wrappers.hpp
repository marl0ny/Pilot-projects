#include "gl_wrappers.hpp"

#include <functional>
#include <set>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
using namespace emscripten;
#endif

static std::function<void(int, Uniform)> s_sim_params_set;
static std::function<void(int, int, std::string)> s_sim_params_set_string;
static std::function<Uniform(int)> s_sim_params_get;
static std::function<void(int, std::string, float)> s_user_edit_set_value;
static std::function<float(int, std::string)> s_user_edit_get_value;
static std::function<std::string(int)>
    s_user_edit_get_comma_separated_variables;
static std::function<void(int)> s_button_pressed;
static std::function<void(int, int)> s_selection_set;
static std::function<void(
    int, const std::string &image_data, int, int)> s_image_set;
static std::function<unsigned char *()> s_bmp_image;
static std::function<unsigned int ()> s_bmp_image_size;
static std::function<void (int, bool)> s_configure_bmp_recording;
static std::function<void(int, std::string, float)>
    s_sim_params_set_user_float_param;


void edit_label_display(int c, std::string text_content) {
    std::string string_val = "";
    string_val += "editLabel(";
    string_val += std::to_string(c);
    string_val += ", ";
    string_val += "\"" + text_content + "\"";
    string_val += ");";
    #ifdef __EMSCRIPTEN__
    emscripten_run_script(&string_val[0]);
    #endif
}

void display_parameters_as_sliders(
    int c, std::set<std::string> variables, std::set<std::string> do_not_show={}) {
    for (auto &e: do_not_show)
        variables.erase(e);
    std::string string_val = "[";
    for (auto &e: variables)
        string_val += "\"" + e + "\", ";
    string_val += "]";
    string_val 
        = "modifyUserSliders(" + std::to_string(c) + ", " + string_val + ");";
    printf("%s\n", &string_val[0]);
    #ifdef __EMSCRIPTEN__
    emscripten_run_script(&string_val[0]);
    #endif
}


/* Setters for the simulation parameters struct, where they act
as the exposed entry point for JavaScript code in the WASM build.
There are multiple functions, one for each type. They all take as the first
argument a param_code representing each field of the parameter struct,
where these codes must be written and enumerated separately in JavaScript.
For the function setters of scalar quantities, the next and final argument is
just the quantity itself. For those that set a vector quantity,
these next arguments in order must be passed into the function: 
the number of elements the vector contains, the index of the vector to change
the value, and lastly the value itself. The actual vector structs
themselves are not passed as argument: this is to avoid the complexity of 
getting non-primitive objects to be passed between JS/C++.
*/

void set_int_param(int param_code, int i) {
    s_sim_params_set(param_code, Uniform((int)i));
}

int get_int_param(int param_code) {
    Uniform u = s_sim_params_get(param_code);
    return u.i32;
}

void set_float_param(int param_code, float f) {
    s_sim_params_set(param_code, Uniform((float)f));
}

void set_bool_param(int param_code, bool b) {
    s_sim_params_set(param_code, Uniform((bool)b));
}

void set_string_param(int param_code, int index, std::string s) {
    s_sim_params_set_string(param_code, index, s);
}

void set_user_float_param(int param_code, std::string s, float value) {
    s_sim_params_set_user_float_param(param_code, s, value);
}

// std::string send_json_string() {
//     return {"This", "is", "some", "text\n";
// }

float user_edit_get_value(int div_code, std::string variable_name) {
    return s_user_edit_get_value(div_code, variable_name);
}

void user_edit_set_value(int div_code, std::string variable_name, float value) {
    s_user_edit_set_value(div_code, variable_name, value);
}

std::string user_edit_get_comma_separated_variables(int div_code) {
    return s_user_edit_get_comma_separated_variables(div_code);
}

void set_vec_param(int param_code, int elem_count, int index, float val) {
    auto u = s_sim_params_get(param_code);
    if (elem_count == 2) {
        u.vec2[index] = val;
    } else if (elem_count == 3) {
        u.vec3[index] = val;
    } else {
        u.vec4[index] = val;
    }
    s_sim_params_set(param_code, u);
}

void set_ivec_param(int param_code, int elem_count, int index, float val) {
    auto u = s_sim_params_get(param_code);
    if (elem_count == 2) {
        u.ivec2[index] = val;
    } else if (elem_count == 3) {
        u.ivec3[index] = val;
    } else {
        u.ivec4[index] = val;
    }
    s_sim_params_set(param_code, u);
}

void button_pressed(int param_code) {
    s_button_pressed(param_code);
}

void selection_set(int param_code, int value) {
    s_selection_set(param_code, value);
}

void image_set(
    int param_code, const std::string &image_data,
    int width, int height) {
    s_image_set(param_code, image_data, width, height);
}

/* bmp_image is called in JavaScript to get the bmp image data.
This must further call s_bmp_image_size and s_bmp_image, which
are lambda functions that must be implemented in the source files.
s_bmp_image gives the actual bmp image data (including its header),
and s_bmp_image_size must give its size. 

An additional function configure_bmp_recording is also called in JavaScript
to notify when the bmp recording checkbox has been checked or not.
This calls the function s_configure_bmp_recording, which must be implemented
in the source file, where this is used to modify 
the BMPRecord parameter in the SimParams instantiation.

Finally, the function download_bmp_image runs JavaScript code that
calls bmp_image to download the bmp image. This must be called from
main looping function in the main source file, after the draw calls.

So here is an outline to get bmp image screenshots to work:
 - Add a parameter of type BMPRecord in the json file
 - Implement s_configure_bmp_recording lambda function that
   sets the BMPRecord's parameter field is_recording using
   this function's is_recording parameter.
 - In the Simulation class, implement a member that stores the
   image data. In the Simulation class's view method, 
   access the BMPRecord field from the view method's SimParams
   parameter. If the BMPRecord's is_recording field is set to true,
   get the render image data and copy it to that member that stores
   the image data. Fill out the BMP header
   as well and store it at the front of the image data.
 - Implement the lambda functions s_bmp_image, s_bmp_image_size.
   The s_bmp_image should return a pointer to the start of
   the image data from the Simulation object, and s_bmp_image_size
   should return its size in bytes.
 - In the main loop function, if the BMPRecord's is_recording field
   is true, call download_bmp_image.

*/
#ifdef __EMSCRIPTEN__
val bmp_image() {
    return val(typed_memory_view(s_bmp_image_size(), s_bmp_image()));
}
#endif

void download_bmp_image(std::string postfix_name) {
    std::string js_code1 = R"(
    let imageData = Module.bmp_image();
    let blob = new Blob([imageData], {type: "octet/stream"});
    let aTag = document.createElement('a');
    let url = URL.createObjectURL(blob);
    aTag.hidden = true;
    aTag.href = url;
    let time = Date.now();
    aTag.download = `${time}-)";
    std::string js_code2 = R"(.bmp`;
    new Promise(() => aTag.click()).then(() => aTag.remove());
    )";
    std::string js_code = js_code1 + postfix_name + js_code2;
    #ifdef __EMSCRIPTEN__
    emscripten_run_script(&js_code[0]);
    #endif
}

void configure_bmp_recording(int param_code, bool b) {
    s_configure_bmp_recording(param_code, b);
}

// void set_mouse_mode(int type) {
//     s_input_type = type;
// }

void start_gui(void *window) {
}

void display_gui(void *data) {

}

bool outside_gui() {
    return true;
}

#ifdef __EMSCRIPTEN__

EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::function("set_float_param", set_float_param);
    emscripten::function("set_int_param", set_int_param);
    emscripten::function("get_int_param", get_int_param);
    emscripten::function("set_bool_param", set_bool_param);
    emscripten::function("set_vec_param", set_vec_param);
    emscripten::function("set_ivec_param", set_ivec_param);
    emscripten::function("set_string_param", set_string_param);
    emscripten::function("user_edit_get_value", user_edit_get_value);
    emscripten::function("user_edit_set_value", user_edit_set_value);
    emscripten::function("user_edit_get_comma_separated_variables",
             user_edit_get_comma_separated_variables);
    emscripten::function("button_pressed", button_pressed);
    emscripten::function("selection_set", selection_set);
    emscripten::function("image_set", image_set, allow_raw_pointers());
    emscripten::function("bmp_image", bmp_image);
    emscripten::function("configure_bmp_recording", configure_bmp_recording);
    emscripten::function("set_user_float_param", set_user_float_param);
}
#endif
