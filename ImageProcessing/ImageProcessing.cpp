// ImageProcessing.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_OPENCV
#include <opencv/cv.h>  
#include <opencv/cvaux.h>  
#include <opencv2\highgui\highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/utility.hpp>
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
#pragma comment(lib, "common_debug.lib")
#else
#pragma comment(lib, "common.lib")
#endif

using namespace std;

#define _DEBUG
////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////
void Resize(const char *inpath, const char *outpath = NULL)
{
	Mat in = imread(inpath),out;
	if (in.empty())
		return;

#define OUTPUT_SIZE 224
	resize(in, out, cv::Size(OUTPUT_SIZE, OUTPUT_SIZE));
#ifdef _DEBUG
	imshow("resized", out);
	waitKey(1000);
#endif
	if (outpath != NULL)
		imwrite(outpath, out);

}

void testGoodFeatures(const char *inpath, const char *outpath = NULL)
{
	Mat in = imread(inpath), grayImg;
	if (in.empty())
		return;

	if (in.type() == CV_8UC3)
		cvtColor(in, grayImg, CV_BGR2GRAY);
	else if (in.type() == CV_8UC1)
		grayImg = in;
	else
		return;

	resize(grayImg, grayImg, cv::Size(600, 300));
	blur(grayImg, grayImg, Size(3, 3));

	Mat gf_space = Mat::zeros(grayImg.rows, grayImg.cols, CV_8UC1);
	std::vector<cv::Point2f> pointlist = ocr_test_goodfeatures_as_points(grayImg);
	for (unsigned int i = 0; i< pointlist.size(); ++i)
	{
		cv::ellipse(gf_space, pointlist[i], cv::Size(2, 1), 0, 0, 360, cv::Scalar(255), 2, 8, 0);
	}

	Mat otsu;
	threshold(grayImg, otsu, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);
#ifdef _DEBUG
	imshow("grayImg",grayImg);
	imshow("otsu", otsu);
	imshow("goodfeature_space", gf_space);
	waitKey(10000);
#endif
	if (outpath != NULL)
		imwrite(outpath, gf_space);

}

