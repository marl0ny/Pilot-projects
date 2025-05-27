#include "histogram.hpp"


void histogram::add_data_point(
    histogram::Histogram2D &hist, double x, double y, double val) {
    // x = min_val.x + range*(i/dimensions.x);
    // y = min_val.y + range*(j/dimensions.y);
    int i = double(hist.dimensions.x)*(x - hist.min_val.x)/hist.range.x;
    int j = double(hist.dimensions.y)*(y - hist.min_val.y)/hist.range.y;
    int ind = j*hist.dimensions[0] + i;
    if (ind >= 0 && ind < hist.arr.size())
        hist.arr[ind] += val;
}
