// DirProcessing.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
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

void saveimage(const char* inpath, const char* outpath = NULL)
{
	Mat img = imread(inpath);
	if (img.empty())
		return;
	if(outpath) 
		imwrite(outpath, img);
}
std::string int2string(int num)
{
	std::ostringstream out_stream;
	out_stream << num;
	return out_stream.str();
}
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
	int fileindex = 0;
	ofstream output(output_file.c_str());

	string outpath, outdir, cmd, dir_name = "ready";
	outdir = output_directory + dir_name;
	cmd = "mkdir " + outdir;
	system(cmd.data());

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

		fprintf(stderr, "Processing image %s\n", strPrefix.c_str());

		fileindex++;
		output_file = outdir + "\\" + int2string(fileindex) + ".png";
		saveimage(strPrefix.c_str(), output_file.c_str());
	}
}

const char *input_dir = "H:\\Samples\\bkg-indoor\\bg-files\\";
const char *output_dir = "H:\\Samples\\bkg-indoor\\bg-files\\";

int main(int argc, char** argv)
{
	processDir(input_dir, output_dir);
	return 0;
}

