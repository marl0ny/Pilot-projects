#include "trajectories_wire_frame.hpp"

static std::vector<float> get_vertices_set_elements(
    std::vector<int> &elements, int n_oscillators, int total_count) {
    int elem_count = 0;
    std::vector<float> vertices {};
    int row_size = n_oscillators;
    for (int row_count = 0; row_count < total_count; row_count++) {
        for (int column_count = 0; column_count < row_size;
            column_count++) {
            vertices.push_back(float(column_count));
            vertices.push_back(float(row_count));
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
    std::vector<int> &elements, int n_oscillators, int total_count) {
    int elem_count = 0;
    std::vector<float> vertices {};
    int row_size = n_oscillators;
    for (int row_count = 0; row_count < total_count; row_count++) {
        for (int column_count = 0; column_count < row_size; column_count++) {
            vertices.push_back((float)column_count - 0.24);
            vertices.push_back((float)row_count);
            elem_count++;
            vertices.push_back((float)column_count + 0.24);
            vertices.push_back((float)row_count);
            elements.push_back(elem_count-1);
            elements.push_back(elem_count);
            elem_count++;
        }
    }
    return vertices;

}

static std::vector<float> get_vertices_set_elements_with_endpoints(
    std::vector<int> &elements, int n_oscillators, int total_count) {
    int elem_count = 0;
    std::vector<float> vertices {};
    int row_size = n_oscillators;
    for (int row_count = 0; row_count < total_count; row_count++) {
        vertices.push_back(-1.0);
        vertices.push_back(row_count);
        elem_count++;
        for (int column_count = 0; column_count < row_size;
            column_count++) {
            vertices.push_back(float(column_count));
            vertices.push_back(float(row_count));
            elements.push_back(elem_count-1);
            elements.push_back(elem_count);
            elem_count++;
        }
        vertices.push_back(row_size);
        vertices.push_back(row_count);
        elements.push_back(elem_count-1);
        elements.push_back(elem_count);
        elem_count++;
    }
    return vertices;
}

WireFrame trajectories_wire_frame::get(
        int total_trajectory_count, int n_oscillators, int view_type) {
    std::vector<int> elements {};
    std::vector<float> vertices;
    switch(view_type) {
        case LINES_WITH_ZERO_ENDPOINTS: 
        case LINES_PERIODIC:
        vertices = get_vertices_set_elements_with_endpoints(
            elements, n_oscillators, total_trajectory_count);
        break;
        case DISCONNECTED_LINES:
        vertices = get_vertices_set_elements_with_disconnected(
            elements, n_oscillators, total_trajectory_count);
        break;
        default:
        vertices = get_vertices_set_elements(
            elements, n_oscillators, total_trajectory_count);
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
