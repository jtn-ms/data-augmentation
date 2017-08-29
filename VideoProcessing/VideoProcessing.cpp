// VideoProcessing.cpp : Defines the entry point for the console application.
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

#ifdef _DEBUG
#pragma comment(lib, "common_debug.lib")
#else
//#pragma comment(lib, "common.lib")
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
std::string int2string(int num)
{
	std::ostringstream out_stream;
	out_stream << num;
	return out_stream.str();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
int getfilecount(string input_directory)
{
	DIR     *dir_p = opendir(input_directory.c_str());
	struct dirent *dir_entry_p;
	if (dir_p == NULL) {
		fprintf(stderr, "Failed to open directory %s\n", input_directory.c_str());
		return 0;
	}
	//  
	int filecount = 0;
	while ((dir_entry_p = readdir(dir_p)) != NULL)
	{
		if (strcmp(dir_entry_p->d_name, ""))
			fprintf(stderr, "Examining file %s\n", dir_entry_p->d_name);
		if (!strcmp(dir_entry_p->d_name, ".") || !strcmp(dir_entry_p->d_name, ".."))
			continue;
		filecount++;
	}
	closedir(dir_p);
	return filecount;

}
//
double getSimilarity(Mat A, Mat B) {
	if (A.rows > 0 && A.rows == B.rows && A.cols > 0 && A.cols == B.cols) {
		// Calculate the L2 relative error between images.
		double errorL2 = norm(A, B, CV_L2);
		// Convert to a reasonable scale, since L2 error is summed across all pixels of the image.
		double similarity = errorL2 / (double)(A.rows * A.cols);
		return similarity;
	}
	else {
		//Images have a different size
		return 100000000.0;  // Return a bad value
	}
}
//
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

	string outpath, outdir, cmd;
#define ONE_DIR_FOR_ANY_VIDEO_FILE
#ifndef ONE_DIR_FOR_ANY_VIDEO_FILE
#define ONE_DIR_PER_ONE_VIDEO_FILE
#endif

#ifdef ONE_DIR_FOR_ANY_VIDEO_FILE
	string dir_name = "bg-files";
	outdir = output_directory + dir_name;
	cmd = "mkdir " + outdir;
	system(cmd.data());
#endif

	int frame_count = 0;
	int file_index = 0;
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

		VideoCapture cap(strPrefix);

		if (!cap.isOpened())
			continue;

			//Make output dir
#ifdef ONE_DIR_PER_ONE_VIDEO_FILE
		fprintf(stderr, "Processing image %s\n", strPrefix.c_str());
		outpath = output_directory + dir_entry_p->d_name;
		outdir = outpath.substr(0, outpath.find_first_of("."));
		cmd = "mkdir " + outdir;
		system(cmd.data());
		frame_count = 0;
		file_index = 0;
#endif

#ifdef SPECIFIC_DATA_PROCESSING
		if (String(dir_entry_p->d_name) != String("IMG_1351.m4v"))
			continue;
#endif
		Mat in;
		int fps = 5;
		Mat tmp;
		while (true)
		{
			bool success = cap.read(in);
			if (!success)
				break;
			if (!in.empty())
			{
				frame_count++;
				/*int file_index = frame_count / fps;*/
				if ((frame_count % fps) == 0)
				{
					if (tmp.empty())
					{
						imwrite(outdir + "//" + int2string(file_index) + ".png", in);
						resize(in, tmp, Size(500, 500));
						file_index++;
					}
					else{
						Mat in_resized;
						resize(in, in_resized, Size(500, 500));
						if (getSimilarity(in_resized, tmp) > 0.25)
						{
							imwrite(outdir + "//" + int2string(file_index) + ".png", in);
							file_index++;
							in_resized.copyTo(tmp);
						}
					}
				}
			}
			//imshow(dir_entry_p->d_name,in);
			//waitKey(1);
		}


	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
static const char * dir1 = "H:\\Samples\\bkg-indoor\\";
static const char * dir2 = "H:\\Samples\\bkg-indoor\\";
int _tmain(int argc, char* argv[])
{
	processDir(dir1, dir2);
	return 0;
}

