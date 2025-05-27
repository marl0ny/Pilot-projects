#include <vector>

#ifndef _ORTHOGONAL_TRANSFORMS_
#define _ORTHOGONAL_TRANSFORMS_

void make_dst(std::vector<double> &dst, std::vector<double> &idst, int n);

void make_dsct(std::vector<double> &dsct, std::vector<double> &idsct, int n);

void make_dst(std::vector<float> &dst, std::vector<float> &idst, int n);

void make_dsct(std::vector<float> &dsct, std::vector<float> &idsct, int n);

std::vector<double> apply_transform(
    const std::vector<double> &transform, const std::vector<double> &x);

#endif