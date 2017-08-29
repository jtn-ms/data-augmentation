#pragma once

#include <opencv/cv.h>  

//integral projection
void calcHorizontalProjection(cv::Mat thresh_output, int* sum_h, int value);
void calcVerticalProjection(cv::Mat thresh_output, int* sum_v, int value);
//dealing with 
int calcMax(int* array_, int arraySize);
int calcMin(int* array_, int arraySize);
int getFirstIndex(int* array_, int arraySize, int value);
int getLastIndex(int* array_, int arraySize, int value);
void calcSize(int* array_, int arraySize, int& firstindex, int& lastindex);
void findTopBottom(int* array_x, int rows, int& top, int& bottom);
void findTopBottom_bottom2up(int* array_x, int rows, int& top, int& bottom);
void findLeftRight(int* array_y, int cols, int& left, int& right);
void findLeftRight_revised(int* array_y, int cols, int& first_left, int& first_right, int& second_left, int &second_right, bool isMessage);
///
void showArray_h(char* title, int* array_h, int arraySize);
void showArray_v(char* title, int* array_v, int arraySize);

void eliminateLowValueRegion(int* array_, int arraySize, int thr);

void blurArray(int* array_t, int arraySize, int kernelsize);