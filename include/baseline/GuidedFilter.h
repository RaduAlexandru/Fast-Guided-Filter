#pragma once

#include <iostream>

// class GuidedFilter{
// public:
//   GuidedFilter();
// };


#include <opencv2/opencv.hpp>

class GuidedFilterImpl;

class GuidedFilter
{
public:
    GuidedFilter(const cv::Mat &I, int r, double eps, int scale);
    ~GuidedFilter();

    cv::Mat filter(const cv::Mat &p, int depth = -1) const;

private:
    GuidedFilterImpl *impl_;
    int scale;
};

cv::Mat guidedFilter(const cv::Mat &I, const cv::Mat &p, int r, double eps, int depth = -1);
