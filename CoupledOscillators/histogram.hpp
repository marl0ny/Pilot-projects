#include "gl_wrappers.hpp"
#include <vector>

#ifndef _HISTOGRAM_
#define _HISTOGRAM_

namespace histogram {

struct Histogram2D {
    IVec2 dimensions;
    Vec2 min_val, range;
    std::vector<float> arr;
};
    
void add_data_point(
    histogram::Histogram2D &hist, double x, double y, double val);

}

#endif