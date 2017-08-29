#pragma once

#include <opencv/cv.h>  

cv::Mat pad(cv::Mat);
cv::Mat Rotation(cv::Mat in, double angle = 0.0, double scale = 1.0);
cv::Mat getMask(cv::Mat);
cv::Mat threshold_color(cv::Mat);