#pragma once
#include "ScreenCaptureDC.h"

using namespace cv;
using namespace std;

struct rect_sorter_x {
	bool operator() (const Rect& a, const Rect& b) {
		return (a.x < b.x);
	}
};

struct rect_sorter_y {
	bool operator() (const Rect& a, const Rect& b) {
		return (a.y > b.y);
	}
};

struct rect_sorter_y_opposite {
	bool operator() (const Rect& a, const Rect& b) {
		return (a.y < b.y);
	}
};



static class FrameEditor {
public:
	static Mat ExtractDino_Obastacles(Mat src, bool dino_obs);
	static Mat FindContours(Mat src, bool dino_obs);
	static Mat DrawRectangle(Mat src, Mat original, float *disObsDino, float *disObs, float *areaObs,float *lastObsHeight, bool *foundBird);
private:
	static const int thresh = 100;
	static const int gradient = 3;
};