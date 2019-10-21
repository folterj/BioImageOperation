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

#include "ScriptProcessing.h"
#include "ImageOperations.h"
//#include "BackgroundRollingBall.h"
#include "NumericPath.h"
#include "Constants.h"
#include "Util.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

using namespace System;
using namespace System::Runtime::InteropServices;


/*
 * Initialisation
 */

ScriptProcessing::ScriptProcessing(Observer^ observer)
{
	this->observer = observer;

	// initialise static lookup tables
	ColorScale::init();

	//ScriptOperation::writeOperationList("D:\\BioImageOperation.rtf");
}

/*
 * Destructor
 */

ScriptProcessing::~ScriptProcessing()
{
	if (imageList)
	{
		delete imageList;
		imageList = NULL;
	}

	if (imageSeries)
	{
		delete imageSeries;
		imageSeries = NULL;
	}

	if (accumBuffer)
	{
		delete accumBuffer;
		accumBuffer = NULL;
	}

	if (backgroundBuffer)
	{
		delete backgroundBuffer;
		backgroundBuffer = NULL;
	}

	if (averageBuffer)
	{
		delete averageBuffer;
		averageBuffer = NULL;
	}

	if (imageTrackers)
	{
		delete imageTrackers;
		imageTrackers = NULL;
	}

	if (scriptOperations)
	{
		delete scriptOperations;
		scriptOperations = NULL;
	}

	if (dummyImage)
	{
		delete dummyImage;
		dummyImage = NULL;
	}
}

/*
 * Reset class properties when (re)starting script processing
 */

void ScriptProcessing::reset()
{
	basePath = "";
	sourceWidth = 0;
	sourceHeight = 0;
	sourceFps = 0;
	logPower = 0;
	abort = false;

	scriptOperations->reset();
	imageList->reset();
	backgroundBuffer->reset();
	averageBuffer->reset();
	imageSeries->reset();
	accumBuffer->reset();

	imageTrackers->reset();
	observer->resetImages();
}

/*
 * Start processing in separate thread
 */

void ScriptProcessing::startProcess(System::String^ filePath, System::String^ script)
{
	reset();
	basePath = Util::extractFilePath(filePath);

	try
	{
		scriptOperations->extract(script, 0);
		observer->resetProgressTimer();

		processThread = gcnew Thread(gcnew ThreadStart(this, &ScriptProcessing::processThreadMethod));
		processThread->Start();
	}
	catch (System::Exception^ e)
	{
		observer->showErrorMessage(Util::getExceptionDetail(e));
		doAbort(true);
	}
}

void ScriptProcessing::processThreadMethod()
{
	processOperations(scriptOperations, NULL);
	doAbort(false);
}

/*
 * Main script loop - called recursively for { internal } loops
 */

void ScriptProcessing::processOperations(ScriptOperations* operations, ScriptOperation* prevOperation0)
{
	ScriptOperation* operation;
	ScriptOperation* prevOperation = prevOperation0;
	bool operationFinished;

	while (!abort)
	{
		operation = operations->getCurrentOperation();
		if (operation)
		{
			operation->reset();
			operationFinished = processOperation(operation, prevOperation);
			if (operationFinished)
			{
				operations->moveNextOperation();
			}
			prevOperation = operation;
		}
		else
		{
			break;
		}
	}
}

/*
 * Process single script operation
 */