void doSomething(const char *inpath, const char *outpath = NULL)
{
#ifdef _DEBUG
	testGoodFeatures(inpath);
#else
	testGoodFeatures(inpath,outpath);
#endif
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

#ifdef MAKE_OUTDIR
		string cmd = "mkdir " + outpath.substr(0, outpath.find_last_of("\\"));
		system(cmd.data());
#endif
		doSomething(strPrefix.c_str(), outpath.c_str());
#endif
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
static const char * indir = "H:\\Samples\\DoGCat\\cats\\";
static const char * outdir = "H:\\Samples\\DoGCat\\fined\\cats\\";
const char *filepath = "";
////////////////////////////////////////////////////////////////////////////

//watershed

//#define TEST_WATERSHED_TWO
//#define TEST_GRABCUT_ONE
#define TEST_GRABCUT_TWO
#ifdef TEST_GOOD_FEATURES

#include <cstdio>
#include <iostream>

static void help()
{
	cout << "\nThis program demonstrates the famous watershed segmentation algorithm in OpenCV: watershed()\n"
		"Usage:\n"
		"./watershed [image_name -- default is ../data/fruits.jpg]\n" << endl;
	cout << "Hot keys: \n"
		"\tESC - quit the program\n"
		"\tr - restore the original image\n"
		"\tw or SPACE - run watershed segmentation algorithm\n"
		"\t\t(before running it, *roughly* mark the areas to segment on the image)\n"
		"\t  (before that, roughly outline several markers on the image)\n";
}
Mat markerMask, img;
Point prevPt(-1, -1);
static void onMouse(int event, int x, int y, int flags, void*)
{
	if (x < 0 || x >= img.cols || y < 0 || y >= img.rows)
		return;
	if (event == EVENT_LBUTTONUP || !(flags & EVENT_FLAG_LBUTTON))
		prevPt = Point(-1, -1);
	else if (event == EVENT_LBUTTONDOWN)
		prevPt = Point(x, y);
	else if (event == EVENT_MOUSEMOVE && (flags & EVENT_FLAG_LBUTTON))
	{
		Point pt(x, y);
		if (prevPt.x < 0)
			prevPt = pt;
		line(markerMask, prevPt, pt, Scalar::all(255), 5, 8, 0);
		line(img, prevPt, pt, Scalar::all(255), 5, 8, 0);
		prevPt = pt;
		imshow("image", img);
	}
}


int main(int argc, char** argv)
{
	string inpath;
	if (argc == 2)
	{
		inpath = argv[1];
	}
	else{
		inpath = filepath;
	}
	doSomething(inpath.c_str());
	return 0;
}
#elif defined(TEST_WATERSHED_FIRST)
int main(int argc, char** argv)
{
	string filename;
	cv::CommandLineParser parser(argc, argv, "{help h | | }{ @input | ../data/fruits.jpg | }");
	if (parser.has("help"))
	{
		help();
		return 0;
	}
	if (argc < 2)
		filename = "H:\\DataProcessing\\ImageProcessing\\bin\\x64\\Release\\water_coins.jpg";
	else
		filename = parser.get<string>("@input");

	Mat img0 = imread(filename, 1), imgGray;
	if (img0.empty())
	{
		cout << "Couldn'g open image " << filename << ". Usage: watershed <image_name>\n";
		return 0;
	}
	help();
	namedWindow("image", 1);
	img0.copyTo(img);
	cvtColor(img, markerMask, COLOR_BGR2GRAY);
	cvtColor(markerMask, imgGray, COLOR_GRAY2BGR);
	markerMask = Scalar::all(0);
	imshow("image", img);
	setMouseCallback("image", onMouse, 0);
	for (;;)
	{
		char c = (char)waitKey(0);
		if (c == 27)
			break;
		if (c == 'r')
		{
			markerMask = Scalar::all(0);
			img0.copyTo(img);
			imshow("image", img);
		}
		if (c == 'w' || c == ' ')
		{
			int i, j, compCount = 0;
			vector<vector<Point> > contours;
			vector<Vec4i> hierarchy;
			findContours(markerMask, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE);
			if (contours.empty())
				continue;
			Mat markers(markerMask.size(), CV_32S);
			markers = Scalar::all(0);
			int idx = 0;
			for (; idx >= 0; idx = hierarchy[idx][0], compCount++)
				drawContours(markers, contours, idx, Scalar::all(compCount + 1), -1, 8, hierarchy, INT_MAX);
			if (compCount == 0)
				continue;
			vector<Vec3b> colorTab;
			for (i = 0; i < compCount; i++)
			{
				int b = theRNG().uniform(0, 255);
				int g = theRNG().uniform(0, 255);
				int r = theRNG().uniform(0, 255);
				colorTab.push_back(Vec3b((uchar)b, (uchar)g, (uchar)r));
			}

			double t = (double)getTickCount();
			watershed(img0, markers);
			t = (double)getTickCount() - t;
			printf("execution time = %gms\n", t*1000. / getTickFrequency());
			Mat wshed(markers.size(), CV_8UC3);
			// paint the watershed image
			for (i = 0; i < markers.rows; i++)
			for (j = 0; j < markers.cols; j++)
			{
				int index = markers.at<int>(i, j);
				if (index == -1)
					wshed.at<Vec3b>(i, j) = Vec3b(255, 255, 255);
				else if (index <= 0 || index > compCount)
					wshed.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
				else
					wshed.at<Vec3b>(i, j) = colorTab[index - 1];
			}
			wshed = wshed*0.5 + imgGray*0.5;
			imshow("watershed transform", wshed);
		}
	}
	return 0;
}
#elif defined(TEST_WATERSHED_TWO)
int main(int, char** argv)
{
	// Load the image
	Mat src = imread(argv[1]);
	// Check if everything was fine
	if (!src.data)
		return -1;
	// Show source image
	imshow("Source Image", src);
	// Change the background from white to black, since that will help later to extract
	// better results during the use of Distance Transform
	for (int x = 0; x < src.rows; x++) {
		for (int y = 0; y < src.cols; y++) {
			if (src.at<Vec3b>(x, y) == Vec3b(255, 255, 255)) {
				src.at<Vec3b>(x, y)[0] = 0;
				src.at<Vec3b>(x, y)[1] = 0;
				src.at<Vec3b>(x, y)[2] = 0;
			}
		}
	}
	// Show output image
	imshow("Black Background Image", src);
	// Create a kernel that we will use for accuting/sharpening our image
	Mat kernel = (Mat_<float>(3, 3) <<
		1, 1, 1,
		1, -8, 1,
		1, 1, 1); // an approximation of second derivative, a quite strong kernel
	// do the laplacian filtering as it is
	// well, we need to convert everything in something more deeper then CV_8U
	// because the kernel has some negative values,
	// and we can expect in general to have a Laplacian image with negative values
	// BUT a 8bits unsigned int (the one we are working with) can contain values from 0 to 255
	// so the possible negative number will be truncated
	Mat imgLaplacian;
	Mat sharp = src; // copy source image to another temporary one
	filter2D(sharp, imgLaplacian, CV_32F, kernel);
	src.convertTo(sharp, CV_32F);
	Mat imgResult = sharp - imgLaplacian;
	// convert back to 8bits gray scale
	imgResult.convertTo(imgResult, CV_8UC3);
	imgLaplacian.convertTo(imgLaplacian, CV_8UC3);
	// imshow( "Laplace Filtered Image", imgLaplacian );
	imshow("New Sharped Image", imgResult);
	src = imgResult; // copy back
	// Create binary image from source image
	Mat bw;
	cvtColor(src, bw, CV_BGR2GRAY);
	threshold(bw, bw, 40, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	imshow("Binary Image", bw);
	// Perform the distance transform algorithm
	Mat dist;
	distanceTransform(bw, dist, CV_DIST_L2, 3);
	// Normalize the distance image for range = {0.0, 1.0}
	// so we can visualize and threshold it
	normalize(dist, dist, 0, 1., NORM_MINMAX);
	imshow("Distance Transform Image", dist);
	// Threshold to obtain the peaks
	// This will be the markers for the foreground objects
	threshold(dist, dist, .4, 1., CV_THRESH_BINARY);
	// Dilate a bit the dist image
	Mat kernel1 = Mat::ones(3, 3, CV_8UC1);
	dilate(dist, dist, kernel1);
	imshow("Peaks", dist);
	// Create the CV_8U version of the distance image
	// It is needed for findContours()
	Mat dist_8u;
	dist.convertTo(dist_8u, CV_8U);
	// Find total markers
	vector<vector<Point> > contours;
	findContours(dist_8u, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	// Create the marker image for the watershed algorithm
	Mat markers = Mat::zeros(dist.size(), CV_32SC1);
	// Draw the foreground markers
	for (size_t i = 0; i < contours.size(); i++)
		drawContours(markers, contours, static_cast<int>(i), Scalar::all(static_cast<int>(i)+1), -1);
	// Draw the background marker
	circle(markers, Point(5, 5), 3, CV_RGB(255, 255, 255), -1);
	imshow("Markers", markers * 10000);
	// Perform the watershed algorithm
	watershed(src, markers);
	Mat mark = Mat::zeros(markers.size(), CV_8UC1);
	markers.convertTo(mark, CV_8UC1);
	bitwise_not(mark, mark);
	//    imshow("Markers_v2", mark); // uncomment this if you want to see how the mark
	// image looks like at that point
	// Generate random colors
	vector<Vec3b> colors;
	for (size_t i = 0; i < contours.size(); i++)
	{
		int b = theRNG().uniform(0, 255);
		int g = theRNG().uniform(0, 255);
		int r = theRNG().uniform(0, 255);
		colors.push_back(Vec3b((uchar)b, (uchar)g, (uchar)r));
	}
	// Create the result image
	Mat dst = Mat::zeros(markers.size(), CV_8UC3);
	// Fill labeled objects with random colors
	for (int i = 0; i < markers.rows; i++)
	{
		for (int j = 0; j < markers.cols; j++)
		{
			int index = markers.at<int>(i, j);
			if (index > 0 && index <= static_cast<int>(contours.size()))
				dst.at<Vec3b>(i, j) = colors[index - 1];
			else
				dst.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
		}
	}
	// Visualize the final image
	imshow("Final Result", dst);
	waitKey(0);
	return 0;
}
#elif defined(TEST_GRABCUT_ONE)
int main(int, char** argv)
{
	// Open another image
	Mat image;
	image = imread(argv[1]);

	if (!image.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	// define bounding rectangle
	int border = 20;
	int border2 = border + border;
	cv::Rect rectangle(border, border, image.cols - border2, image.rows - border2);

	cv::Mat result; // segmentation result (4 possible values)
	cv::Mat bgModel, fgModel; // the models (internally used)

	// GrabCut segmentation
	cv::grabCut(image,    // input image
		result,   // segmentation result
		rectangle,// rectangle containing foreground 
		bgModel, fgModel, // models
		1,        // number of iterations
		cv::GC_INIT_WITH_RECT); // use rectangle
	// Get the pixels marked as likely foreground
	cv::compare(result, cv::GC_PR_FGD, result, cv::CMP_EQ);
	// Generate output image
	cv::Mat foreground(image.size(), CV_8UC3, cv::Scalar(255, 255, 255));
	image.copyTo(foreground, result); // bg pixels not copied

	// draw rectangle on original image
	cv::rectangle(image, rectangle, cv::Scalar(255, 255, 255), 1);
	cv::namedWindow("Image");
	cv::imshow("Image", image);

	// display result
	cv::namedWindow("Segmented Image");
	cv::imshow("Segmented Image", foreground);


	waitKey();
	return 0;

}
#elif defined(TEST_GRABCUT_TWO)
static void help()
{
	cout << "\nThis program demonstrates GrabCut segmentation -- select an object in a region\n"
		"and then grabcut will attempt to segment it out.\n"
		"Call:\n"
		"./grabcut <image_name>\n"
		"\nSelect a rectangular area around the object you want to segment\n" <<
		"\nHot keys: \n"
		"\tESC - quit the program\n"
		"\tr - restore the original image\n"
		"\tn - next iteration\n"
		"\n"
		"\tleft mouse button - set rectangle\n"
		"\n"
		"\tCTRL+left mouse button - set GC_BGD pixels\n"
		"\tSHIFT+left mouse button - set GC_FGD pixels\n"
		"\n"
		"\tCTRL+right mouse button - set GC_PR_BGD pixels\n"
		"\tSHIFT+right mouse button - set GC_PR_FGD pixels\n" << endl;
}

const Scalar RED = Scalar(0, 0, 255);
const Scalar PINK = Scalar(230, 130, 255);
const Scalar BLUE = Scalar(255, 0, 0);
const Scalar LIGHTBLUE = Scalar(255, 255, 160);
const Scalar GREEN = Scalar(0, 255, 0);

const int BGD_KEY = EVENT_FLAG_CTRLKEY;
const int FGD_KEY = EVENT_FLAG_SHIFTKEY;

static void getBinMask(const Mat& comMask, Mat& binMask)
{
	if (comMask.empty() || comMask.type() != CV_8UC1)
		CV_Error(Error::StsBadArg, "comMask is empty or has incorrect type (not CV_8UC1)");
	if (binMask.empty() || binMask.rows != comMask.rows || binMask.cols != comMask.cols)
		binMask.create(comMask.size(), CV_8UC1);
	binMask = comMask & 1;
}

class GCApplication
{
public:
	enum{ NOT_SET = 0, IN_PROCESS = 1, SET = 2 };
	static const int radius = 2;
	static const int thickness = -1;

	void reset();
	void setImageAndWinName(const Mat& _image, const string& _winName);
	void showImage() const;
	void mouseClick(int event, int x, int y, int flags, void* param);
	int nextIter();
	int getIterCount() const { return iterCount; }
private:
	void setRectInMask();
	void setLblsInMask(int flags, Point p, bool isPr);

	const string* winName;
	const Mat* image;
	Mat mask;
	Mat bgdModel, fgdModel;

	uchar rectState, lblsState, prLblsState;
	bool isInitialized;

	Rect rect;
	vector<Point> fgdPxls, bgdPxls, prFgdPxls, prBgdPxls;
	int iterCount;
};

void GCApplication::reset()
{
	if (!mask.empty())
		mask.setTo(Scalar::all(GC_BGD));
	bgdPxls.clear(); fgdPxls.clear();
	prBgdPxls.clear();  prFgdPxls.clear();

	isInitialized = false;
	rectState = NOT_SET;
	lblsState = NOT_SET;
	prLblsState = NOT_SET;
	iterCount = 0;
}

void GCApplication::setImageAndWinName(const Mat& _image, const string& _winName)
{
	if (_image.empty() || _winName.empty())
		return;
	image = &_image;
	winName = &_winName;
	mask.create(image->size(), CV_8UC1);
	reset();
}

void GCApplication::showImage() const
{
	if (image->empty() || winName->empty())
		return;

	Mat res;
	Mat binMask;
	if (!isInitialized)
		image->copyTo(res);
	else
	{
		getBinMask(mask, binMask);
		image->copyTo(res, binMask);
	}

	vector<Point>::const_iterator it;
	for (it = bgdPxls.begin(); it != bgdPxls.end(); ++it)
		circle(res, *it, radius, BLUE, thickness);
	for (it = fgdPxls.begin(); it != fgdPxls.end(); ++it)
		circle(res, *it, radius, RED, thickness);
	for (it = prBgdPxls.begin(); it != prBgdPxls.end(); ++it)
		circle(res, *it, radius, LIGHTBLUE, thickness);
	for (it = prFgdPxls.begin(); it != prFgdPxls.end(); ++it)
		circle(res, *it, radius, PINK, thickness);

	if (rectState == IN_PROCESS || rectState == SET)
		rectangle(res, Point(rect.x, rect.y), Point(rect.x + rect.width, rect.y + rect.height), GREEN, 2);

	imshow(*winName, res);
}

void GCApplication::setRectInMask()
{
	CV_Assert(!mask.empty());
	mask.setTo(GC_BGD);
	rect.x = max(0, rect.x);
	rect.y = max(0, rect.y);
	rect.width = min(rect.width, image->cols - rect.x);
	rect.height = min(rect.height, image->rows - rect.y);
	(mask(rect)).setTo(Scalar(GC_PR_FGD));
}

void GCApplication::setLblsInMask(int flags, Point p, bool isPr)
{
	vector<Point> *bpxls, *fpxls;
	uchar bvalue, fvalue;
	if (!isPr)
	{
		bpxls = &bgdPxls;
		fpxls = &fgdPxls;
		bvalue = GC_BGD;
		fvalue = GC_FGD;
	}
	else
	{
		bpxls = &prBgdPxls;
		fpxls = &prFgdPxls;
		bvalue = GC_PR_BGD;
		fvalue = GC_PR_FGD;
	}
	if (flags & BGD_KEY)
	{
		bpxls->push_back(p);
		circle(mask, p, radius, bvalue, thickness);
	}
	if (flags & FGD_KEY)
	{
		fpxls->push_back(p);
		circle(mask, p, radius, fvalue, thickness);
	}
}

void GCApplication::mouseClick(int event, int x, int y, int flags, void*)
{
	// TODO add bad args check
	switch (event)
	{
	case EVENT_LBUTTONDOWN: // set rect or GC_BGD(GC_FGD) labels
	{
								bool isb = (flags & BGD_KEY) != 0,
									isf = (flags & FGD_KEY) != 0;
								if (rectState == NOT_SET && !isb && !isf)
								{
									rectState = IN_PROCESS;
									rect = Rect(x, y, 1, 1);
								}
								if ((isb || isf) && rectState == SET)
									lblsState = IN_PROCESS;
	}
		break;
	case EVENT_RBUTTONDOWN: // set GC_PR_BGD(GC_PR_FGD) labels
	{
								bool isb = (flags & BGD_KEY) != 0,
									isf = (flags & FGD_KEY) != 0;
								if ((isb || isf) && rectState == SET)
									prLblsState = IN_PROCESS;
	}
		break;
	case EVENT_LBUTTONUP:
		if (rectState == IN_PROCESS)
		{
			rect = Rect(Point(rect.x, rect.y), Point(x, y));
			rectState = SET;
			setRectInMask();
			CV_Assert(bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty());
			showImage();
		}
		if (lblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), false);
			lblsState = SET;
			showImage();
		}
		break;
	case EVENT_RBUTTONUP:
		if (prLblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), true);
			prLblsState = SET;
			showImage();
		}
		break;
	case EVENT_MOUSEMOVE:
		if (rectState == IN_PROCESS)
		{
			rect = Rect(Point(rect.x, rect.y), Point(x, y));
			CV_Assert(bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty());
			showImage();
		}
		else if (lblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), false);
			showImage();
		}
		else if (prLblsState == IN_PROCESS)
		{
			setLblsInMask(flags, Point(x, y), true);
			showImage();
		}
		break;
	}
}

