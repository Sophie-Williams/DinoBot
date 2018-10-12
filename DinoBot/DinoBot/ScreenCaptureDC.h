#pragma once
#include <ctime>
#include <windows.h>
#include <iostream>
#include <warning.h>
#include <vector>
#include <string> 

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class ScreenCaptureDC {
public:
	// Singelton pattern for static instance of class
	static ScreenCaptureDC& GetInstance();

	//	Acquiring the current frame from the desired area
	cv::Mat GetFrameInMat();

private:
	//	handles information about DIB (device-independent bitmap)
	//	https://docs.microsoft.com/en-us/windows/desktop/gdi/device-independent-bitmaps
	BITMAPINFOHEADER  bi;

	//	values to focus on the game area of the screen
	int width, height;

private:
	ScreenCaptureDC();
};

