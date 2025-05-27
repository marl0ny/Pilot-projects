#include "gl_wrappers.hpp"

#ifndef _CONFIGS_VIEW_
#define _CONFIGS_VIEW_

namespace configs_view {

    enum {
        LINES_WITH_ZERO_ENDPOINTS, LINES_PERIODIC,
        LINES_NO_ENDPOINTS, DISCONNECTED_LINES
    };

    WireFrame get_configs_view_wire_frame(
        const std::vector<double> &vertices, int number_of_configs, 
        int view_type
    );

};

#endif