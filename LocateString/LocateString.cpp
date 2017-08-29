// LocateString.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_OPENCV
#include <opencv/cv.h>  
#include <opencv/cvaux.h>  
#include <opencv2\highgui\highgui.hpp>
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
using namespace cv;

#ifdef _DEBUG
#pragma comment(lib, "opencv_core320d.lib")
#pragma comment(lib, "opencv_highgui320d.lib")
#pragma comment(lib, "opencv_imgcodecs320d.lib")
#pragma comment(lib, "opencv_imgproc320d.lib")
#else
#pragma comment(lib, "opencv_core320.lib")
#pragma comment(lib, "opencv_highgui320.lib")
#pragma comment(lib, "opencv_imgcodecs320.lib")
#pragma comment(lib, "opencv_imgproc320.lib")
#endif

#endif

#include "array.h"
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
#define DIR_PROCESSING
#ifdef DIR_PROCESSING
// for filelisting  
#include <stdio.h>
#include <string>
#include <sys/stat.h>
// for fileoutput  
#include <string>  
#include <fstream>  
#include <sstream>  
#include "common.h"  
#include <iostream>
#include <ctime>

#ifdef _DEBUG
#pragma comment(lib, "common-debug.lib")
#else
#pragma comment(lib, "common.lib")
#endif

using namespace std;
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////

#define STRING_COUNT 15
#define OCR_PIXEL_MAX 255
#define OCR_PIXEL_MIN 0
#define USER_DEFINED__H_MARGIN 5
#define USER_DEFINED__W_MARGIN 5
#define STRING_HEIGHT_ACCEPTABLE_RANGE_MIN (2 * USER_DEFINED__H_MARGIN + 5)
#define STRING_HEIGHT_ACCEPTABLE_RANGE_MAX 40

bool isMessage = false;

