#ifndef CELLSORTER_MATLAB_EXT_H
#define CELLSORTER_MATLAB_EXT_H

#include <iterator>
#include <opencv/cv.hpp>
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <iostream>

#include "datacontainer.h"

namespace mathlab {
enum regionPropTypes {
    Area = data::Area,
    BoundingBox = data::BoundingBox,
    Centroid = data::Centroid,
    Circularity = data::Circularity,
    ConvexArea = data::ConvexArea,
    Eccentricity = data::Eccentricity,
    Major_axis = data::Major_axis,
    Minor_axis = data::Minor_axis,
    Solidity = data::Solidity,
    Symmetry = data::Symmetry,
    Perimeter = data::Perimeter,
    PixelIdxList = data::PixelIdxList
};

void bwareaopen(cv::Mat& im, double size);

int regionProps(const cv::Mat& img, const int& dataFlags, DataContainer& dc);
double gradientScore(const cv::Mat& img, const cv::Point& centroid);


void removePixels(cv::Mat img, std::vector<cv::Point>* points);

void floodFill(cv::Mat& img);
void floodFill(cv::Mat& img, const cv::Point& p);

double dist(const cv::Point& p0, const cv::Point& p1);

template <typename T>
std::pair<T, unsigned long> min(const std::vector<T>& v) {
    std::pair<T, unsigned long> output;
    auto m = std::min_element(std::begin(v), std::end(v));
    output.first = *m;
    output.second = (unsigned)std::distance(std::begin(v), m);
    return output;
}

template <typename T>
std::pair<T, unsigned long> max(const std::vector<T>& v) {
    std::pair<T, unsigned long> output;
    auto m = std::max_element(std::begin(v), std::end(v));
    output.first = *m;
    output.second = (unsigned)std::distance(std::begin(v), m);
    return output;
}

template <typename T>
std::vector<T> find(std::vector<T>& v, T& term) {
    std::vector<T> out;
    for (T& t : v) {
        if (t == term) {
            out.push_back(t);
        }
    }
    return out;
}

}  // namespace mathlab

#endif  // CELLSORTER_MATLAB_EXT_H