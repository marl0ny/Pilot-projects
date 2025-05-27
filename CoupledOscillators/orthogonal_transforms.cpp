#include "orthogonal_transforms.hpp"
#include <math.h>

static const double PI = 3.141592653589793;

/* Get the elements of the type I DST transformation matrix.

The equations for this can be found in Scipy's documentation for the DST:

https://docs.scipy.org/doc/scipy/reference/generated/scipy.fft.dst.html

*/
static double dst_element(int i, int j, int n) {
    return 2.0*sin(PI*(i + 1)*(j + 1)/(n + 1))/sqrt(2.0*(n + 1));
}

/* Get the elements of the discrete sine cosine transformation
matrix. See equations 14-15 of Johnson and Gutierrez
to see them explicitly written out for the n=8 case.
*/
static double dsct_element(int i, int j, int n) {
    int k = (n % 2)? (j - n/2): (j - n/2 + 1);
    double norm_factor = (n % 2)?
        sqrt(((k != 0)? 2.0: 1.0)/n):
        sqrt(((k != 0 && j != n-1)? 2.0: 1.0)/n);
    return norm_factor*((k < 0)? -sin(2.0*PI*i*k/n): cos(2.0*PI*i*k/n));
}

void make_dst(std::vector<double> &dst, std::vector<double> &idst, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            dst[i*n + j] = dst_element(i, j, n);
            idst[j*n + i] = dst[i*n + j];
        }
    }
}

void make_dsct(std::vector<double> &dsct, std::vector<double> &idsct, int n) {
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            dsct[j*n + i] = dsct_element(i, j, n);
            idsct[i*n + j] = dsct[j*n + i];
        }
    }
}

void make_dst(std::vector<float> &dst, std::vector<float> &idst, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            dst[i*n + j] = dst_element(i, j, n);
            idst[j*n + i] = dst[i*n + j];
        }
    }
}

void make_dsct(std::vector<float> &dsct, std::vector<float> &idsct, int n) {
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            dsct[j*n + i] = dsct_element(i, j, n);
            idsct[i*n + j] = dsct[j*n + i];
        }
    }
}

std::vector<double> apply_transform(
    const std::vector<double> &transform, const std::vector<double> &x) {
    std::vector<double> res (x.size());
    for (int i = 0; i < x.size(); i++) {
        res[i] = 0.0;
        for (int j = 0; j < x.size(); j++) {
            res[i] += transform[i*x.size() + j]*x[i];
        }
    }
    return res;
}

