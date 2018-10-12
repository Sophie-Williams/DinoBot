#include "ScreenCaptureDC.h"

ScreenCaptureDC & ScreenCaptureDC::GetInstance()
{
	static ScreenCaptureDC *m_CaptureManager;
	if (!m_CaptureManager)
		m_CaptureManager = new ScreenCaptureDC();
	return *m_CaptureManager;
}

cv::Mat ScreenCaptureDC::GetFrameInMat()
{
	HDC hwindowDC;
	HDC hwindowCompatibleDC;	// Memory for given DC 
	HBITMAP hbwindow;

	cv::Mat frame;

	//	retrieces a handle to DC for the client area for entire screen
	hwindowDC = GetDC(NULL);
	//	creates memory DC for the entire screen
	hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	//	enables strecn mode which has low costs - because of the grayscale game
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

	//	creates matrix with four color channels and values in the range 0 to 255
	//	-	https://stackoverflow.com/questions/39543580/what-is-the-purpose-of-cvtype-cv-8uc4
	frame.create(height, width, CV_8UC4);

	// create a bitmap for the window
	hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
	// attach the bitmap with the memory DC for our window
	SelectObject(hwindowCompatibleDC, hbwindow);

	// copy from the window device context to the bitmap device context
	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 650, 130, 620, 150, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !

	// Gets the "bits" from the bitmap and copies them into src.data
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, frame.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

	// avoid memory leak																							   // avoid memory leak
	DeleteObject(hbwindow);
	DeleteDC(hwindowCompatibleDC);	//	for any DC that was created by calling the CreateDC function

	return frame;
}

ScreenCaptureDC::ScreenCaptureDC()
{
	width = 600;
	height = 150;
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = 600;
	bi.biHeight = -150;  //the minus sign makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;
}

