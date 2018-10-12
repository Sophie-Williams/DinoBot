#include "FrameEditor.h"

Mat FrameEditor::ExtractDino_Obastacles(Mat src, bool dino_obs)
{
	Mat newFrame;
	Mat srcGrayScale, onlyDino_Obstacles;

	Mat kernel(19, 19, CV_8U);

	// converts the current frame to grayscale...
	cvtColor(src, srcGrayScale, COLOR_BGR2GRAY);

	// filtering noise - cleans out the clouds
		//	-	threshold_value = 200
		//	-	simply converting a picture to binary value colored - black\white
	threshold(srcGrayScale, srcGrayScale, 200, 255, THRESH_BINARY);

	//	useful in closing small holes inside the foreground object - or small black points on the object
		//morphologyEx(cloudsOut, cloudsOut, MORPH_CLOSE, kernel);
	morphologyEx(srcGrayScale, srcGrayScale, MORPH_OPEN, kernel);

	if(dino_obs == true){
		//	wipe out the grass from the buttom of the game screen
		onlyDino_Obstacles = srcGrayScale(Rect(105, 28, 495, 102));
	}
	else {
		//	wipe out the grass from the buttom of the game screen
		onlyDino_Obstacles = srcGrayScale(Rect(0, 0, 150, 130));
	}
	bitwise_not(onlyDino_Obstacles, onlyDino_Obstacles);
	//imshow("Bitwise", onlyDino_Obstacles);	//Testing parameters
	//	also used inorder to reduce noise - https://docs.opencv.org/2.4/doc/tutorials/imgproc/gausian_median_blur_bilateral_filter/gausian_median_blur_bilateral_filter.html
	medianBlur(onlyDino_Obstacles, newFrame, 19);
	//imshow("Bitwise-1", newFrame);			//Testing parameters
	return newFrame;
}

Mat FrameEditor::FindContours(Mat src, bool dino_obs)
{
	Mat CannyOutput;

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	/// Detect edges using canny
	Canny(src, CannyOutput, thresh, thresh * 2, gradient);
	/// Find contours
	if(dino_obs == true)
		findContours(CannyOutput, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(100, 0));
	else
		findContours(CannyOutput, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));
	/// Draw contours
	Mat drawing = Mat::zeros(CannyOutput.size(), CV_8UC3);
	for (int i = 0; i < contours.size(); i++)
	{
		Scalar color;
		if(dino_obs == true){
			if (arcLength(contours.at(i), true) > 45) {
				if(i%3 == 0)
					color = Scalar(0, 0, 255);
				if (i % 3 == 1)
					color = Scalar(0, 255, 255);
				if (i % 3 == 2)
					color = Scalar(0, 255, 0);
			}
			else color = Scalar(255, 255, 0);
		}
		else {
			color = Scalar(255, 255, 0);
		}
		drawContours(drawing, contours, i, color, 1, 8, hierarchy, 0, Point(0,0));
	}
	return drawing;
}

