#include "stdafx.h"
#include "transform.h"
#include <opencv2\highgui\highgui.hpp>
#include <opencv2/core/core.hpp>
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include <cvb_blob_list.h>
#include <ctime>
using namespace cv;

Mat pad(Mat origin)
{
	if (origin.empty())
		return Mat();

	int newsize = 0;
	if (origin.rows >= origin.cols)
		newsize = origin.rows;
	else
		newsize = origin.cols;

	srand(time(NULL));

#define MIN_MARGIN 40

	int margin = rand() % (newsize / 5) + MIN_MARGIN;
	newsize += margin;
	int offset_x = (newsize - origin.cols) / 2;
	
	Mat img;
	if (img.type() == CV_8UC3)
	{
		img = Mat(newsize, newsize, CV_8UC1);
		memset(img.data, 0, newsize * newsize);

		for (int j = 0; j < origin.rows; j++)
		{
			memcpy(img.data + (j + margin / 2) * newsize + offset_x, origin.data + j * origin.cols, origin.cols);
		}
	}
	else {
		img = Mat(newsize, newsize, CV_8UC3);
		memset(img.data, 0, 3 * newsize * newsize);

		for (int j = 0; j < origin.rows; j++)
		{
			memcpy(img.data + (j + margin / 2) * newsize * 3 + offset_x * 3, origin.data + j * origin.cols * 3, 3 * origin.cols);
		}

		//for (int j = 0; j < newsize; j++)
		//{
		//	for (int i = 0; i < newsize; i++)
		//	{
		//		int sum = 0, k = 0;
		//		for (k = 0; k < 3; k++)
		//			sum += *(img.ptr(j, i) + k);
		//		if (sum / (k + 1) > 180)
		//			memset(img.data + 3 * (j * img.cols + i), 0, 3);
		//	}
		//}
	}


	return img;
}
//rotate
#define PI 3.141592
#define RATIO PI/180.0
Mat Rotation(Mat in, double angle, double scale)
{
	Mat out;
	//Corr1
	int nRadius, nWidth, nHeight, nHalfWidth, nHalfHeight, nWidth_, nHeight_, nHalfWidth_, nHalfHeight_, nHalfWidth_tmp, nHalfHeight_tmp, nMIN;
	nWidth = in.cols;
	nHeight = in.rows;
	nHalfWidth = nWidth >> 1;
	nHalfHeight = nHeight >> 1;
	nRadius = (int)sqrt(double(nHalfWidth * nHalfWidth + nHalfHeight * nHalfHeight));
	double angle_;
	angle_ = atan2(double(nHalfHeight), double(nHalfWidth));
	float fAngle1, fAngle2;
	fAngle1 = float(angle * RATIO + angle_);
	fAngle2 = float(angle * RATIO - angle_);
	nMIN = MIN(nHalfWidth, nHalfHeight);
	nHalfWidth_ = (int)abs(nRadius * cos(fAngle1));//
	nHalfWidth_tmp = (int)abs(nRadius * cos(fAngle2));
	nHalfHeight_ = (int)abs(nRadius * sin(fAngle1));//
	nHalfHeight_tmp = (int)abs(nRadius * sin(fAngle2));//
	nHalfWidth_ = MAX(nHalfWidth, MAX(nHalfWidth_, nHalfWidth_tmp));
	nHalfHeight_ = MAX(nHalfHeight, MAX(nHalfHeight_, nHalfHeight_tmp));
	//nHalfWidth_ = nHalfWidth_ / 4 * 4;
	//nHalfHeight_ = nHalfHeight_ / 4 * 4;
	nWidth_ = nHalfWidth_ << 1;
	nHeight_ = nHalfHeight_ << 1;
	Mat in_corr(nHeight_, nWidth_, CV_8UC3);
	int DeltaX, DeltaY;
	DeltaX = abs(nHalfWidth_ - nHalfWidth);
	DeltaY = abs(nHalfHeight_ - nHalfHeight);
	for (int j = 0; j < nHeight_; j++)
	for (int i = 0; i < nWidth_; i++)
	for (int k = 0; k < 3; k++)
	{
		if (j < DeltaY ||
			j >= (nHeight_ - DeltaY) ||
			i < DeltaX ||
			i >= (nWidth_ - DeltaX))
			*(in_corr.ptr(j, i) + k) = 0;
		else{
			*(in_corr.ptr(j, i) + k) = *(in.ptr(j - DeltaY, i - DeltaX) + k);
		}
	}
	//imshow("test",in_corr);
	cv::Point center = cv::Point(nHalfWidth_, nHalfHeight_);
	Mat rotationMat(2, 3, CV_32FC1);
	rotationMat = getRotationMatrix2D(center, angle, scale);
	warpAffine(in_corr, out, rotationMat, out.size());
	//
	if ((out.cols % 4) || (out.rows % 4))
	{
		cv::resize(out, out, cv::Size(in_corr.cols / 4 * 4, in_corr.rows / 4 * 4));
	}
	//
	return out;
}
//
Mat threshold_color(Mat img)
{
	if (img.empty())
		return Mat();

	Mat filtered;
	img.copyTo(filtered);
	for (int j = 0; j < img.rows; j++)
	{
		for (int i = 0; i < img.cols; i++)
		{
			int sum = 0, k = 0;
			for (k = 0; k < 3; k++)
				sum += *(img.ptr(j, i) + k);
			if (sum / (k + 1) > 180)
				memset(filtered.data + 3 * (j * img.cols + i), 0, 3);
		}
	}
	return filtered;
}
//
//cvblobTest
void cvBlobTest_clr(Mat img)
{
	if (img.empty())
		return;

	cv::Mat grey;
	if (img.type() == CV_8UC1)
	{
		img.copyTo(grey);
		cvtColor(img, img, CV_GRAY2BGR);
	}
	else {
		grey.create(img.size(), CV_8UC1);
		cv::cvtColor(img, grey, CV_BGR2GRAY, 1);
	}

	cv::threshold(grey, grey, 0, 255, CV_THRESH_BINARY_INV + CV_THRESH_OTSU);

	LARGE_INTEGER freq;
	LARGE_INTEGER t0, tF, tDiff;
	double elapsedTime;

	// cvblob
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&t0);

	cvb::BlobList blobs;
	blobs.SimpleLabel(grey);
	blobs.FilterByArea(50, img.cols * img.rows / 4);

	QueryPerformanceCounter(&tF);
	tDiff.QuadPart = tF.QuadPart - t0.QuadPart;
	elapsedTime = tDiff.QuadPart / (double)freq.QuadPart;
	std::cout << "cvblob:" << elapsedTime << std::endl;

	for (auto &a_blob : blobs.get_BlobsList()) {
		// extract and draws blob contour
		auto contours = a_blob->get_Contour().get_ContourPolygon();
		if (contours.size() != 1) {
			for (auto &iter = contours.begin(); iter != contours.end(); iter++) {
				auto next_iter = iter;
				next_iter++;
				if (next_iter == contours.end())
					cv::line(img, *iter, contours[0], cv::Scalar(0, 0, 255), 1, 8);
				else
					cv::line(img, *iter, *next_iter, cv::Scalar(0, 0, 255), 1, 8);
			}
		}

		// extract and draws every blob internal hole contour
		auto internal_contours = a_blob->get_InternalContours();
		for (auto &a_contour : internal_contours) {
			auto contour_dot = a_contour->get_ContourPolygon();
			if (contour_dot.size() == 1)
				continue;
			for (auto &iter = contour_dot.begin(); iter != contour_dot.end(); iter++) {
				auto next_iter = iter;
				next_iter++;
				if (next_iter == contour_dot.end())
					cv::line(img, *iter, contour_dot[0], cv::Scalar(0, 255, 0), 1, 8);
				else
					cv::line(img, *iter, *next_iter, cv::Scalar(0, 255, 0), 1, 8);
			}
		}

		// draws bounding box
		cv::rectangle(img, a_blob->get_BoundingBox(), cv::Scalar(255, 0, 255));

		// draws centroid
		cv::circle(img, a_blob->get_Centroid(), 2, cv::Scalar(255, 0, 0), 2);
	};

	cv::imwrite("imggray.png", grey);
	cv::imwrite("imgout.png", img);
	cv::imwrite("imglabel.png", blobs.get_ImageLabel());
	cv::imshow("imggray.png", grey);
	cv::imshow("imgout.png", img);
	cv::imshow("imglabel.png", blobs.get_ImageLabel());
	cvWaitKey(2000);
}
//
Mat getMask(Mat img)
{
	if (img.empty())
		return Mat();

	Mat gray,thr;
	if (img.type() != CV_8UC1)
		cvtColor(img, gray, CV_BGR2GRAY);
	else
		gray = img;

	threshold(gray, thr, 1, 255, CV_THRESH_BINARY);//threshold(gray, thr, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);
	erode(thr, thr, Mat());
	erode(thr, thr, Mat());
	//imshow("thr", thr);
	//cvBlobTest_clr(thr);
	for (int j = 0; j < img.rows; j++)
	{
		
		int non_zero_start = 0, non_zero_end = 0;
		for (int i = 0; i < img.cols; i++)
		{
			if (*thr.ptr(j, i) > 180)
			{
				if (non_zero_start == 0)
					non_zero_start = i + 1;
				non_zero_end = i - 1;
			}
		}
		if (non_zero_end > non_zero_start)
			memset(thr.data + j * thr.cols + non_zero_start, 255, non_zero_end - non_zero_start);
	}

	return thr;
}