////////////////
std::vector<cv::Point2f>  ocr_test_goodfeatures_as_points(Mat in)
{
	std::vector<cv::Point2f> corners;
	if (in.empty() ||
		in.type() > 1)
		return corners;
	// Compute good features to track
	cv::goodFeaturesToTrack(in, corners,
		1000,   // maximum number of corners to be returned
		0.1,   // quality level
		1);   // minimum allowed distance between points
	return corners;
}
#define USER_DEFINED_ERASE_REGION_LEFTRIGHT 0.05//0.2
#define USER_DEFINED_ERASE_REGION_TOPBOTTOM 0.05//0.18
std::vector<cv::Point2f> getGoodFeaturesAsPoints(Mat in)
{
	std::vector<cv::Point2f> corners;

	if (in.empty() || in.type() != CV_8UC1)
		return corners;
	// Compute good features to track

	cv::goodFeaturesToTrack(in, corners,
		1000,//500,//   // maximum number of corners to be returned
		0.01,   // quality level
		1);   // minimum allowed distance between points
	////////////////////////////////////////////////////////////////
	std::vector<cv::Point2f> neighbors;
	std::vector<cv::Point2f>::iterator itc;
#if 1

	itc = corners.begin();
	while (itc != corners.end()) {
		cv::Point2f point = *itc;
		if ((point.y < in.rows * USER_DEFINED_ERASE_REGION_TOPBOTTOM) ||
			(point.y > in.rows * (1 - USER_DEFINED_ERASE_REGION_TOPBOTTOM)) ||
			(point.x < in.cols * USER_DEFINED_ERASE_REGION_LEFTRIGHT) ||
			(point.x > in.cols * (1 - USER_DEFINED_ERASE_REGION_LEFTRIGHT)))
			itc = corners.erase(itc);
		else
			++itc;
	}

	//step 2----->removing noise-diameter = 15///-kjy-todo-2015.4.28
	neighbors = corners;
	itc = corners.begin();
	int thr_dis, thr_con;

	thr_dis = 16;//15;
	thr_con = 7;//6;

	while (itc != corners.end()) {
		//Create bounding rect of object
		cv::Point2f  observingpoint = *itc;
		int count = 0;
		for (std::vector<cv::Point2f>::iterator iter = neighbors.begin(); iter != neighbors.end(); iter++)
		{
			cv::Point2f neighbor = *iter;
			double delta_x = abs(observingpoint.x - neighbor.x);
			double delta_y = abs(observingpoint.y - neighbor.y);
			delta_x *= 1;
			delta_y *= 2;
			double distance = sqrt(delta_x * delta_x + delta_y * delta_y);
			if (distance < thr_dis)//10.0)//8.0)
				count++;
		}
		if (count < thr_con)//3)//5)
			itc = corners.erase(itc);
		else
			++itc;
	}
#endif

#if 1

	itc = corners.begin();
	while (itc != corners.end()) {
		cv::Point2f point = *itc;
		if ((point.y < in.rows * USER_DEFINED_ERASE_REGION_TOPBOTTOM) ||
			(point.y > in.rows * (1 - USER_DEFINED_ERASE_REGION_TOPBOTTOM)) ||
			(point.x < in.cols * USER_DEFINED_ERASE_REGION_LEFTRIGHT) ||
			(point.x > in.cols * (1 - USER_DEFINED_ERASE_REGION_LEFTRIGHT)))
			itc = corners.erase(itc);
		else
			++itc;
	}

	//step 2----->removing noise-diameter = 15///-kjy-todo-2015.4.28
	neighbors = corners;
	itc = corners.begin();
	while (itc != corners.end()) {
		//Create bounding rect of object
		cv::Point2f  observingpoint = *itc;
		int count = 0;
		for (std::vector<cv::Point2f>::iterator iter = neighbors.begin(); iter != neighbors.end(); iter++)
		{
			cv::Point2f neighbor = *iter;
			double delta_x = abs(observingpoint.x - neighbor.x);
			double delta_y = abs(observingpoint.y - neighbor.y);
			delta_x *= 1;
			delta_y *= 2;
			double distance = sqrt(delta_x * delta_x + delta_y * delta_y);
			if (distance < thr_dis)//10.0)//8.0)
				count++;
		}
		if (count < thr_con)//3)//5)
			itc = corners.erase(itc);
		else
			++itc;
	}
#endif
	return corners;
}
Mat testGoodFeatures(Mat grayImg)
{
	Mat gf_space = Mat::zeros(grayImg.rows, grayImg.cols, CV_8UC1);
	std::vector<cv::Point2f> pointlist = getGoodFeaturesAsPoints(grayImg);
	for (unsigned int i = 0; i< pointlist.size(); ++i)
	{
		cv::ellipse(gf_space, pointlist[i], cv::Size(2, 1), 0, 0, 360, cv::Scalar(255), 2, 8, 0);
	}
#ifdef _DEBUG
	imshow("goodfeature_space", gf_space);
	waitKey(1000);
#endif
	return gf_space;
}
/////////////////
void getRectList(std::vector<cv::Rect>& rectlist,Mat extracted)
{
	int top, left, right, bottom;
	int *sum_h, *sum_v;
	uchar *data = extracted.data;
	sum_h = new int[extracted.rows];
	sum_v = new int[extracted.cols];
	////////Get Top & Bottom (an info about the horizontal location of an assumed StringRect)///////////////////////////
	calcHorizontalProjection(extracted, sum_h, OCR_PIXEL_MAX);
	eliminateLowValueRegion(sum_h, extracted.rows, 2);
	blurArray(sum_h, extracted.rows,1);
	///
	for (int i = 0; i < STRING_COUNT; i++)
	{
//#define OCR_ONCE
#ifdef OCR_ONCE
		showArray_h("horizontal", sum_h, extracted.rows);
#endif
		///
		if (!isMessage)
			findTopBottom(sum_h, extracted.rows, top, bottom);
		else
			findTopBottom_bottom2up(sum_h, extracted.rows, top, bottom);
		top -= USER_DEFINED__H_MARGIN;
		bottom += USER_DEFINED__H_MARGIN;
		if (top < 0) top = 0;
		if (bottom >= extracted.rows) bottom = extracted.rows - 1;
		//
		if (top == 0 &&
			bottom == (extracted.rows - 1))
			break;
		//Ignore Non-String Part(Line Part)//////////////////////////////////////////////////////////
		if (((bottom - top) < STRING_HEIGHT_ACCEPTABLE_RANGE_MIN) ||
			((bottom - top) > STRING_HEIGHT_ACCEPTABLE_RANGE_MAX))
		{
			//memset(data + extracted.cols * top, 0, extracted.cols * (bottom - top));
			for (int ii = top; ii < bottom; ii++)
				sum_h[ii] = 0;
			continue;
		}
		/////////Get Left & Right (an info about the vertical location of an assumed StringRect)///////////////////////////
		Mat croppedbyTopBottom(extracted, cv::Rect(0, top, extracted.cols, bottom - top));
		calcVerticalProjection(croppedbyTopBottom, sum_v, OCR_PIXEL_MAX);
#ifdef OCR_ONCE
		imshow("cropped", croppedbyTopBottom);
		waitKey(1000);
		showArray_v("TEst", sum_v, croppedbyTopBottom.cols);
#endif
#ifdef DEFAULT_PROBLEM//one region per line
		findLeftRight(sum_v, croppedbyTopBottom.cols, left, right);
		////////////////////////////kjy-todo-2015.05.08///////////
		left -= USER_DEFINED__W_MARGIN;
		right += USER_DEFINED__W_MARGIN;
		////////////////////////////////////////////////
		if (left < 0) left = 0;
		if (right >= extracted.cols) right = extracted.cols - 1;
		/////////Form the rect of the suspicious StringRect///////////////////////////////
		cv::Rect rect = cv::Rect(left, top, right - left, bottom - top);
		/////////Reflect the Engine//////////////////////////////////////////////////////////
		//memset(data + extracted.cols * top, 0, extracted.cols * (bottom - top));
		for (int ii = top; ii < bottom; ii++)
			sum_h[ii] = 0;
		rectlist.push_back(rect);
#else//two region per line
		int left2, right2;
		findLeftRight_revised(sum_v, croppedbyTopBottom.cols, left, right,left2,right2,isMessage);
		////////////////////////////kjy-todo-2015.05.08///////////
		if (left2 == right2)
		{
			left -= USER_DEFINED__W_MARGIN;
			right += USER_DEFINED__W_MARGIN;
			////////////////////////////////////////////////
			if (left < 0) left = 0;
			if (right >= extracted.cols) right = extracted.cols - 1;
			/////////Form the rect of the suspicious StringRect///////////////////////////////
			cv::Rect rect = cv::Rect(left, top, right - left, bottom - top);
			/////////Reflect the Engine//////////////////////////////////////////////////////////
			for (int ii = top; ii < bottom; ii++)
				sum_h[ii] = 0;
			//exception handle////////////////////////////////////////////////
			if (top == 0 &&
				bottom == (extracted.rows - 1))
				continue;
			if (2 * rect.width > extracted.cols)
				continue;
			rectlist.push_back(rect);
		}
		else
		{
			left -= USER_DEFINED__W_MARGIN;
			left2 -= USER_DEFINED__W_MARGIN;
			right += USER_DEFINED__W_MARGIN;
			right2 += USER_DEFINED__W_MARGIN;
			////////////////////////////////////////////////
			if (left < 0) left = 0;
			if (left2 < 0) left2 = 0;
			if (right >= extracted.cols) right = extracted.cols - 1;
			if (right2 >= extracted.cols) right2 = extracted.cols - 1;
			/////////Form the rect of the suspicious StringRect///////////////////////////////
			cv::Rect rect1 = cv::Rect(left, top, right - left, bottom - top),
					 rect2 = cv::Rect(left2, top, right2 - left2, bottom - top);
			/////////Reflect the Engine//////////////////////////////////////////////////////////
			for (int ii = top; ii < bottom; ii++)
				sum_h[ii] = 0;
			//////////////////////////////////////////////////
			rectlist.push_back(rect1);
			rectlist.push_back(rect2);
		}
		if (isMessage)
			break;
#endif
	}
	delete[]sum_v;
	delete[]sum_h;
	return;
}
////
Mat GreenFilter(Mat bgr)
{
	if (bgr.empty() ||
		bgr.type() != CV_8UC3)
		return Mat();
	Mat hsv;
	cvtColor(bgr, hsv, CV_BGR2HSV);
	Mat filter = Mat::zeros(bgr.rows, bgr.cols, CV_8UC1);
	for (int j = 0; j < hsv.rows; j++)
	{
		for (int i = 0; i < hsv.cols; i++)
		{
			if (*hsv.ptr(j, i) >100 && *hsv.ptr(j, i) < 110 &&
				(*(hsv.ptr(j, i) + 1) > 100 && *(hsv.ptr(j, i) + 1) <180) &&
				*(hsv.ptr(j, i) + 2) > 180)
				*filter.ptr(j, i) = 255;
		}
	}
	imshow("colorFilter", filter);
	waitKey(1000);
	return filter;
}
Mat GrayFilter(Mat bgr)
{
	if (bgr.empty() ||
		bgr.type() != CV_8UC3)
		return Mat();
	Mat hsv;
	cvtColor(bgr, hsv, CV_BGR2HSV);
	Mat filter = Mat::zeros(bgr.rows, bgr.cols, CV_8UC1);
	for (int j = 0; j < hsv.rows; j++)
	{
		for (int i = 0; i < hsv.cols; i++)
		{
			int b = *bgr.ptr(j, i);
			int g = *(bgr.ptr(j, i) + 1);
			int r = *(bgr.ptr(j, i) + 2);
			if (*hsv.ptr(j, i) != 0 ||
				r != g ||
				r != b ||
				g != b)
			{
				*filter.ptr(j, i) = 255;
			}
		}
	}
	imshow("grayFilter", filter);
	waitKey(1000);
	return filter;
}
bool isGreenMessage(Mat in,Mat hsv,Mat& gray)
{
	if (hsv.empty() || hsv.type() != CV_8UC3)
		return false;

	bool isTrue = false;
	int  left_start = 0, right_start = 0;
	int j;

	for (j = 0; j < hsv.rows; j++)
	{
		bool isLeft = false;
		for (int i = 0; i < hsv.cols; i++)
		{
			//Black-Only-Filter
			int b = *in.ptr(j, i);
			int g = *(in.ptr(j, i) + 1);
			int r = *(in.ptr(j, i) + 2);
			if (*hsv.ptr(j, i) != 0 ||
				r != g ||
				r != b ||
				g != b)
			{
				*gray.ptr(j, i) = 255;
			}
			//GreenDetect
			if ((*hsv.ptr(j, i) >100 && *hsv.ptr(j, i) < 110) &&
				(*(hsv.ptr(j, i) + 1) > 100 && *(hsv.ptr(j, i) + 1) <180) &&
				*(hsv.ptr(j, i) + 2) > 180)
			{
				if (i * 2 < hsv.cols)
				{
					if (!isLeft && 2 * j > hsv.rows)
					{
						isLeft = true;
						left_start = i;
					}
				}
				else
				{
					if (isLeft && (i - left_start) * 2 > hsv.cols)
						isTrue = true;
				}
				if (j * 1.2 > in.rows && i * 2 > in.cols)
					*gray.ptr(j, i) = 0;
			}
		}
		if (isTrue)
			break;
	}
#define GREEN_STRING_HEIGHT 30
	if (isTrue)
	{
		for (int jj = (j - GREEN_STRING_HEIGHT); jj < hsv.rows; jj++)
		{
			for (int ii = 0; ii < hsv.cols; ii++)
			{
				//Black-Only-Filter
				if (jj >= j)
				{
					int b = *in.ptr(jj, ii);
					int g = *(in.ptr(jj, ii) + 1);
					int r = *(in.ptr(jj, ii) + 2);
					if (*hsv.ptr(jj, ii) != 0 ||
						r != g ||
						r != b ||
						g != b)
						*gray.ptr(jj, ii) = 255;
				}
				//GreenFilter
				if ((*hsv.ptr(jj, ii) >100 && *hsv.ptr(jj,ii) < 110) &&
					(*(hsv.ptr(jj, ii) + 1) > 100 && *(hsv.ptr(jj, ii) + 1) <180) &&
					*(hsv.ptr(jj, ii) + 2) > 150)
					*gray.ptr(jj, ii) = 0;
			}
		}
	}
	return isTrue;
}
////
Mat g_in;
#ifndef _DEBUG
#define _DEBUG
#endif
#ifndef _DEBUG
#define DIR_MODE
#endif
Mat estimateType(const char *inpath, const char *outpath = NULL)
{
	Mat in = imread(inpath);
	if (in.empty())
		return Mat();
	resize(in, in, Size(300, 600));
	in.copyTo(g_in);
	//erode(in, in, Mat());
#ifdef _DEBUG
	imshow("in", in);
	waitKey(1000);
#endif
	Mat gray, hsv;
	cvtColor(in, hsv, CV_BGR2HSV);
	cvtColor(in, gray, CV_BGR2GRAY);

#ifdef _DEBUG
	testGoodFeatures(gray);
#endif

	Mat hist;
	resize(in, hist, Size(20, 40), 0.0, 0.0, 0);
	cvtColor(hist, hist, CV_BGRA2GRAY);
	threshold(hist, hist, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);

	isMessage = isGreenMessage(in,hsv,gray);

#ifdef _DEBUG
	imshow("gray", gray);
	waitKey(1000);
	imshow("mask1", hist);
	waitKey(1000);
#endif
	//filtering
#define MESSAGE_MASK
#ifdef MESSAGE_MASK
	bool everInwhite = false;
	for (int j = 0; j < hist.rows; j++)
	{
		int whites = 0,
			blacks = 0;
		int first_white = -1,
			first_black = -1,
			last_white = -1,
			last_black = -1;
		//set first and last white,black pos
		for (int i = 0; i < hist.cols; i++)
		{
			if (*hist.ptr(j, i) > 128)
			{
				if (first_white == -1)
					first_white = i;
				last_white = i;
				whites++;
			}
			else
			{
				if (first_black == -1)
					first_black = i;
				last_black = i;
				blacks++;
			}
		}
		//
		int margin = 1;
		
		if (whites > blacks)
		{
			everInwhite = true;
			if (first_white != 0)
			{
				memset(hist.data + j * hist.cols + first_white, 255, last_white - first_white - margin);
				memset(hist.data + j * hist.cols + last_white, 0, margin);//shrink-row
			}
			else
				memset(hist.data + j * hist.cols + first_white, 255, hist.cols);
		}
		else
		{
			if (everInwhite)
				memset(hist.data + (j - 1) * hist.cols, 0, hist.cols);//shrink-col
			everInwhite = false;
			if (first_black != 0)
				memset(hist.data + j * hist.cols + first_black, 0, last_black-first_black);
			else
				memset(hist.data + j * hist.cols + first_black, 0, hist.cols);
		}
	}
	//
	threshold(hist, hist, 128, 255, THRESH_BINARY);
#ifdef _DEBUG
	imshow("mask2", hist);
	waitKey(1000);
#endif
	int *sum_h = new int[hist.rows];
	int *sum_v = new int[hist.cols];
	calcHorizontalProjection(hist, sum_h, OCR_PIXEL_MAX);
	calcVerticalProjection(hist, sum_v, OCR_PIXEL_MAX);
	if (!isMessage)
		if (!isMessage &&
			sum_h[0] == 0 &&
			sum_h[hist.rows - 1] == 0 &&
			sum_v[0] == 0 &&
			sum_v[hist.cols - 1] == 0)
			isMessage = true;
		else
			isMessage = false;
	delete[]sum_h;
	delete[]sum_v;

	//
#endif
	//apply mask to in
	int h_ratio = in.rows / hist.rows,
		w_ratio = in.cols / hist.cols;

	memset(gray.data, 255, h_ratio * in.cols);
	memset(gray.data + (in.rows - h_ratio) * in.cols, 255, h_ratio* in.cols);

	for (int j = 0; j < hist.rows; j++)
	{
		memset(gray.data + j * h_ratio * gray.cols, 255, w_ratio);
		memset(gray.data + h_ratio * gray.cols + (in.cols - w_ratio), 255, w_ratio);
		for (int i = 0; i < hist.cols; i++)
		{
			if (*hist.ptr(j, i) < 128)
			{
				for (int k = 0; k < h_ratio; k++)
					memset(gray.data + (j * h_ratio + k) * in.cols + i * w_ratio, 255, w_ratio);
			}
		}
	}
	//
#if 0
	for (int j = 0; j < hist.rows; j++)
	{
		int sum = 0;
		for (int i = 0; i < hist.cols; i++)
			sum += *hist.ptr(j, i);
		sum /= hist.cols;
		printf("f(%d) = %d \n", j, sum);
	}
#endif

#ifdef _DEBUG
	resize(hist, hist, Size(300, 600), 0.0, 0.0, 0);
	if(outpath != NULL) imwrite(outpath, hist);
	imshow("hist", hist);
	waitKey(1000);
#endif
	//
	return gray;
}
//
#define IGNORE_MARGIN_HEIGHT 15
#define IGNORE_MARGIN_WIDTH 10
void locateString(const char *inpath,const char *outpath = NULL)
{
	Mat featureSpace;
#define USE_MASK
#ifdef USE_MASK
	Mat gray;
	Mat mask = estimateType(inpath);
	if (mask.empty())
		return;
	gray = mask;
	//Get the FeatureSpace
	threshold(gray, featureSpace, 128, 255, THRESH_BINARY_INV);
#else
	Mat in;
	in = imread(inpath);
	if (in.empty())
		return;
	
	resize(in,in,Size(300,600));
	in.copyTo(g_in);
	//2.Gray
	Mat gray;
	Mat hsv;
	cvtColor(in, hsv, CV_BGR2HSV);
	cvtColor(in, gray, CV_BGR2GRAY);
	featureSpace = testGoodFeatures(gray);
#ifdef _DEBUG
	imshow("in", in);
	waitKey(1000);
	imshow("featureSpace", featureSpace);
	waitKey(1000);
#endif
#endif
	//4.locate
	std::vector<cv::Rect> rectlist;
	getRectList(rectlist, featureSpace);
	//5.show
#if 1
	for (int i = 0; i < rectlist.size(); i++)
		rectangle(g_in, rectlist[i], Scalar(0, 255, 255), 3);
#ifdef _DEBUG
	imshow("marked", g_in);
	waitKey(1000);
#endif
#endif
#ifdef _DEBUG
	Mat origin = imread(inpath);
	float ratio_h = (origin.rows / 600.0);
	float ratio_w = (origin.cols / 300.0);
	for (int i = 0; i < rectlist.size(); i++)
	{
		Mat singlerect(origin, Rect(rectlist[i].x * ratio_w, rectlist[i].y * ratio_h, rectlist[i].width * ratio_w, rectlist[i].height * ratio_h));
		Mat sr_gray;
		Mat sr_thr;
		cvtColor(singlerect, sr_gray, CV_BGR2GRAY);
		threshold(sr_gray, sr_thr, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);
		imshow("binarization", sr_thr);
		waitKey(1000);	
	}
#endif
	//6.save
	if (outpath != NULL)
		imwrite(outpath, g_in);
	//concatenate all
#ifdef CONCAT_ALL
	int height_single = 0;
	int width_single = 0;
	for (int i = 0; i < rectlist.size(); i++)
	{
		if (height_single < rectlist[i].height)
			height_single = rectlist[i].height;
		width_single += rectlist[i].width;
	}
	Mat concat = Mat::zeros(height_single, width_single, CV_8UC1);
	int offset_x = 0;
	for (int i = 0; i < rectlist.size(); i++)
	{
		for (int h = 0; h < rectlist[i].height; h++)
		{
			memcpy(concat.data + h * width_single + offset_x, gray_backup.data + (rectlist[i].y + h) * in.cols + rectlist[i].x, rectlist[i].width);
		}
		offset_x += rectlist[i].width;
	}
	//imshow("concat", concat);
	//waitKey(1000);
#endif

}


