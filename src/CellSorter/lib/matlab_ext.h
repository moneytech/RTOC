#ifndef CELLSORTER_MATLAB_EXT_H
#define CELLSORTER_MATLAB_EXT_H

#include <opencv/cv.hpp>
#include <iterator>
#include <cmath>
#include <iostream>
#include <algorithm>

#include "datacontainer.h"

namespace matlab {
enum regionPropTypes {
    Area = data::Area,
    BoundingBox = data::BoundingBox,
    Centroid = data::Centroid,
    Circularity = data::Circularity,
    ConvexArea = data::ConvexArea,
    Eccentricity = data::Eccentricity,
    GradientScore = data::GradientScore,
    Major_axis = data::Major_axis,
    Minor_axis = data::Minor_axis,
    Solidity = data::Solidity,
    Symmetry = data::Symmetry,
    Perimeter = data::Perimeter,
    PixelIdxList = data::PixelIdxList
};

void bwareaopen(cv::Mat& im, double size);

int regionProps(const cv::Mat& img, const int& dataFlags, DataContainer& dc);

void removePixels(cv::Mat img, std::vector<cv::Point>* points);

void floodFill(cv::Mat& img);
void floodFill(cv::Mat& img, const cv::Point& p);

double dist(const cv::Point& p0, const cv::Point& p1);

template <typename T>
std::pair<T,long> min(std::vector<T>& v) {
    std::pair<T,long> output;
    auto m = std::min_element(std::begin(v), std::end(v));
    output.first = *m;
    output.second = std::distance(std::begin(v), m);
    return output;
}

template <typename T>
std::pair<T,long> max(std::vector<T>& v) {
    std::pair<T,long> output;
    auto m = std::max_element(std::begin(v), std::end(v));
    output.first = *m;
    output.second = std::distance(std::begin(v), m);
    return output;
}


}  // namespace matlab

#endif  // CELLSORTER_MATLAB_EXT_H
