#include "configs_view.hpp"


static std::vector<float> get_vertices_set_elements(
    const std::vector<double> &configs, std::vector<int> &elements,
    int number_of_configs
) {
    int elem_count = 0;
    std::vector<float> vertices {};
    int row_size = configs.size()/number_of_configs;
    for (int row_count = 0; row_count < number_of_configs; row_count++) {
        for (int column_count = 0; column_count < row_size;
             column_count++) {
            float x_pos = -1.0 + 2.0*(column_count + 0.5F)/float(row_size);
            float y_pos = (float)configs[row_count*row_size + column_count];
            vertices.push_back(x_pos);
            vertices.push_back(y_pos);
            if (column_count > 0) {
                elements.push_back(elem_count-1);
                elements.push_back(elem_count);
            }
            elem_count++;
        }
    }
    return vertices;
}

static std::vector<float> get_vertices_set_elements_with_disconnected(
    const std::vector<double> &configs, std::vector<int> &elements,
    int number_of_configs
) {
    int elem_count = 0;
    std::vector<float> vertices {};
    int row_size = configs.size()/number_of_configs;
    for (int row_count = 0; row_count < number_of_configs; row_count++) {
        for (int column_count = 0; column_count < row_size;
             column_count++) {
            float x_pos = -1.0 + 2.0*(column_count + 0.26)/float(row_size);
            float y_pos = (float)configs[row_count*row_size + column_count];
            vertices.push_back(x_pos);
            vertices.push_back(y_pos);
            elem_count++;
            x_pos = -1.0 + 2.0*(column_count + 0.74)/float(row_size);
            y_pos = (float)configs[row_count*row_size + column_count];
            vertices.push_back(x_pos);
            vertices.push_back(y_pos);
            elements.push_back(elem_count-1);
            elements.push_back(elem_count);
            elem_count++;
        }
    }
    return vertices;
}

static std::vector<float> get_vertices_set_elements_with_endpoints(
    const std::vector<double> &configs, std::vector<int> &elements,
    int number_of_configs
) {
    int elem_count = 0;
    std::vector<float> vertices {};
    int row_size = configs.size()/number_of_configs;
    for (int row_count = 0; row_count < number_of_configs; row_count++) {
        vertices.push_back(-1.0);
        vertices.push_back(0.0);
        // elements.push_back(elem_count);
        elem_count++;
        for (int column_count = 0; column_count < row_size;
             column_count++) {
            float x_pos = -1.0 + 2.0*(column_count + 1)/float(row_size + 1);
            float y_pos = (float)configs[row_count*row_size + column_count];
            vertices.push_back(x_pos);
            vertices.push_back(y_pos);
            elements.push_back(elem_count-1);
            elements.push_back(elem_count);
            elem_count++;
        }
        vertices.push_back(1.0);
        vertices.push_back(0.0);
        elements.push_back(elem_count-1);
        elements.push_back(elem_count);
        elem_count++;
    }
    return vertices;
}

static std::vector<float> get_vertices_set_elements_periodic(
    const std::vector<double> &configs, std::vector<int> &elements,
    int number_of_configs
) {
    int elem_count = 0;
    std::vector<float> vertices {};
    int row_size = configs.size()/number_of_configs;
    for (int row_count = 0; row_count < number_of_configs; row_count++) {
        vertices.push_back(-1.0);
        vertices.push_back((float)configs[row_count*row_size + row_size -1]);
        // elements.push_back(elem_count);
        elem_count++;
        for (int column_count = 0; column_count < row_size;
             column_count++) {
            float x_pos = -1.0 + 2.0*(column_count + 1)/float(row_size + 1);
            float y_pos = (float)configs[row_count*row_size + column_count];
            vertices.push_back(x_pos);
            vertices.push_back(y_pos);
            elements.push_back(elem_count-1);
            elements.push_back(elem_count);
            elem_count++;
        }
        vertices.push_back(1.0);
        vertices.push_back((float)configs[row_count*row_size]);
        elements.push_back(elem_count-1);
        elements.push_back(elem_count);
        elem_count++;
    }
    return vertices;
}

WireFrame configs_view::get_configs_view_wire_frame(
    const std::vector<double> &configs, int number_of_configs, int view_type
) {
    std::vector<int> elements {};
    std::vector<float> vertices;
    switch(view_type) {
        case LINES_NO_ENDPOINTS:
        vertices = get_vertices_set_elements(
            configs, elements, number_of_configs);
        break;
        case LINES_WITH_ZERO_ENDPOINTS:
        vertices = get_vertices_set_elements_with_endpoints(
            configs, elements, number_of_configs);
        break;
        case LINES_PERIODIC:
        vertices = get_vertices_set_elements_periodic(
            configs, elements, number_of_configs);
        break;
        case DISCONNECTED_LINES:
        vertices = get_vertices_set_elements_with_disconnected(
            configs, elements, number_of_configs);
        break;
        default:
        vertices = get_vertices_set_elements(
            configs, elements, number_of_configs);
        break;
    }
    Attributes attributes = {
        {"position", {
                .size=2, .type=GL_FLOAT, .normalized=false,
                .stride=0, .offset=0}}};
    return WireFrame(
        attributes, vertices, elements, WireFrame::LINES
    );
}