//#define TEST_SPECIAL
void processDir(string input_directory, string output_directory)
{
	string strPrefix;
	string strPostfix;
	string output_file;

	/* Get a file listing of all files with in the input directory */
	DIR     *dir_p = opendir(input_directory.c_str());
	struct dirent *dir_entry_p;
	if (dir_p == NULL) {
		fprintf(stderr, "Failed to open directory %s\n", input_directory.c_str());
		return;
	}
	//     init output of rectangles to the info file  
	fprintf(stderr, "done.\n");
	//
	int count = 0;
	ofstream output(output_file.c_str());
	while ((dir_entry_p = readdir(dir_p)) != NULL)
	{
		if (strcmp(dir_entry_p->d_name, ""))
			fprintf(stderr, "Examining file %s\n", dir_entry_p->d_name);
		/* TODO: Assign postfix/prefix info */
		strPostfix = "";
		strPrefix = input_directory;
		strPrefix += dir_entry_p->d_name;
		fprintf(stderr, "Loading image %s\n", strPrefix.c_str());
		
		string processtag = "";//"processed\\";
#ifdef TEST_SPECIAL
		string outpath = output_directory + processtag + dir_entry_p->d_name;
		output_file = outpath.substr(0, outpath.find_last_of(".")) + "_masked.png";
		estimateType(strPrefix.c_str(), output_file.c_str());
#else		
		string outpath = output_directory + processtag + dir_entry_p->d_name;
		fprintf(stderr, "Processing image %s\n", strPrefix.c_str());
		string cmd = "mkdir " + outpath.substr(0, outpath.find_last_of("\\"));
		system(cmd.data());
		locateString(strPrefix.c_str(), outpath.c_str());
#endif
	}
}

