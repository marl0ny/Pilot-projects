#include "gl_wrappers.hpp"


#ifndef _TRAJECTORIES_WIRE_FRAME_
#define _TRAJECTORIES_WIRE_FRAME_

namespace trajectories_wire_frame {

    enum {
        LINES_WITH_ZERO_ENDPOINTS, LINES_PERIODIC,
        LINES_NO_ENDPOINTS, DISCONNECTED_LINES
    };

    WireFrame get(
        int total_trajectory_count, int n_oscillators, int view_type);

};

#endif