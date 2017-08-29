#include "stdafx.h"
#include "array.h"
#include <opencv2\highgui\highgui.hpp>
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
using namespace cv;
/////Integral Projection
void calcHorizontalProjection(Mat thresh_output, int* sum_h, int value)
{
	if (!thresh_output.data || !sum_h || value < 0 || value > 255) return;
	for (int i = 0; i< thresh_output.rows; i++)
	{
		sum_h[i] = 0;
		for (int j = 0; j < thresh_output.cols; j++)
		{
			if (*thresh_output.ptr(i, j) == value)
				sum_h[i]++;
		}
		if (sum_h[i] > thresh_output.cols * 0.5)
			sum_h[i] = 0;
	}
	return;
}
void calcVerticalProjection(Mat thresh_output, int* sum_v, int value)
{
	if (!thresh_output.data || !sum_v || value < 0 || value > 255) return;
	for (int i = 0; i< thresh_output.cols; i++)
	{
		sum_v[i] = 0;
		for (int j = 0; j < thresh_output.rows; j++)
		if (*thresh_output.ptr(j, i) == value)
			sum_v[i]++;
	}
	return;
}
//Blur
void eliminateLowValueRegion(int* array_, int arraySize, int thr)
{
	for (int i = 0; i < arraySize; i++)
	if (array_[i] < thr) array_[i] = 0;
}
/////
int calcMax(int* array_, int arraySize)
{
	int max = 0;
	for (int i = 0; i < arraySize; i++)
	if (array_[i]  > max) max = array_[i];
	return max;
}
int calcMin(int* array_, int arraySize)
{
	int min = calcMax(array_, arraySize);//2 * arraySize;
	for (int i = 0; i < arraySize; i++)
	if (array_[i] < min) min = array_[i];
	return min;
}
int getFirstIndex(int* array_, int arraySize, int value)
{
	for (int i = 1; i < arraySize; i++)
	if (array_[i - 1] >= value) return i - 1;
	else if (array_[i - 1] < value && array_[i] >= value) return i - 1;
	return 0;
}
int getLastIndex(int* array_, int arraySize, int value)
{
	for (int i = (arraySize - 2); i >= 0; i--)
	if (array_[i + 1] >= value) return i + 1;
	else if (array_[i] >= value && array_[i + 1] < value) return i + 1;
	return arraySize - 1;
}
int getFirstIndex_revised(int* array_, int arraySize, int value, int seed)
{
	for (int i = seed; i < arraySize; i++)
	if (array_[i - 1] >= value) return i - 1;
	else if (array_[i - 1] < value && array_[i] >= value) return i - 1;
	return 0;
}
/////
void calcSize(int* array_, int arraySize, int& firstindex, int& lastindex)
{
	//blur_(array_,arraySize);
	//Gradient_(array_,arraySize);
	int min = calcMin(array_, arraySize);
	min = min < 0 ? 0 : min;
	int max = calcMax(array_, arraySize);
	int mean = min;//min * 0.9 + max * 0.1;//calcMean(array_,arraySize);//(int)(float(max - min) / 1.4142);//
	int maxindex = getFirstIndex(array_, arraySize, max);//kjy-todo-2015.3.27
	//int oldfirstindex = getFirstIndex(array_,arraySize,mean - 1),oldlastindex = getLastIndex(array_,arraySize,mean - 1);

	for (int i = maxindex; i < arraySize - 1; i++)
	if ((array_[i] >= mean && array_[i + 1] <= mean) || i == arraySize - 2)
	{
		lastindex = i;
		break;
	}
	for (int j = maxindex; j > 0; j--)
	if ((array_[j] >= mean && array_[j - 1] <= mean) || j == 1)
	{
		firstindex = j;
		break;
	}
	return;
}
void calcSize_bottom2up(int* array_, int arraySize, int& firstindex, int& lastindex)
{
	//blur_(array_,arraySize);
	//Gradient_(array_,arraySize);
	int min = calcMin(array_, arraySize);
	min = min < 0 ? 0 : min;
	int max = calcMax(array_, arraySize);
	int mean = min;//min * 0.9 + max * 0.1;//calcMean(array_,arraySize);//(int)(float(max - min) / 1.4142);//
	int maxindex = getFirstIndex(array_, arraySize, max);//kjy-todo-2015.3.27
	//int oldfirstindex = getFirstIndex(array_,arraySize,mean - 1),oldlastindex = getLastIndex(array_,arraySize,mean - 1);

	lastindex = -1;
	for (int j = arraySize - 1; j > 0; j--)
	{
		if (array_[j] > mean && lastindex == -1)
		{
			lastindex = j;
		}
		if (lastindex != -1 && array_[j] == mean)
		{
			firstindex = j;
			break;
		}
	}
	return;
}
void findTopBottom(int* array_x, int rows, int& top, int& bottom)
{
	calcSize(array_x, rows, top, bottom);

	if (top < 0)
		top = 0;
	if (bottom >= rows)
		bottom = rows;
	if (top > bottom)
	{
		top = 0;
		bottom = rows - 1;
	}
	return;
}
void findTopBottom_bottom2up(int* array_x, int rows, int& top, int& bottom)
{
	calcSize_bottom2up(array_x, rows, top, bottom);

	if (top < 0)
		top = 0;
	if (bottom >= rows)
		bottom = rows;
	if (top > bottom)
	{
		top = 0;
		bottom = rows - 1;
	}
	return;
}
void findLeftRight(int* array_y, int cols, int& left, int& right)
{
	left = getFirstIndex(array_y, cols, 1);
	right = getLastIndex(array_y, cols, 1);
	////////////////////////////////////
	if (left > right)
	{
		left = 0;
		right = cols - 1;
	}
	return;
}
void findLeftRight_revised(int* array_y, int cols, int& first_left, int& first_right, int& second_left, int &second_right, bool isMessage = false)
{
	int left = getFirstIndex(array_y, cols, 1);
	int right = getLastIndex(array_y, cols, 1);
	//
	if (left > right)
	{
		left = 0;
		right = cols - 1;
		second_left = second_right = 0;
		return;
	}
	//Check if it is message rect.//////////////////////////////////
	int sect_length = (right - left) / 10;
#if 1
	bool isMulti = true;
	for (int i = (left + 4 * sect_length); i < (left + 6 * sect_length); i++)
	{
		if (array_y[i] != 0)
		{
			isMulti = false;
			break;
		}
	}
#endif
	//
	if (isMessage ||
		(isMulti && (right - left) * 2 > cols))
	{
		first_left = left;
		first_right = getLastIndex(array_y, left + 5 * sect_length, 1);
		second_left = getFirstIndex_revised(array_y, cols, 1, left + 5 * sect_length);// left + 6 * sect_length);
		second_right = right;
	}
	else
	{
		//if (isMulti && (right - left) * 2 > cols)
		//{
		//	first_left = left;
		//	first_right = getLastIndex(array_y, left + 5 * sect_length, 1);
		//}
		//else
		//{
			first_left = left;
			first_right = right;
		//}
		second_left = second_right = 0;
	}

	return;
}
/////
#define OCR_SHOWARRAY_HEIGHT 300
void showArray_h(char* title, int* array_h, int arraySize)
{
	Mat ip_h(arraySize, OCR_SHOWARRAY_HEIGHT, CV_8UC3);
	if (!array_h)
		return;
	for (int i = 0; i< arraySize; i++)
		line(ip_h, cv::Point(0, i), cv::Point(array_h[i], i), Scalar(255, 0, 0), 2, 8, 0);
	imshow(title, ip_h);
	waitKey(4000);
}
void showArray_v(char* title, int* array_v, int arraySize)
{
	Mat ip_v(OCR_SHOWARRAY_HEIGHT, arraySize, CV_8UC3);
	if (!array_v)
		return;
	for (int i = 0; i< arraySize; i++)
		line(ip_v, cv::Point(i, 0), cv::Point(i, array_v[i]), Scalar(255, 0, 0), 2, 8, 0);
	imshow(title, ip_v);
	waitKey(4000);
}

void blurArray(int* array_t, int arraySize,int kernelsize = 1)
{
	if (kernelsize < 1 ||
		2 * kernelsize > arraySize)
		return;
	int *tmp = new int[arraySize];
	for (int i = 0; i < arraySize; i++)
		tmp[i] = array_t[i];
	for (int i = kernelsize; i < (arraySize - kernelsize); i++)
	{
		int sum = 0;
		for (int j = -kernelsize; j < kernelsize; j++)
			sum += array_t[i + j];
		sum /= (kernelsize * 2 + 1);
		tmp[i] = sum;
	}
	for (int i = 0; i < arraySize; i++)
		array_t[i] = tmp[i];
	delete[]tmp;
}