/*****************************************************************************
 * Bio Image Operation
 * Copyright (C) 2013-2018 Joost de Folter <folterj@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#include "BenchMark.h"
#include "ImageOperations.h"
#include "Averager.h"
#include "VideoSource.h"
#include "VideoOutput.h"
#include "Util.h"

#pragma unmanaged
#include "opencv2/opencv.hpp"
#include "opencv2/core/cuda.hpp"
#include "opencv2/core/ocl.hpp"
#pragma managed

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

using namespace System;
using namespace cv;
//using namespace cuda;

// opencv GPU acceleration with Thrust library https://docs.opencv.org/trunk/d8/db9/tutorial_gpu_thrust_interop.html

// save using (low level) ffmpeg directly: https://blog.lemberg.co.uk/how-process-live-video-stream-using-ffmpeg-and-opencv


BenchMark::BenchMark(Observer^ observer)
{
	this->observer = observer;

	bool useOptimised = useOptimized();
	bool useOpenCl = ocl::useOpenCL();
	bool haveOpenCl = ocl::haveOpenCL();
	int n = cuda::getCudaEnabledDeviceCount();		// 0 means not compiled with CUDA support!
	// OpenCL: https://opencv.org/platforms/opencl.html
	// GPU samples (including video reader!): https://github.com/opencv/opencv/tree/master/samples/gpu/
	// Compile OpenCV with CUDA support: https://jamesbowley.co.uk/build-compile-opencv-3-4-in-windows-with-cuda-9-0-and-intel-mkl-tbb/
}

BenchMark::~BenchMark()
{
	delete stopwatch;
	delete tracker;
	delete accumBuffer;
}

void BenchMark::reset()
{
	abort = false;
	tracker->reset();
	accumBuffer->reset();
	observer->resetProgressTimer();
}

void BenchMark::doTest()
{
	benchmarkTest();
	//gpuTest();
	//accumTest();
}

void BenchMark::benchmarkTest()
{
	long long time0 = 0;
	long long time1, time2, time3, time4, time5, time6, time7, time8, tottime;
	Averager t1, t2, t3, t4, t5, t6, t7, t8, ta, tt;
	VideoSource video;
	//VideoOutput videoOutput;
	Mat image, grayImage, difImage, binImage, binImage2, colorImage;
	bool done = false;
	bool clusterOk;
	int framei = 0;
	cv::String output;

	try
	{
		reset();

		Mat backImage = imread("D:\\Video\\Ants\\Copied 17 April\\background.tif", IMREAD_GRAYSCALE);
		Mat maskImage = imread("D:\\Video\\Ants\\Copied 17 April\\mask.tif", IMREAD_GRAYSCALE);

		if (!video.init(0, "D:\\Video\\Ants\\Copied 17 April\\", "test.MTS"))
			return;

		int nframes = video.nframes;

		//videoOutput.init("d:\\", "test.mp4", "MJPG", 50);	

		stopwatch->Restart();

		while (!done && !abort)
		{
			tottime = stopwatch->ElapsedMilliseconds - time0;
			time0 = stopwatch->ElapsedMilliseconds;

			if (video.getNextImage(&image))
			{
				time1 = stopwatch->ElapsedMilliseconds;
				t1.addValue((double)(time1 - time0));

				ImageOperations::convertToGrayScale(image, grayImage);
				time2 = stopwatch->ElapsedMilliseconds;
				t2.addValue((double)(time2 - time1));

				ImageOperations::difference(grayImage, backImage, difImage, true);
				time3 = stopwatch->ElapsedMilliseconds;
				t3.addValue((double)(time3 - time2));

				ImageOperations::threshold(difImage, binImage, 0.05);
				time4 = stopwatch->ElapsedMilliseconds;
				t4.addValue((double)(time4 - time3));

				ImageOperations::mask(binImage, maskImage, binImage2);
				time5 = stopwatch->ElapsedMilliseconds;
				t5.addValue((double)(time5 - time4));

				clusterOk = tracker->createClusters(&binImage2, 10, 30, "");
				time6 = stopwatch->ElapsedMilliseconds;
				t6.addValue((double)(time6 - time5));

				if (clusterOk)
				{
					tracker->createTracks(2, 3, 3, "");
					//tracker->createPaths(PathMode::Simple, 10);
				}
				time7 = stopwatch->ElapsedMilliseconds;
				t7.addValue((double)(time7 - time6));

				if (framei % 10 == 0)
				{
					
					//ImageOperations::convertToColor(binImage2, colorImage);
					//tracker->drawClusters(&colorImage);
					//tracker->drawTracks(&colorImage);

					//observer->displayImage(&binImage2, 0);
					//observer->displayImage(&colorImage, 0);

					//videoOutput.writeImage(&colorImage);					
				}
				time8 = stopwatch->ElapsedMilliseconds;
				t8.addValue((double)(time8 - time7));

				ta.addValue((double)(time8 - time0));
				tt.addValue((double)tottime);

				output = format("#%d V:%.1f G:%.1f D:%.1f T:%.1f M:%.1f Cl:%.1f Tr:%.1f S:%.1f T':%.1f (T:%.1f)",
					framei, t1.getAverage(), t2.getAverage(), t3.getAverage(), t4.getAverage(), t5.getAverage(), t6.getAverage(), t7.getAverage(), t8.getAverage(), ta.getAverage(), tt.getAverage());

				observer->showStatus(Util::netString(output), (double)framei / nframes);
				System::Windows::Forms::Application::DoEvents();							// this takes time too, and depends on amount work/updates; (~1 ms)

				framei++;
			}
			else
			{
				done = true;
			}
		}
		//imwrite("D:\\test.png", colorImage);
		//videoOutput.close();
	}
	catch (cv::Exception& e)
	{
		// opencv exception
		System::String^ errorMsg;
		if (e.err != "")
		{
			errorMsg = Util::netString(e.err);
		}
		else
		{
			errorMsg = Util::netString(e.msg);
		}
		observer->showErrorMessage(errorMsg);
		// error codes are in types_c.h, e.g. CV_StsUnsupportedFormat
	}
	catch (System::Exception^ e)
	{
		System::String^ errorMsg = Util::getExceptionDetail(e);
		observer->showErrorMessage(errorMsg);
	}
	doAbort();
}

void BenchMark::gpuTest()
{
	/*
	long long time0 = 0;
	long long time1, time2, time3, time4, time5, time6, time7, time8, tottime;
	Averager t1, t2, t3, t4, t5, t6, t7, t8, ta, tt;
	VideoCapture video;
	Mat sourceMat, binMat;
	GpuMat sourceImage, grayImage, difImage, binImage, binImage2, colorImage;
	bool done = false;
	bool videoOk, clusterOk;
	int framei = 0;
	cv::String output;

	reset();

	GpuMat backImage(imread("D:\\Video\\Ants\\Copied 17 April\\background.tif", IMREAD_GRAYSCALE));
	GpuMat maskImage(imread("D:\\Video\\Ants\\Copied 17 April\\mask.tif", IMREAD_GRAYSCALE));

	video.open("D:\\Video\\Ants\\Copied 17 April\\test.MTS");
	if (!video.isOpened())  // check if we succeeded
		return;
	
	int nframes = (int)video.get(VideoCaptureProperties::CAP_PROP_FRAME_COUNT);

	//cv::Ptr<cudacodec::VideoReader> cudaVideo = cudacodec::createVideoReader(std::string("D:\\Video\\Ants\\Copied 17 April\\test.MTS"));

	stopwatch->Restart();

	while (!done && !abort)
	{
		tottime = stopwatch->ElapsedMilliseconds - time0;
		time0 = stopwatch->ElapsedMilliseconds;

		videoOk = video.read(sourceMat);
		//videoOk = cudaVideo->nextFrame(sourceImage);
		sourceImage.upload(sourceMat);

		if (videoOk && !sourceImage.empty())
		{
			time1 = stopwatch->ElapsedMilliseconds;
			t1.addValue((double)(time1 - time0));

			cuda::cvtColor(sourceImage, grayImage, ColorConversionCodes::COLOR_BGR2GRAY);
			time2 = stopwatch->ElapsedMilliseconds;
			t2.addValue((double)(time2 - time1));

			cuda::absdiff(grayImage, backImage, difImage);
			time3 = stopwatch->ElapsedMilliseconds;
			t3.addValue((double)(time3 - time2));

			cuda::threshold(difImage, binImage, (0.05 * 0xFF), 0xFF, ThresholdTypes::THRESH_BINARY);
			time4 = stopwatch->ElapsedMilliseconds;
			t4.addValue((double)(time4 - time3));

			binImage.copyTo(binImage2, maskImage);
			time5 = stopwatch->ElapsedMilliseconds;
			t5.addValue((double)(time5 - time4));

			binImage2.download(binMat);

			clusterOk = tracker.createClusters(&binMat, 10, 30);
			time6 = stopwatch->ElapsedMilliseconds;
			t6.addValue((double)(time6 - time5));

			if (clusterOk)
			{
				tracker.createTracks(2, 3, 3);
			}
			time7 = stopwatch->ElapsedMilliseconds;
			t7.addValue((double)(time7 - time6));

			if (framei % 10 == 0)
			{
				//observer->displayImage(&binMat, 0);

				//cvtColor(binImage2, colorImage, ColorConversionCodes::COLOR_GRAY2BGR);
				//tracker.drawClusters(colorImage);
				//tracker.drawTracks(&colorImage);
				//observer->displayImage(&colorImage, 0);
			}
			time8 = stopwatch->ElapsedMilliseconds;
			t8.addValue((double)(time8 - time7));

			ta.addValue((double)(time8 - time0));
			tt.addValue((double)tottime);

			output = format("#%d V:%.1f G:%.1f D:%.1f T:%.1f M:%.1f Cl:%.1f Tr:%.1f S:%.1f T':%.1f (T:%.1f)",
				framei, t1.getAverage(), t2.getAverage(), t3.getAverage(), t4.getAverage(), t5.getAverage(), t6.getAverage(), t7.getAverage(), t8.getAverage(), ta.getAverage(), tt.getAverage());

			observer->showStatus(Util::netString(output), (double)framei / nframes);
			System::Windows::Forms::Application::DoEvents();							// this takes time too, and depends on amount work/updates; (~1 ms)

			framei++;
		}
		else
		{
			done = true;
		}
	}
	//imwrite("D:\\test.png", colorImage);
	doAbort();
	*/
}

