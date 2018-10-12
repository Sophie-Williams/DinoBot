#include "InputManager.h"

void InputManager::CalcDistanceBetweenFrames(Mat src, float* last_distance)
{
	Mat CannyOutput;

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	/// Detect edges using canny
	Canny(src, CannyOutput, 100, 200, 3);
	/// Find contours
	findContours(CannyOutput, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));
	/// Draw contours
	if(contours.size() == 2){
		// get the moments
		if(contourArea(contours[0], false) - contourArea(contours[1], false) < 10){
			vector<Moments> mu(contours.size());
			for (int i = 0; i < contours.size(); i++)
			{
				mu[i] = moments(contours[i], false);
			}

			// get the centroid of figures
			vector<Point2f> mc(contours.size());
			for (int i = 0; i < contours.size(); i++)
			{
				mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
			}

			// draw circle in centroid coordinates
			Mat drawing(CannyOutput.size(), CV_8UC3, Scalar(255, 255, 255));
			for (int i = 0; i < contours.size(); i++)
			{
				Scalar color = Scalar(167, 151, 0); // B G R values
				circle(src, mc[i], 4, color, -1, 8, 0);
			}

			if (*last_distance == 0)
				if(abs(mc[1].x - mc[0].x) > 55)				//	Mask noise values
				*last_distance = abs(mc[1].x - mc[0].x);
			
			if (abs(mc[1].x - mc[0].x) - *last_distance > 0 )	//	Mask noise values
				if(*last_distance + 20 > abs(mc[1].x - mc[0].x) - *last_distance)	//	Mask noise values
				if (abs(mc[1].x - mc[0].x) > 55)			//	Mask noise values
					*last_distance = abs(mc[1].x - mc[0].x);
		}
	}
}

void InputManager::CalcDinoHeight(Mat src, float* currDinoHeight)
{
	Mat CannyOutput;

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	vector<Rect> boundRect;

	/// Detect edges using canny
	Canny(src, CannyOutput, 100, 200, 3);
	/// Find contours
	findContours(CannyOutput, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));
	/// Draw contours
	//printf("%d, ", contours.size());
	if (contours.size() == 1) {
		// get the moments
		vector<Moments> mu(contours.size());
		for (int i = 0; i < contours.size(); i++)
		{
			mu[i] = moments(contours[i], false);
		}

		// get the centroid of figures
		vector<Point2f> mc(contours.size());
		for (int i = 0; i < contours.size(); i++)
		{
			mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
		}

		// draw circle in centroid coordinates
		Mat drawing(CannyOutput.size(), CV_8UC3, Scalar(255, 255, 255));
		for (int i = 0; i < contours.size(); i++)
		{
			Scalar color = Scalar(167, 151, 0); // B G R values
			circle(src, mc[i], 4, color, -1, 8, 0);
			*currDinoHeight = 120 - mc[i].y;
		}
	}
}

