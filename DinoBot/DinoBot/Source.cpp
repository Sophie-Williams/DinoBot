#define UNICODE
#define _CRT_SECURE_NO_WARNINGS

#include <chrono>
#include <fstream>
#include "ScreenCaptureDC.h"
#include "FrameEditor.h"
#include "InputManager.h"

using namespace std;
using namespace cv;


//	Required for trackbar window
const int alpha_slider_max = 1;
int alpha_slider = 0;
double alpha, beta;

bool firstFrame = true;

Mat img1,img2, frame; 

//	Instance of capture class
ScreenCaptureDC capturer = ScreenCaptureDC::GetInstance(); 

void FuckingMainLoop() {

	//ofstream inputFile;
	//char* keyboard_log;
	//strcpy(keyboard_log, "KeyboardLogger2.dll");
	
	/*
	HINSTANCE hinst_1 = LoadLibrary(convertCharArrayToLPCWSTR("KeyboardLogger2.dll"));
	typedef void(*Install)();
	typedef void(*Uninstall)();

	Install install = (Install)GetProcAddress(hinst_1, "install");
	Uninstall uninstall = (Uninstall)GetProcAddress(hinst_1, "uninstall");

	install();
	*/

	//	Definition for capturing the window handler for chrome
	HWND hwndDesktop = NULL;
	hwndDesktop = FindWindowEx(0, hwndDesktop, L"Chrome_WidgetWin_1", 0);

	//	Extra variables for processing input for NNInput structure
	///	Some are used in order to maskout values created by noises
	NNInput input;
	float last_distance = 0;
	float last_speed = 200;
	float curr_disObsDino = 495;
	float curr_disObs = 0;
	float last_height = 0, dino_height = 2;
	float last_areaObs = 0;

	//	Variables for managing the frames captured by the program
	Mat f1, f2, f2_after, f5, absSub;
	int inputFrame = 1;
	bool bird = false;
	bool game_over = false;

	//	Matrices for use
	Mat gray1, gray2;
	Mat absOut, absOut1, testPause, 
		only_Dino, only_Obstacles, 
		rect_Obstacles, rect_Dino;
	Mat kernel(15, 15, CV_8U);

	//	Time measurment
	chrono::time_point<chrono::steady_clock> f1_time, f5_time;
	chrono::duration<float> elapsed;

	//inputFile.open("TrainSets.csv");
	
	while (true)
	{
		//	img1 contains the current original frame

		//printf("")
		img1 = capturer.GetFrameInMat();
		if (!firstFrame) {
			/*	// Checks the current pressed keys
			if (GetKeyState(VK_UP) & 0x8000) {
				printf("UP\n");
			} else if (GetKeyState(VK_DOWN) & 0x8000) {
				printf("DOWN\n");
			}
			else {
				printf("CALM\n");
			}*/


			absdiff(img1, img2, testPause);
			cvtColor(testPause, testPause, COLOR_BGR2GRAY);

			//	Check if the game is freezed -> Gameover or just starting
			if (countNonZero(testPause) < 1) {
				last_distance = 0;
				last_speed = 200;
				last_height = 0;
				dino_height = 2;
				curr_disObs = 0;
				curr_disObsDino = 0;
				bird = false;
				game_over = true;
				std::printf("GAME OVER\n");
			}
			else {
				game_over = false;
			}
		}
		//	clone the image so img2 will not show the rectangles over obstacles
		img2 = img1.clone();

		//	Indicates whenever to start checking for GameOver
		firstFrame = false;

		//	gray1 is the binary image of the frame with some effects
		///	returns the frame in size of 600 X 102, starting in height 28 (from top of the window)
		///	this helps to wipe out the 'grass' and the score stamp (includes dino)
		gray1 = FrameEditor::ExtractDino_Obastacles(img1, true);
		gray2 = FrameEditor::ExtractDino_Obastacles(img1, false);
		
		//	highlight the contours of obstacles in the image (lines on black)
		///	the frame in size 495 X 102, starting in position 105 (from left of the window)
		///	this helps to detect only the obstacles in each captured frame
		only_Obstacles = FrameEditor::FindContours(gray1, true);
		only_Dino = FrameEditor::FindContours(gray2, false);
		
		//	draws yellow rectangle on the black img, and pink one the the original screen
		///	We also update the distances of the obstacles in the same phase -> minimize processing time
		rect_Obstacles = FrameEditor::DrawRectangle(only_Obstacles, img1, &curr_disObs, &curr_disObsDino, &last_areaObs, &last_height, &bird);
		rect_Dino = FrameEditor::DrawRectangle(only_Dino, img1, NULL, NULL, NULL, &last_height, &bird);

		input.distanceDino = curr_disObsDino;
		input.distanceObstacles = curr_disObs;
		input.areaObstacle = last_areaObs;
		input.heightObstacle = last_height;

		std::printf("%.3f,	%.3f\n", last_areaObs, last_height);

		InputManager::CalcDinoHeight(rect_Dino, &dino_height);
		input.heightDino = dino_height;

		//printf("%.5f, ", dino_height);
		imshow("OnlyDino", rect_Dino);
		imshow("OnlyObstacles", rect_Obstacles);
		
		//	Processing frames - in order to calculate speed of the game
		if (inputFrame == 1) {
			f1 = rect_Obstacles;
			f1_time = chrono::high_resolution_clock::now();
		}
		if (inputFrame == 5) {
			f5 = rect_Obstacles;
			f5_time = chrono::high_resolution_clock::now();
			absdiff(f5, f1, absSub);
			InputManager::CalcDistanceBetweenFrames(absSub, &last_distance);

			elapsed = (f5_time - f1_time);
			if (abs((last_distance / elapsed.count()) / 100) - last_speed < 2)
				input.speed = (last_distance / elapsed.count()) / 100;
			//imshow("Centroid-SpeedCalculations", absSub);
			inputFrame = 0;
		}
		inputFrame++;
		if (!game_over) {

		}
		imshow("DinoRunOriginal", img1);
		cv::waitKey(1);
	}
	//uninstall();
}