void BenchMark::accumTest()
{
	long long time0 = 0;
	long long time1, time2, time3, time4, tottime;
	Averager t1, t2, t3, t4, t5, t6, t7, t8, ta, tt;
	VideoCapture video;
	Mat image, grayImage, difImage, binImage, binImage2, combImage, colorImage;
	bool done = false;
	bool videoOk;
	int framei = 0;
	cv::String output;

	reset();

	Mat backImage = imread("D:\\Video\\Ants\\Copied 17 April\\background.tif", IMREAD_GRAYSCALE);
	Mat maskImage = imread("D:\\Video\\Ants\\Copied 17 April\\mask.tif", IMREAD_GRAYSCALE);

	video.open("D:\\Video\\Ants\\Copied 17 April\\test.MTS");
	if (!video.isOpened())  // check if we succeeded
		return;

	int nframes = (int)video.get(VideoCaptureProperties::CAP_PROP_FRAME_COUNT);

	stopwatch->Restart();

	while (!done && !abort)
	{
		tottime = stopwatch->ElapsedMilliseconds - time0;
		time0 = stopwatch->ElapsedMilliseconds;

		videoOk = video.read(image);

		if (videoOk && !image.empty())
		{
			time1 = stopwatch->ElapsedMilliseconds;
			t1.addValue((double)(time1 - time0));

			ImageOperations::convertToGrayScale(image, grayImage);
			ImageOperations::difference(grayImage, backImage, difImage, true);
			ImageOperations::threshold(difImage, binImage, 0.05);
			ImageOperations::mask(binImage, maskImage, binImage2);
			time2 = stopwatch->ElapsedMilliseconds;
			t2.addValue((double)(time2 - time1));

			accumBuffer->addImage(&binImage2, AccumMode::Usage);	// Mode speed: Usage about 2.5 times as long as Age
			time3 = stopwatch->ElapsedMilliseconds;
			t3.addValue((double)(time3 - time2));

			accumBuffer->getImage(&combImage, 1, Palette::Grayscale);
			time4 = stopwatch->ElapsedMilliseconds;
			t4.addValue((double)(time4 - time3));

			//observer->displayImage(&combImage, 0);

			ta.addValue((double)(time4 - time0));
			tt.addValue((double)tottime);

			output = format("#%d V:%.1f P:%.1f CA:%.1f CG:%.1f T':%.1f (T:%.1f)",
				framei, t1.getAverage(), t2.getAverage(), t3.getAverage(), t4.getAverage(), ta.getAverage(), tt.getAverage());

			observer->showStatus(Util::netString(output), (double)framei / nframes);
			System::Windows::Forms::Application::DoEvents();							// this takes time too, and depends on amount work/updates; (~1 ms)

			framei++;
		}
		else
		{
			done = true;
		}
	}
	//imwrite("D:\\test.png", colorImage);
	doAbort();
}

void BenchMark::doAbort()
{
	abort = true;
	observer->resetUI();
}
