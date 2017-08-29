// CropImage.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
//////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_OPENCV
#include <opencv/cv.h>  
#include <opencv/cvaux.h>  
#include <opencv2\highgui\highgui.hpp>
#include <opencv2/core/core.hpp>

using namespace cv;

#ifdef _DEBUG
#pragma comment(lib, "opencv_core2411d.lib")
#pragma comment(lib, "opencv_highgui2411d.lib")
//#pragma comment(lib, "opencv_imgcodecs2411d.lib")
#pragma comment(lib, "opencv_imgproc2411d.lib")
#else
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#pragma comment(lib, "opencv_core320.lib")
#pragma comment(lib, "opencv_highgui320.lib")
#pragma comment(lib, "opencv_imgcodecs320.lib")
#pragma comment(lib, "opencv_imgproc320.lib")
#endif

#endif
////////////////////////////////////////////////////////////////////////////////////////////
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
#pragma comment(lib, "common.lib")
#else
#pragma comment(lib, "common.lib")
#endif

using namespace std;
///////////////////////////////////////////////////////////////////////////////////////////////
void calcHorizontalProjection(Mat thresh_output, int* sum_h, int value)
{
	if (!thresh_output.data || !sum_h || value < 0 || value > 255) return;
	for (int i = 0; i< thresh_output.rows; i++)
	{
		sum_h[i] = 0;
		for (int j = 0; j < thresh_output.cols; j++)
		if (*thresh_output.ptr(i, j) == value)
			sum_h[i]++;
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
#define OCR_SHOWARRAY_HEIGHT 400
void showArray_h(char* title, int* array_h, int arraySize)
{
	Mat ip_h(arraySize, OCR_SHOWARRAY_HEIGHT, CV_8UC3);
	if (!array_h)
		return;
	for (int i = 0; i< arraySize; i++)
		line(ip_h, cv::Point(0, i), cv::Point(array_h[i], i), Scalar(255, 0, 0), 2, 8, 0);
	imshow(title, ip_h);
	waitKey(1000);
}
void showArray_v(char* title, int* array_v, int arraySize)
{
	Mat ip_v(OCR_SHOWARRAY_HEIGHT, arraySize, CV_8UC3);
	if (!array_v)
		return;
	for (int i = 0; i< arraySize; i++)
		line(ip_v, cv::Point(i, 0), cv::Point(i, array_v[i]), Scalar(255, 0, 0), 2, 8, 0);
	imshow(title, ip_v);
	waitKey(1000);
}
int start_x, start_y, item_w, item_h, interval_x,interval_y;
void getItemInfo(int *ip_array,int array_size,int &start,int &itemsize,int &interval)
{
	/*
		in:
			ip_array,array_size
		out:
			start,itemsize,interval
	*/
	bool getStart = false, getSize = false, getInv = false;
	for (int i = 0; i < array_size; i++)
	{
		if (!getStart && ip_array[i] != 0)
		{
			getStart = true;
			start = i;
		}
		if (!getSize && getStart && ip_array[i] == 0)
		{
			getSize = true;
			itemsize = i - start;
		}
		if (getSize && ip_array[i] != 0)
		{
			getInv = true;
			interval = i - start - itemsize;
			break;
		}
	}
	if (!getInv)
	{
		int end = array_size - start - itemsize;
		interval = start < end ? start:end;
	}
	return;
}
void analyzeGraph(int *array_h, int size_harray,int * array_v, int size_varray)
{
	getItemInfo(array_h, size_harray, start_y, item_h, interval_y);
	getItemInfo(array_v, size_varray, start_x, item_w, interval_x);
}
int string2int(std::string& str)
{
	std::istringstream in_stream(str);
	int res;
	in_stream >> res;
	return res;
}

std::string int2string(int num)
{
	std::ostringstream out_stream;
	out_stream << num;
	return out_stream.str();
}
void doCropping(Mat in,string outpath)
{
	int step_w = item_w + interval_x, step_h = item_h + interval_y;
	int margin = min(5,min(interval_y / 2, interval_x / 2));
	int start_xx = start_x - margin, start_yy = start_y - margin;
	int crop_w = margin * 2 + item_w, crop_h = 2 * margin + item_h;
	int count = 1;
	for (int i = start_xx; i < (in.cols - interval_x/2 - item_w); i += step_w)
	{
		for (int j = start_yy; j < (in.rows - interval_y/2 - item_h); j += step_h)
		{
			Mat roi(in, Rect(i, j, crop_w, crop_h));

			imwrite(outpath + "\\" + int2string(count) + ".png", roi);
			count++;
			//imshow("cropped",roi);
			//waitKey(1000);
		}
	}

}
void getInfo(Mat in)
{
	Mat gray,thr;
	if (in.type() == CV_8UC3)
		cvtColor(in, gray, CV_BGR2GRAY);
	else
		gray = in;
	threshold(gray, thr, 220, 255, THRESH_BINARY_INV);
#ifdef _DEBUG
	//imshow("thr",thr);
#endif
	int *ip_h = new int[in.rows], *ip_v = new int[in.cols];
	calcHorizontalProjection(thr, ip_h, 255);
	calcVerticalProjection(thr, ip_v, 255);
#ifdef _DEBUG
	//showArray_h("horizontal", ip_h, in.rows);
	//showArray_v("vertical", ip_v, in.cols);
#endif
	analyzeGraph(ip_h,in.rows,ip_v,in.cols);
	//doCropping(in);
	delete	[]ip_h;
	delete	[]ip_v;
	//waitKey(1000);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
static const char * dir1 = "H:\\Samples\\Brickets\\";
static const char * dir2 = "H:\\Samples\\Brickets\\";
int _tmain(int argc, char* argv[])
{
	string strPrefix;
	string strPostfix;
	string input_directory;
	string output_directory;
	string output_file;


	//#define CREATE_GRAY_IMAGES
	input_directory = dir1;//argv[1];
	output_directory = dir2;//argv[2];
	/* Get a file listing of all files with in the input directory */
	DIR     *dir_p = opendir(input_directory.c_str());
	struct dirent *dir_entry_p;
	if (dir_p == NULL) {
		fprintf(stderr, "Failed to open directory %s\n", input_directory.c_str());
		return -1;
	}
	//     init output of rectangles to the info file  
	fprintf(stderr, "done.\n");
	//
	srand(time(NULL));
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
		//strPrefix+=bmp_file.name;  
		fprintf(stderr, "Loading image %s\n", strPrefix.c_str());
		Mat in = imread(strPrefix), out;
		if (!in.empty())
		{
			fprintf(stderr, "Processing image %s\n", strPrefix.c_str());
			string outpath = output_directory + dir_entry_p->d_name;
			string outdir = outpath.substr(0,outpath.find_first_of("."));
			string cmd = "mkdir " + outdir;
			system(cmd.data());
			//imshow("show", in);
			getInfo(in);
			doCropping(in, outdir);
			cvWaitKey(100);
		}

	}
	return 0;
}


