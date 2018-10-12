#pragma once
#include "ScreenCaptureDC.h"

using namespace cv;
using namespace std;

struct NNInput {
	float speed;			//	Speed of the game
	float distanceObstacles;//	Distance between obstacles
	float distanceDino;		//	Distance to next obstacle
	float heightDino;		//	Height of Dino
	float heightObstacle;	//	Height of obstacle
	float areaObstacle;		//	Area of obstacle
};

static class InputManager {
public:
	static void CalcDistanceBetweenFrames(Mat src, float* last_distance); 
	static void CalcDinoHeight(Mat src, float* currDinoHeight);
};