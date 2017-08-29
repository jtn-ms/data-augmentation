// MergeFolder.cpp : Defines the entry point for the console application.
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
#pragma comment(lib, "opencv_videoio320.lib")
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

using namespace std;
////////////////////////////////////////////////////////////////////////////////////////////////////
std::string int2string(int num)
{
	std::ostringstream out_stream;
	out_stream << num;
	return out_stream.str();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
static const char * in1 = "H:\\Samples\\Drugs\\IMG_1341\\";
static const char * in2 = "H:\\Samples\\Drugs\\IMG_1342\\";
static const char * out = "H:\\Samples\\Drugs\\No\\";

void processOneFolder(string input_directory, string output_directory, int& count)
{
	string strPrefix;
	string strPostfix;

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
	while ((dir_entry_p = readdir(dir_p)) != NULL)
	{
		if (strcmp(dir_entry_p->d_name, ""))
			fprintf(stderr, "Examining file %s\n", dir_entry_p->d_name);
		/* TODO: Assign postfix/prefix info */
		strPostfix = "";
		strPrefix = input_directory;
		strPrefix += dir_entry_p->d_name;
		fprintf(stderr, "Loading image %s\n", strPrefix.c_str());

		Mat in = imread(strPrefix), out;
		if (!in.empty())
		{
			resize(in, out, Size(256, 256));
			count++;
			imwrite(output_directory + int2string(count) + ".png", out);
		}
	}
}

int main(int argc, char** argv)
{
	int count = 0;
	processOneFolder(in1, out, count);
	processOneFolder(in2, out, count);
	return 0;
}



