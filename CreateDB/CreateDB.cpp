// CreateDB.cpp : Defines the entry point for the console application.
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

#include "transform.h"

//#define _DEBUG
///////////////////////////////////////////////////////////////////////////////////////////////////
char dir[] = "H:\\Samples\\Remotes\\";
char bg_dir[] = "H:\\Samples\\bkg-indoor\\bg-files\\ready\\";
char subdir[] = "Vec\\";
char outdir[] = "TrainData\\";
char filename[] = "ah59-02547b.png";
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
		if (!strcmp(dir_entry_p->d_name, ".") || !strcmp(dir_entry_p->d_name, ".."))
			continue;
		filecount++;
	}
	closedir(dir_p);
	return filecount;

}
///////////////////////////////////////////////////////////////////////////////////////////////////
std::string int2string(int num)
{
	std::ostringstream out_stream;
	out_stream << num;
	return out_stream.str();
}
Mat equalizeIntensity(const Mat& inputImage)
{
	if (inputImage.channels() >= 3)
	{
		Mat ycrcb;

		cvtColor(inputImage, ycrcb, CV_BGR2YCrCb);

		vector<Mat> channels;
		split(ycrcb, channels);

		equalizeHist(channels[0], channels[0]);

		Mat result;
		merge(channels, ycrcb);

		cvtColor(ycrcb, result, CV_YCrCb2BGR);

		return result;
	}
	return Mat();
}