int GCApplication::nextIter()
{
	if (isInitialized)
		grabCut(*image, mask, rect, bgdModel, fgdModel, 1);
	else
	{
		if (rectState != SET)
			return iterCount;

		if (lblsState == SET || prLblsState == SET)
			grabCut(*image, mask, rect, bgdModel, fgdModel, 1, GC_INIT_WITH_MASK);
		else
			grabCut(*image, mask, rect, bgdModel, fgdModel, 1, GC_INIT_WITH_RECT);

		isInitialized = true;
	}
	iterCount++;

	bgdPxls.clear(); fgdPxls.clear();
	prBgdPxls.clear(); prFgdPxls.clear();

	return iterCount;
}

GCApplication gcapp;

static void on_mouse(int event, int x, int y, int flags, void* param)
{
	gcapp.mouseClick(event, x, y, flags, param);
}

int main(int argc, char** argv)
{
	cv::CommandLineParser parser(argc, argv, "{help h||}{@input||}");
	if (parser.has("help"))
	{
		help();
		return 0;
	}
	string filename = parser.get<string>("@input");
	if (filename.empty())
	{
		cout << "\nDurn, empty filename" << endl;
		return 1;
	}
	Mat image = imread(filename, 1);
	if (image.empty())
	{
		cout << "\n Durn, couldn't read image filename " << filename << endl;
		return 1;
	}

	help();

	const string winName = "image";
	namedWindow(winName, WINDOW_AUTOSIZE);
	setMouseCallback(winName, on_mouse, 0);

	gcapp.setImageAndWinName(image, winName);
	gcapp.showImage();

	for (;;)
	{
		char c = (char)waitKey(0);
		switch (c)
		{
		case '\x1b':
			cout << "Exiting ..." << endl;
			goto exit_main;
		case 'r':
			cout << endl;
			gcapp.reset();
			gcapp.showImage();
			break;
		case 'n':
			int iterCount = gcapp.getIterCount();
			cout << "<" << iterCount << "... ";
			int newIterCount = gcapp.nextIter();
			if (newIterCount > iterCount)
			{
				gcapp.showImage();
				cout << iterCount << ">" << endl;
			}
			else
				cout << "rect must be determined>" << endl;
			break;
		}
	}

exit_main:
	destroyWindow(winName);
	return 0;
}
#endif