char filename[] = "H:\\Samples\\Brickets\\1.jpg";
int dir_count = 15;
char *input__dir = "G:\\tesseract\\sample\\liuwei\\";
char *output__dir = "G:\\tesseract\\sample\\output\\";
char *dirs[] = { "huawei-2017\\BLN-AL10\\",
				 "huawei-2017\\CAM-TL00\\",
				 "huawei-2017\\CAZ-TL10\\",
				 "huawei-2017\\CUN-TL00\\",
				 "huawei-2017\\DIG-AL00\\",
				 "huawei-2017\\EDI-AL00\\",
				 "huawei-2017\\KNT-TL10\\",
				 "huawei-2017\\Mate9-Pro\\",
				 "huawei-2017\\MHA-AL00\\",
				 "huawei-2017\\MLA-AL00\\",
				 "huawei-2017\\NEM-UL10\\",
				 "huawei-2017\\PRA-AL00\\",
				 "huawei-2017\\VNS-AL00\\",
				 "leshi-2017\\leshi2\\",
				 "sanxing-2017\\galaxys7\\"
				};

//#define DIR_MODE
#ifndef DIR_MODE
#define FILE_MODE
#endif

int main(int argc, char** argv)
{
#ifdef DIR_MODE
	for (int i = 0; i < dir_count; i++)
	{
		string inpath = input__dir;inpath+=dirs[i];
		string outpath = output__dir; outpath += dirs[i];
		processDir(inpath, outpath);
	}
#endif
#ifdef FILE_MODE
	string filepath;
	char fn[] = "0.1.png";
	if (argc < 2)
	{
		filepath = input__dir;
		filepath += dirs[6];
		filepath += fn;
		
	}
	else
	{
		filepath = argv[1];
	}
#ifdef TEST_SPECIAL
	estimateType(filepath.c_str());
#else
	//locateString(filepath.c_str());
	locateString(filename);
#endif
#endif
	return 0;
}