bool ScriptProcessing::processOperation(ScriptOperation* operation, ScriptOperation* prevOperation)
{
	int count = operation->count;

	operation->count++;

	if (operation->interval > 1)
	{
		if ((count % operation->interval) != operation->offset)
		{
			return true;
		}
	}

	Mat* image = NULL;			// pointer to source image
	Mat* newImage = NULL;		// pointer to new image
	Mat* refImage = NULL;		// auxiliary image pointer
	bool newImageSet = false;	// if newImage is set

	if (prevOperation)
	{
		image = prevOperation->imageRef;				// source: pointer to previous operation image
		operation->imageRef = prevOperation->imageRef;	// default: set current operation image to previous operation image
	}

	if (!image)
	{
		image = dummyImage;								// prevent NULL error
	}

	newImage = &operation->image;						// newImage is pointer to current operation image

	NumericPath outputPath;
	System::String^ source;
	int width, height;
	int displayi;
	double fps;
	int time;
	bool done = true;

	try
	{
		switch (operation->operationType)
		{
		case ScriptOperationType::SetPath:
			basePath = operation->getArgument(ArgumentLabel::Path);
			break;

		case ScriptOperationType::CreateImage:
			width = (int)operation->getArgumentNumeric(ArgumentLabel::Width);
			height = (int)operation->getArgumentNumeric(ArgumentLabel::Height);
			if (width == 0 && height == 0)
			{
				width = sourceWidth;
				height = sourceHeight;
			}
			ImageOperations::create(newImage, width, height, operation->getArgument<ImageColorMode>(ArgumentLabel::ColorMode, ImageColorMode::Color), operation->getArgumentNumeric(ArgumentLabel::Red), operation->getArgumentNumeric(ArgumentLabel::Green), operation->getArgumentNumeric(ArgumentLabel::Blue));
			sourceWidth = width;
			sourceHeight = height;
			newImageSet = true;
			break;

		case ScriptOperationType::OpenImage:
			operation->initFrameSource(FrameType::Image, 0, basePath, operation->getArgument(ArgumentLabel::Path), operation->getArgument(ArgumentLabel::Start), operation->getArgument(ArgumentLabel::Length), sourceFps, (int)operation->getArgumentNumeric(ArgumentLabel::Interval));
			if (operation->frameSource->getNextImage(newImage))
			{
				observer->showStatus("", operation->frameSource->getCurrentFrame(), operation->frameSource->getTotalFrames(), true);
				done = false;
			}
			sourceWidth = operation->frameSource->getWidth();
			sourceHeight = operation->frameSource->getHeight();
			newImageSet = true;
			break;

		case ScriptOperationType::OpenVideo:
			operation->initFrameSource(FrameType::Video, (int)operation->getArgumentNumeric(ArgumentLabel::API), basePath, operation->getArgument(ArgumentLabel::Path), operation->getArgument(ArgumentLabel::Start), operation->getArgument(ArgumentLabel::Length), 0, (int)operation->getArgumentNumeric(ArgumentLabel::Interval));
			if (operation->frameSource->getNextImage(newImage))
			{
				observer->showStatus(operation->frameSource->getLabel(), operation->frameSource->getCurrentFrame(), operation->frameSource->getTotalFrames(), true);
				done = false;
			}
			else
			{
				// already past last frame; current image invalid
				return true;
			}
			sourceWidth = operation->frameSource->getWidth();
			sourceHeight = operation->frameSource->getHeight();
			sourceFps = operation->frameSource->getFps();
			newImageSet = true;
			break;

		case ScriptOperationType::OpenCapture:
			source = operation->getArgument(ArgumentLabel::Path);
			if (source != "")
			{
				source = operation->getArgumentNumeric().ToString();
			}
			operation->initFrameSource(FrameType::Capture, (int)operation->getArgumentNumeric(ArgumentLabel::API), basePath, source, "", "", 0, (int)operation->getArgumentNumeric(ArgumentLabel::Interval));

			if (operation->frameSource->getNextImage(newImage))
			{
				observer->showStatus(operation->frameSource->getCurrentFrame());
				done = false;
			}
			else
			{
				// capture failed; current image invalid
				return true;
			}
			sourceWidth = operation->frameSource->getWidth();
			sourceHeight = operation->frameSource->getHeight();
			newImageSet = true;
			break;

		case ScriptOperationType::SaveImage:
			operation->initFrameOutput(FrameType::Image, basePath, operation->getArgument(ArgumentLabel::Path), Util::netString(Constants::defaultImageExtension), operation->getArgument(ArgumentLabel::Start), operation->getArgument(ArgumentLabel::Length), sourceFps);
			operation->frameOutput->writeImage(getLabelOrCurrentImage(operation, image, true));
			break;

		case ScriptOperationType::SaveVideo:
			fps = operation->getArgumentNumeric(ArgumentLabel::Fps);
			if (fps == 0)
			{
				fps = sourceFps;
			}
			operation->initFrameOutput(FrameType::Video, basePath, operation->getArgument(ArgumentLabel::Path), Util::netString(Constants::defaultVideoExtension), operation->getArgument(ArgumentLabel::Start), operation->getArgument(ArgumentLabel::Length), fps, operation->getArgument(ArgumentLabel::Codec));
			operation->frameOutput->writeImage(getLabelOrCurrentImage(operation, image, true));
			break;

		case ScriptOperationType::ShowImage:
			refImage = getLabelOrCurrentImage(operation, image, false);
			if (Util::isValidImage(refImage))
			{
				observer->displayImage(refImage, (int)operation->getArgumentNumeric(ArgumentLabel::None, true));
			}
			break;

		case ScriptOperationType::GetImage:
			*newImage = *imageList->getImage(operation->getArgument());
			newImageSet = true;
			break;

		case ScriptOperationType::StoreImage:
			imageList->setImage(image, operation->getArgument());
			break;

		case ScriptOperationType::Scale:
			ImageOperations::scale(*getLabelOrCurrentImage(operation, image, false), *newImage,
									(int)operation->getArgumentNumeric(ArgumentLabel::Width), (int)operation->getArgumentNumeric(ArgumentLabel::Height));
			newImageSet = true;
			break;

		case ScriptOperationType::Crop:
			ImageOperations::crop(getLabelOrCurrentImage(operation, image, false), newImage,
									(int)operation->getArgumentNumeric(ArgumentLabel::X), (int)operation->getArgumentNumeric(ArgumentLabel::Y),
									(int)operation->getArgumentNumeric(ArgumentLabel::Width), (int)operation->getArgumentNumeric(ArgumentLabel::Height));
			newImageSet = true;
			break;

		case ScriptOperationType::Mask:
			ImageOperations::mask(*image, *imageList->getImage(operation->getArgument()), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::Grayscale:
			ImageOperations::convertToGrayScale(*getLabelOrCurrentImage(operation, image, false), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::Color:
			ImageOperations::convertToColor(*getLabelOrCurrentImage(operation, image, false), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::ColorAlpha:
			ImageOperations::convertToColorAlpha(*getLabelOrCurrentImage(operation, image, false), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::GetSaturation:
			ImageOperations::getSaturation(*getLabelOrCurrentImage(operation, image, false), newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::GetHsValue:
			ImageOperations::getHsValue(*getLabelOrCurrentImage(operation, image, false), newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::GetHsLightness:
			ImageOperations::getHsLightness(*getLabelOrCurrentImage(operation, image, false), newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::Threshold:
			ImageOperations::threshold(*getLabelOrCurrentImage(operation, image, false), *newImage, operation->getArgumentNumeric());
			newImageSet = true;
			break;

		case ScriptOperationType::Difference:
			ImageOperations::difference(*image, *imageList->getImage(operation->getArgument()), *newImage, false);
			newImageSet = true;
			break;

		case ScriptOperationType::DifferenceAbs:
			ImageOperations::difference(*image, *imageList->getImage(operation->getArgument()), *newImage, true);
			newImageSet = true;
			break;

		case ScriptOperationType::Add:
			ImageOperations::add(*image, *imageList->getImage(operation->getArgument()), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::Multiply:
			ImageOperations::multiply(*getLabelOrCurrentImage(operation, image, false), operation->getArgumentNumeric(), *newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::Invert:
			ImageOperations::invert(*getLabelOrCurrentImage(operation, image, false), *newImage);
			newImageSet = true;
			break;
		/*
		case ScriptOperationType::RollingBall:
			newImage = BackgroundSubtract::subtract_background_rolling_ball(getLabelOrCurrentImage(operation, image, false), 50, false);
			newImageSet = true;
			break;
		*/
		case ScriptOperationType::UpdateBackground:
			backgroundBuffer->addImage(getLabelOrCurrentImage(operation, image, true), operation->getArgumentNumeric());
			backgroundBuffer->getImage(newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::UpdateAverage:
			averageBuffer->addImage(getLabelOrCurrentImage(operation, image, true), operation->getArgumentNumeric());
			averageBuffer->getImage(newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::AddSeries:
			imageSeries->addImage(getLabelOrCurrentImage(operation, image, true), (int)operation->getArgumentNumeric());
			break;

		case ScriptOperationType::GetSeriesMedian:
			newImageSet = imageSeries->getMedian(*newImage, observer);
			break;

		case ScriptOperationType::AddAccum:
			accumBuffer->addImage(getLabelOrCurrentImage(operation, image, true), operation->getArgument<AccumMode>(ArgumentLabel::AccumMode, AccumMode::Age));
			break;

		case ScriptOperationType::GetAccum:
			logPower = operation->getArgumentNumeric();
			logPalette = operation->getArgument<Palette>(ArgumentLabel::Palette, Palette::Grayscale);
			accumBuffer->getImage(newImage, (float)logPower, logPalette);
			newImageSet = true;
			break;

		case ScriptOperationType::CreateClusters:
			imageTrackers->getTracker(operation->getArgument(ArgumentLabel::Tracker), true)->createClusters(image, operation->getArgumentNumeric(ArgumentLabel::MinArea), operation->getArgumentNumeric(ArgumentLabel::MaxArea), basePath, debugMode);
			break;

		case ScriptOperationType::CreateTracks:
			imageTrackers->getTracker(operation->getArgument(ArgumentLabel::Tracker))->createTracks(operation->getArgumentNumeric(ArgumentLabel::MaxMove), (int)operation->getArgumentNumeric(ArgumentLabel::MinActive), (int)operation->getArgumentNumeric(ArgumentLabel::MaxInactive), basePath);
			break;

		case ScriptOperationType::CreatePaths:
			imageTrackers->getTracker(operation->getArgument(ArgumentLabel::Tracker))->createPaths(operation->getArgumentNumeric(ArgumentLabel::Distance));
			break;

		case ScriptOperationType::DrawClusters:
			imageTrackers->getTracker(operation->getArgument(ArgumentLabel::Tracker))->drawClusters(getLabelOrCurrentImage(operation, image, true), newImage, operation->getClusterDrawMode(ClusterDrawMode::ClusterDefault));
			newImageSet = true;
			break;

		case ScriptOperationType::DrawTracks:
			imageTrackers->getTracker(operation->getArgument(ArgumentLabel::Tracker))->drawTracks(getLabelOrCurrentImage(operation, image, true), newImage, operation->getClusterDrawMode(ClusterDrawMode::TracksDefault), (int)sourceFps);
			newImageSet = true;
			break;

		case ScriptOperationType::DrawPaths:
			logPower = operation->getArgumentNumeric();
			logPalette = operation->getArgument<Palette>(ArgumentLabel::Palette, Palette::Grayscale);
			imageTrackers->getTracker(operation->getArgument(ArgumentLabel::Tracker))->drawPaths(getLabelOrCurrentImage(operation, image, true), newImage,
															operation->getArgument<PathDrawMode>(ArgumentLabel::PathDrawMode, PathDrawMode::Age), (float)logPower, logPalette);
			newImageSet = true;
			break;

		case ScriptOperationType::ShowTrackInfo:
			observer->showInfo(imageTrackers->getTracker(operation->getArgument(ArgumentLabel::Tracker))->getInfo(), (int)operation->getArgumentNumeric(ArgumentLabel::Display, true));
			break;

		case ScriptOperationType::DrawTrackInfo:
			imageTrackers->getTracker(operation->getArgument(ArgumentLabel::Tracker))->drawTrackInfo(getLabelOrCurrentImage(operation, image, true), newImage);
			newImageSet = true;
			break;

		case ScriptOperationType::SaveClusters:
			outputPath.setOutputPath(basePath, operation->getArgument(ArgumentLabel::Path), Util::netString(Constants::defaultDataExtension));
			imageTrackers->getTracker(operation->getArgument(ArgumentLabel::Tracker))->saveClusters(outputPath.createFilePath(count), count, operation->getArgumentBoolean(ArgumentLabel::ByLabel));
			break;

		case ScriptOperationType::SaveTracks:
			outputPath.setOutputPath(basePath, operation->getArgument(ArgumentLabel::Path), Util::netString(Constants::defaultDataExtension));
			imageTrackers->getTracker(operation->getArgument(ArgumentLabel::Tracker))->saveTracks(outputPath.createFilePath(count), count, operation->getArgumentBoolean(ArgumentLabel::ByLabel));
			break;

		case ScriptOperationType::SavePaths:
			outputPath.setOutputPath(basePath, operation->getArgument(ArgumentLabel::Path), Util::netString(Constants::defaultDataExtension));
			imageTrackers->getTracker(operation->getArgument(ArgumentLabel::Tracker))->savePaths(outputPath.createFilePath(count), count);
			break;

		case ScriptOperationType::SaveTrackInfo:
			outputPath.setOutputPath(basePath, operation->getArgument(ArgumentLabel::Path), Util::netString(Constants::defaultDataExtension));
			imageTrackers->getTracker(operation->getArgument(ArgumentLabel::Tracker))->saveTrackInfo(outputPath.createFilePath(count), count);
			break;

		case ScriptOperationType::SaveTrackLog:
			outputPath.setOutputPath(basePath, operation->getArgument(ArgumentLabel::Path), Util::netString(Constants::defaultDataExtension));
			imageTrackers->getTracker(operation->getArgument(ArgumentLabel::Tracker))->initLogClusterTrack(outputPath.createFilePath(count));
			break;

		case ScriptOperationType::DrawLegend:
			displayi = (int)operation->getArgumentNumeric(ArgumentLabel::Display);
			if (displayi > 0)
			{
				// show in separate display
				ImageOperations::create(newImage, 1000, 1000);
				ImageOperations::drawLegend(*newImage, *newImage, DrawPosition::Full, logPower, logPalette);
				observer->displayImage(newImage, displayi - 1);		// manual one-base correction
			}
			else
			{
				// draw on image
				ImageOperations::drawLegend(*getLabelOrCurrentImage(operation, image, true), *newImage, operation->getArgument<DrawPosition>(ArgumentLabel::Position, DrawPosition::BottomRight), logPower, logPalette);
				newImageSet = true;
			}
			break;

		case ScriptOperationType::Wait:
			time = (int)operation->getArgumentNumeric();
			if (time == 0)
			{
				time = 1000;
			}
			Thread::Sleep(time);
			break;

		case ScriptOperationType::Debug:
			debugMode = true;
			break;

		}	// end of switch

		if (operation->hasInnerOperations())
		{
			if (newImageSet)
			{
				operation->imageRef = newImage;
			}
			processOperations(operation->innerOperations, operation);
		}

		if (operation->asignee != "")
		{
			if (newImageSet)
			{
				imageList->setImage(newImage, Util::netString(operation->asignee));
			}
			else
			{
				imageList->setImage(image, Util::netString(operation->asignee));
			}
		}
		else
		{
			if (newImageSet)
			{
				operation->imageRef = newImage;
			}
		}
	}
	catch (System::Threading::ThreadAbortException^)
	{
		// ignore abort
	}
	catch (cv::Exception& e)
	{
		// opencv exception
		System::String^ errorMsg;
#ifdef _DEBUG
		errorMsg = Util::netString(e.msg);
#else
		if (e.err != "")
		{
			errorMsg = Util::netString(e.err);
		}
		else
		{
			errorMsg = Util::netString(e.msg);
		}
#endif
		if (errorMsg->Contains("=="))
		{
			// adding more user friendly messages:
			if (errorMsg->Contains("CV_MAT_TYPE")) {
				errorMsg += " (Image types don't match)";
			}
			if (errorMsg->Contains("CV_MAT_CN")) {
				errorMsg += " (Image color channels don't match)";
			}
		}
		errorMsg += " in\n" + Util::netString(operation->line);
		observer->showErrorMessage(errorMsg);
		doAbort(false);
	}
	catch (System::Exception^ e)
	{
		System::String^ errorMsg = Util::getExceptionDetail(e) + " in\n" + Util::netString(operation->line);
#ifdef _DEBUG
		errorMsg += e->StackTrace;
#endif
		observer->showErrorMessage(errorMsg);
		doAbort(false);
	}
	return done;
}

/*
 * Helper function to get reference image, or else current image
 */

Mat* ScriptProcessing::getLabelOrCurrentImage(ScriptOperation* operation, Mat* currentImage, bool explicitArgument)
{
	Mat* image;
	System::String^ label;

	if (explicitArgument)
	{
		label = operation->getArgument(ArgumentLabel::Label);
	}
	else
	{
		label = operation->getArgument();
	}
	image = imageList->getImage(label, false);
	if (!Util::isValidImage(image))
	{
		image = currentImage;
	}
	return image;
}

/*
 * Abort thread, attempt closing output streams to prevent data loss
 */

void ScriptProcessing::doAbort(bool tryKill)
{
	abort = true;

	if (tryKill)
	{
		if (processThread)
		{
			if (processThread->IsAlive)
			{
				Thread::Sleep(100);
				processThread->Abort();
			}
		}
	}

	imageTrackers->close();
	scriptOperations->close();

	observer->resetUI();
	observer->clearStatus();
}