void SelectGameRegion(int, void*) {
	alpha = (double)alpha_slider / alpha_slider_max;
	beta = 1.0 - alpha;
	if (alpha == 1)	FuckingMainLoop();
	else{
		imshow("DinoRunOriginal", capturer.GetFrameInMat());
		//imshow("OnlyObstacles", capturer.GetFrameInMat());
		//imshow("Centroid-SpeedCalculations", capturer.GetFrameInMat());
		//imshow("OnlyDino", capturer.GetFrameInMat());
	}
}


void pressUpper() {
	INPUT upper = { 0 };
	upper.type = INPUT_KEYBOARD;
	upper.ki.wVk = VK_UP;
	SendInput(1, &upper, sizeof(INPUT)); // Send KeyDown
	upper.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &upper, sizeof(INPUT)); // Send KeyUp
}

void pressDown() {
	INPUT upper = { 0 };
	upper.type = INPUT_KEYBOARD;
	upper.ki.wVk = VK_DOWN;
	SendInput(1, &upper, sizeof(INPUT)); // Send KeyDown	
}

void relasseDown() {
	INPUT upper = { 0 };
	upper.type = INPUT_KEYBOARD;
	upper.ki.wVk = VK_DOWN;
	upper.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &upper, sizeof(INPUT)); // Send KeyUp
}


int main()
{
	//ShellExecute(0, 0, "http://wayou.github.io/t-rex-runner/", 0, 0, SW_SHOW);
	
	namedWindow("DinoRunOriginal", 1);
	
	char TrackbarName[50];
	sprintf(TrackbarName, "Move to right when done focusing on game", alpha_slider_max);

	createTrackbar(TrackbarName, "DinoRunOriginal", &alpha_slider, 1, SelectGameRegion);
	SelectGameRegion(alpha_slider, 0);

	cv::waitKey(0);
}