//
void generateTrainData(const char* fg_path, const char* bg_path = NULL, const char* outpath = NULL)
{
	//Load Image
	Mat origin = imread(fg_path);
	if (origin.empty() || bg_path == NULL)
		return;

	//Start
	srand(time(NULL));

#define APPLY_ROTATE
#define APPLY_RATIO_CHANGE

	//ratio change
#ifdef APPLY_RATIO_CHANGE
	int ratiochange = 1;//rand() % 2;
	if (ratiochange)
	{
		int width = origin.cols;
		int height = origin.rows;
		if (width > height)
			height = height + (width - height) * 0.2 + rand() % int((width - height) * 0.4);
		else if (width < height)
			width = width + (height - width) * 0.2 + rand() % int((height - width) * 0.4);
		resize(origin, origin, Size(width, height));
	}
#endif
	//1.initialize size
	int size = 500;//rand() % 300 + 256;
	size -= (size % 4);
	//2.foreground calibration
	Mat fg = pad(origin);
	Mat tmp = threshold_color(fg);
	resize(fg, fg, Size(size, size));
	resize(tmp, tmp, Size(size, size));
	//fg = equalizeIntensity(fg);
	fg.copyTo(origin);
	//
	blur(tmp, tmp, Size(3, 3));
	Mat mask = getMask(tmp);
	cvtColor(mask, mask, CV_GRAY2BGR);
	////rotate

#ifdef APPLY_ROTATE	
	int enableRotate = rand() % 2;
	if (enableRotate)
	{
		int angle = rand() % 8 - 4;
		fg = Rotation(fg, angle);
		mask = Rotation(mask, angle);
		size = fg.cols;
	}
#endif
	cvtColor(mask, mask, CV_BGR2GRAY);
	////blur
#ifdef APPLY_BLUR
	int enableBlur = rand() % 2;
	if (enableBlur)
		blur(fg, fg, Size(3, 3));
#endif

	int bg_file_count = getfilecount(bg_path);
	if (bg_path == NULL || bg_file_count == 0)
	{		
		imshow("foreground",fg);		
		waitKey(2000);
		return;
	}
	//3.background selection
	int bg_index = (rand() + rand()) % bg_file_count + 1;
	char format[] = ".png";
	Mat bg_org = imread(string(bg_dir) + int2string(bg_index) + format);
	int offset_x = 0, offset_y = 0;
	int bg_w = bg_org.cols > size ? bg_org.cols : size;
	int bg_h = bg_org.rows > size ? bg_org.rows : size;
	resize(bg_org, bg_org, Size(bg_w, bg_h));
	if (bg_org.cols != size)
		offset_x = rand() % (bg_org.cols - size);
	if (bg_org.rows != size)
		offset_y = rand() % (bg_org.rows - size);
	Mat bg(bg_org, Rect(offset_x, offset_y, size, size));
#ifdef _DEBUG
	imshow("fg", fg);
	imshow("bg", bg);
	imshow("mask", mask);
#endif
	//synthesize
	Mat img;
	bg.copyTo(img);
	for (int j = 0; j < size; j++)
	{
		for (int i = 0; i < size; i++)
		{
			if (*mask.ptr(j, i) > 128)
				memcpy(img.data + 3 * (j * size + i), fg.data + 3 * (j * size + i), 3);
		}
	}
	//
	if (outpath == NULL)
	{
		imshow("origin", origin);
		//imshow("mask", mask);
		imshow("out", img);
		waitKey(1000);
		return;
	}
	imwrite(outpath,img);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void doPadTest(const char *inpath, const char *outpath = NULL)
{
	Mat padding = pad(imread("H:\\Samples\\Remotes\\Real\\ah59-02547b-real.png"));
	resize(padding, padding, Size(256, 256));
#ifdef _DEBUG
	imshow("padding",padding);
	waitKey(3000);
#endif
	if(outpath != NULL) imwrite("H:\\ah59-02547b-real-padded.png", padding);
}

void doSomething(const char* inpath, const char* outpath = NULL)
{
	doPadTest(inpath, outpath);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
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
		if (!strcmp(dir_entry_p->d_name, ".") || !strcmp(dir_entry_p->d_name, ".."))
			continue;
		if (strcmp(dir_entry_p->d_name, ""))
			fprintf(stderr, "Examining file %s\n", dir_entry_p->d_name);
		/* TODO: Assign postfix/prefix info */
		strPostfix = "";
		strPrefix = input_directory;
		strPrefix += dir_entry_p->d_name;
		fprintf(stderr, "Loading image %s\n", strPrefix.c_str());

		//if (String(dir_entry_p->d_name) != String("tornado-v5.jpg"))
		//	continue;
	//#define TEST_SPECIAL
#ifdef TEST_SPECIAL
		string processtag = "";//"processed\\";
		string outpath = output_directory + processtag + dir_entry_p->d_name;
		output_file = outpath.substr(0, outpath.find_last_of(".")) + "_masked.png";
		doSomething(strPrefix.c_str(), output_file.c_str());
#else		

		string model_name = string(dir_entry_p->d_name);
		model_name = model_name.substr(0, model_name.find_last_of("."));
		string outpath = output_directory + model_name + "\\";// + dir_entry_p->d_name;
		fprintf(stderr, "Processing image %s\n", strPrefix.c_str());
		string cmd = "mkdir " + outpath.substr(0, outpath.find_last_of("\\"));
		system(cmd.data());
		int sample_count = 1000;
		for (int i = 0; i < sample_count; i++)
		{
			output_file = outpath + int2string(i) + ".png";
			//generateTrainData(strPrefix.c_str(), bg_dir);
			generateTrainData(strPrefix.c_str(), bg_dir, output_file.c_str());
		}
#endif
	}
}


///////////////
#define DIR_MODE
int main(int argc, char** argv)
{
#ifdef DIR_MODE
	string inpath = dir; inpath += subdir;
	string outpath = dir; outpath += outdir;
	processDir(inpath, outpath);
#else
	string fullpath;
	if (argc == 2)
		fullpath = argv[1];
	else
	{
		fullpath = dir;
		fullpath += subdir;
		fullpath += filename;
	}
	//doSomething(fullpath.c_str());
	//doSomething(fullpath.c_str(),bg_dir);

#endif
	return 0;
}