Mat FrameEditor::DrawRectangle(Mat src, Mat original, float *disObsDino, float *disObs, float *areaObs, float *lastObsHeight, bool *foundBird)
{
	Mat CannyOutput;
	//static Point P1_H, P2_H;

	//	Static bool variable to remeber if bird found 
	static bool isBird = false;

	//	Extra variables for determining the new output Mat
	Mat drawing;
	Mat black_yellow;
	vector<vector<Point>> contours;
	vector<Rect> boundRect;
	vector<Vec4i> hierarchy;
	
	//	Find contours in source image
	Canny(src, CannyOutput, thresh, thresh * 2, gradient);
	findContours(CannyOutput, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE, Point(0, 0));
	
	//	Determine the bounding rectangles over the objects
	for (int i = 0; i < contours.size(); i++) {
		if (disObs != NULL){								//	For obstacles
			if (arcLength(contours.at(i), true) > 45) {
				auto cnt = contours.at(i);
				boundRect.push_back(boundingRect(cnt));
			}
		}
		else {												//	For dino
			auto cnt = contours.at(i);
			boundRect.push_back(boundingRect(cnt));
		}
	}

	//	Sorting the contours according to X\Y axis
	if(disObs != NULL)
		sort(boundRect.begin(), boundRect.end(), rect_sorter_x());
	else
	{
		if (isBird == true) 
			sort(boundRect.begin(), boundRect.end(), rect_sorter_y_opposite());
		else
			sort(boundRect.begin(), boundRect.end(), rect_sorter_y());
	}

	//	Croping the desired area to draw on
	if (disObs != NULL)
		//	frame size in size 595 X 102, which starts at point (5,28) - from top
		///	helps to draw the rectangles in the right place on the original image
		drawing = original(Rect(105, 28, 495, 102));
	else
		drawing = original(Rect(0, 30, 100, 102));

	//	Black matrix
	black_yellow = Mat::zeros(CannyOutput.size(), CV_8UC3);

	//	Draw the rectangles
	if(disObs != NULL){														//	For obstacles
		for (int i = 0; i < boundRect.size(); i++) {
			Point p1, p2;
			p1 = Point(boundRect.at(i).x - 95, boundRect.at(i).y);
			p2 = Point(boundRect.at(i).x - 95 + boundRect.at(i).width,
				boundRect.at(i).y + boundRect.at(i).height);
			rectangle(black_yellow, p1, p2, Scalar(140, 140, 255), 1, LINE_AA, 0);	 //	Red
			rectangle(drawing, p1, p2, Scalar(0, 140, 255), 1, LINE_AA, 0);			 // Pink 
		}
	}
	else {	

		Point p1, p2;
		int last;

		if (boundRect.size() != 0) {	//	filter out rectengales whom are sure not the dino
			for (auto it = boundRect.begin(); it != boundRect.end(); ) {
				if (it->x < 20 || (it->x > 80)) {
					it = boundRect.erase(it);
				}
				else
					++it;
			}
			last = boundRect.size() - 1;
		}

		//	For dino
		if(boundRect.size()!=0){	//	Delete undesired contours from dino frame
			for (auto it = boundRect.begin(); it != boundRect.end(); ) {	
				if (it->x < 20 || (it->x > 80 && it->y < 90)) {
						it = boundRect.erase(it);
				}
				else
					++it;
			}
		}
		last = boundRect.size() - 1;

		if(last + 1 > 0){			//	if there are contours in the frame
			if ((boundRect.at(last).y < 115)) {
				if(boundRect.at(last).y > 95){	//	Dino in normal state...
					if(*lastObsHeight > 35 && boundRect.at(last).y < 110){
							printf("Bird coming, \n\n");
							isBird = true;
							*foundBird = true;
							//printf("CALM - WISHED\n");
							for (auto it = boundRect.begin(); it != boundRect.end(); ) {
								if (it->y < 95) {
									if (boundRect.size() != 1)
										it = boundRect.erase(it);
									else
										++it;
								}
								else
									++it;
							}

					}
					last = boundRect.size() - 1;
					p1 = Point(boundRect.at(last).x, boundRect.at(last).y);
					p2 = Point(boundRect.at(last).x + boundRect.at(last).width,
						boundRect.at(last).y + boundRect.at(last).height);
					rectangle(black_yellow, p1, p2, Scalar(255, 140, 140), 1, LINE_AA, 0);	 //	PURPLE
					rectangle(drawing, p1, p2, Scalar(0, 140, 255), 1, LINE_AA, 0);			 // Pink 
					
				}
				else {	// Dino in jumping state
					if (*lastObsHeight > 35 && boundRect.at(last).x > 80) {
						p1 = Point(boundRect.at(0).x, boundRect.at(0).y);
						p2 = Point(boundRect.at(0).x + boundRect.at(0).width,
							boundRect.at(0).y + boundRect.at(0).height);
					}
					else{
						//printf("JUMP\n");
						p1 = Point(boundRect.at(last).x, boundRect.at(last).y);
						p2 = Point(boundRect.at(last).x + boundRect.at(last).width,
							boundRect.at(last).y + boundRect.at(last).height);
					}
					rectangle(black_yellow, p1, p2, Scalar(140, 140, 255), 1, LINE_AA, 0);	 //	ORANGE
					rectangle(drawing, p1, p2, Scalar(0, 140, 255), 1, LINE_AA, 0);			 // Pink
				}
			}
			else {	// DUCK
				//printf("DUCK\n");
				if (isBird) {
					if (boundRect.size() != 0) {	//	Delete undesired contours from dino frame
						for (auto it = boundRect.begin(); it != boundRect.end(); ) {
							if (it->y < 120){
								if (boundRect.size() != 1)
									it = boundRect.erase(it);
								else
									++it;
							}
							else
								++it;
						}
					}
					last = boundRect.size()-1;
					p1 = Point(boundRect.at(last).x, boundRect.at(last).y);
					p2 = Point(boundRect.at(last).x + boundRect.at(last).width,
						boundRect.at(last).y + boundRect.at(last).height);
				}
				else{
					p1 = Point(boundRect.at(last).x, boundRect.at(last).y);
					p2 = Point(boundRect.at(last).x + boundRect.at(last).width,
						boundRect.at(last).y + boundRect.at(last).height);
				}
				rectangle(black_yellow, p1, p2, Scalar(0, 0, 255), 1, LINE_AA, 0);	 //	Red
				rectangle(drawing, p1, p2, Scalar(255, 255, 0), 1, LINE_AA, 0);		 // Pink 
			}
		}
	}

	//	Update data about obstacles and coming birds
	if(disObs != NULL){
		//	No obstacle in the horizon
		if (boundRect.size() == 0) {	
			///	Test for verifying the calculations
			//line(black_yellow, Point(0, 60), Point(495, 60), Scalar(140, 140, 0), 1);

			//	verified calculations
			if (isBird == true) {
				if (*lastObsHeight > 35) {		//	Avarage data for coming bird
					*areaObs = 75;		
					*lastObsHeight = 36;
				}
				else {
					*lastObsHeight = 0;
					*areaObs = 0;
					isBird = false;
					*foundBird = false;
				}
			}
			else{
				*lastObsHeight = 0;
				*areaObs = 0;
				isBird = false;
				*foundBird = false;
			}
			*disObs = 0;
			*disObsDino = 495;
		}
		//	Only 1 obstacle
		if (boundRect.size() == 1) {
			///	Test for verifying the calculations
			///Point temp = Point(boundRect.at(0).x, boundRect.at(0).y);
			///line(black_yellow, temp, Point(0, boundRect.at(0).y), Scalar(140, 140, 0), 1);
			///line(black_yellow, Point(495, boundRect.at(0).y), temp, Scalar(0, 140, 140), 1);

			//	verified calculations
			if (isBird == true) {
				if (boundRect.at(0).x < 200) {	//	untill next obstacle crosses border
					if(102 - boundRect.at(0).y < 36){
						printf("Restarted bird - 1\n");
						if (boundRect.at(0).x < 550) {
							*areaObs = boundRect.at(0).width * 2
								+ boundRect.at(0).height * 2;
						}
						else
							*areaObs = 0;
						*lastObsHeight = 102 - boundRect.at(0).y;
						isBird = false;
						*foundBird = false;
					}
					else {
						*lastObsHeight = 36;
						*areaObs = 75;
					}
				}
				else {
					*lastObsHeight = 36;
					*areaObs = 75;
				}
			}
			else {
				if (boundRect.at(0).x < 550) {
					*areaObs = boundRect.at(0).width * 2
						+ boundRect.at(0).height * 2;
				}
				else
					*areaObs = 0;
				*lastObsHeight = 102 - boundRect.at(0).y;
				isBird = false;
				*foundBird = false;
			}

			*disObs = 495 - boundRect.at(0).x;	
			*disObsDino = boundRect.at(0).x;
		}
		//	Only 2 obstacles
		if (boundRect.size() == 2) {
			///	Test for verifying the calculations
			///Point temp1 = Point(boundRect.at(0).x, boundRect.at(0).y);
			///Point temp2 = Point(boundRect.at(1).x, boundRect.at(1).y);
			///line(black_yellow, temp1, Point(0, boundRect.at(0).y), Scalar(140, 140, 0), 1);
			///line(black_yellow, Point(temp1.x, boundRect.at(1).y), temp2, Scalar(0, 140, 140), 1);

			//	verified calculations
			if (isBird == true) {
				if (boundRect.at(1).x < 200) {
					printf("Restarted bird - 2\n");
					if (boundRect.at(0).x < 550) {
						*areaObs = boundRect.at(0).width * 2
							+ boundRect.at(0).height * 2;
					}
					else
						*areaObs = 0;
					*lastObsHeight = 102 - boundRect.at(0).y;
					isBird = false;
					*foundBird = false;
				}
				else {
					*lastObsHeight = 36;	
					*areaObs = 75;
				}
			}
			else {
				if (boundRect.at(0).x < 550) {
					*areaObs = boundRect.at(0).width * 2
						+ boundRect.at(0).height * 2;
				}
				else
					*areaObs = 0;
				*lastObsHeight = 102 - boundRect.at(0).y;
				isBird = false;
				*foundBird = false;
			}

			*disObs = boundRect.at(1).x - boundRect.at(0).x;
			*disObsDino = boundRect.at(0).x;
		}
	}
	return black_yellow;
}


