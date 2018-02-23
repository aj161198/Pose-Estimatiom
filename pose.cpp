#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

// Checks if a matrix is a valid rotation matrix.
bool isRotationMatrix(Mat &R)
{
    Mat Rt;
    transpose(R, Rt);
    Mat shouldBeIdentity = Rt * R;
    Mat I = Mat::eye(3,3, shouldBeIdentity.type());
     
    return  norm(I, shouldBeIdentity) < 1e-6;
     
}
 
// Calculates rotation matrix to euler angles
// The result is the same as MATLAB except the order
// of the euler angles ( x and z are swapped ).
Vec3f rotationMatrixToEulerAngles(Mat &R)
{
 
    assert(isRotationMatrix(R));
     
    float sy = sqrt(R.at<double>(0,0) * R.at<double>(0,0) +  R.at<double>(1,0) * R.at<double>(1,0) );
 
    bool singular = sy < 1e-6; // If
 
    float x, y, z;
    if (!singular)
    {
        x = atan2(R.at<double>(2,1) , R.at<double>(2,2));
        y = atan2(-R.at<double>(2,0), sy);
        z = atan2(R.at<double>(1,0), R.at<double>(0,0));
    }
    else
    {
        x = atan2(-R.at<double>(1,2), R.at<double>(1,1));
        y = atan2(-R.at<double>(2,0), sy);
        z = 0;
    }
    return Vec3f(x * 180 / CV_PI, y * 180 / CV_PI, z * 180 / CV_PI);    
}

int main()
{
	VideoCapture cap(0); // open the default camera
	if(!cap.isOpened())  // check if we succeeded
        {
		cout << "Error in using VideoCapture function." << endl;	
		return -1;
	}
    	namedWindow("Thresholding_Window",CV_WINDOW_AUTOSIZE);	//Create Window to show Thresholding
	namedWindow("Original",CV_WINDOW_AUTOSIZE);		//Create Window to show Original frame
	namedWindow("Contours",CV_WINDOW_AUTOSIZE);	//Create Window to show Quadrangle contour
	
	int low_r = 0, low_b = 168, low_g = 0, high_r = 255, high_b = 255, high_g = 152;	//Initializes the track-bar values 	
	
	createTrackbar("Low_Red", "Original", &low_r, 255);			// Create Track-Bar
	createTrackbar("Low_Blue", "Original", &low_b, 255);		
	createTrackbar("Low_Green", "Original", &low_g, 255);
	createTrackbar("High_Red", "Original", &high_r, 255);
	createTrackbar("High_Blue", "Original", &high_b, 255);
	createTrackbar("High_Green", "Original", &high_g, 255);
	
	
	
	while(1)
    	{
		Mat frame;
		bool cap_status = cap.read(frame); // get a new frame from camera
		if (cap_status == 0)			//If status is false returns 1 to console
		{
			cout << "Error in capturing Frames" << endl;			
			return 1;
		}

	
		imshow("Original", frame);			// Shows the original frames		
		
			
		setTrackbarPos("Low_Red","Original", low_r);			// Set the values of Trackbar in runtime to threshold.
		setTrackbarPos("Low_Blue","Original", low_b);
		setTrackbarPos("Low_Green","Original", low_g);
		setTrackbarPos("High_Red","Original", high_r);
		setTrackbarPos("High_Blue","Original", high_b);
		setTrackbarPos("High_Green","Original", high_g);	
//<--------------------------------------------------------------------------------------------------------------------------->//
		Mat thresh;			
		inRange(frame,Scalar(low_b,low_g,low_r), Scalar(high_b,high_g,high_r),thresh);	//Threshold using inRange Function
	
		Mat element = getStructuringElement( MORPH_ELLIPSE, Size(5, 5));		//TO remove holes and other noises in thresholded-image
		dilate(thresh, thresh, element);
		erode(thresh, thresh, element);
		erode(thresh, thresh, element);	
		dilate(thresh, thresh, element);                                       
		
		
		imshow("Thresholding", thresh);		//Shows the thresholded frames
//<----------------------------------------------------------------------------------------------------------------------------------->//

		Mat thresh_c = thresh.clone(); 		//Create a clone of thresholded image-thresh
		
		vector<vector<Point> > contours;	
		vector<Vec4i> hierarchy;

		findContours(thresh_c, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);		//Find contours and store it in contours
		Mat img(frame.size(), CV_8UC3, Scalar(0, 0, 0));						//Single channel black image is created
		
		// Filtering of Contours
				
		Point2f corners[4];
		
		double area = -1;
		size_t index = -1;
		for (size_t idx = 0; idx < contours.size(); idx++) 					//Locating all the corners for every contour and drawing the rectangles above some value
        	{
			if (contourArea(contours[idx]) > 2000)							// To remove noises sets threshold value as 5000
			{
				double epsilon = arcLength(contours[idx], true) * 0.1; 				// calculate error value based on arclength			
				approxPolyDP(contours[idx], contours[idx], epsilon, true);			// approximates any contour to a polygon				
				if (contours[idx].size() == 4) 							// If polygon has 4 sides
				{
					if (area < contourArea(contours[idx]))
					{	
						area = contourArea(contours[idx]);
						index = idx;
					}
				}
			}
		}

		if (index >= 0 && area > 2000)
		{
			drawContours(frame, contours, index, Scalar(255, 0, 255), 1, 8, hierarchy);		// Draws the contours
			corners[0] = contours[index][0];						// And Stores all the corner values
			corners[1] = contours[index][1];
			corners[2] = contours[index][2];
			corners[3] = contours[index][3];
			
		}
		
		//<------------------------------------------------------------------------------------------------------------------------------------------------------------->>//		
		/* ARRANGING THE POINTS IN A DEFINITE ORDER*/

		vector<Point2f> imagePoints;
				
		char str[5];
		int index_1 = 0, _index[4];
		float max_1 = 640;
		int i;

		for (i = 0; i < 4; i++)
		{
			if (corners[i].x < max_1)
			{
				index_1 = i;
				max_1 = corners[i].x;
			}
		}
		corners[index_1].x = 640;
		
		int index_2 = 0;
		float max_2 = 640;
		for (i = 0; i < 4; i++)
		{
			if (corners[i].x < max_2)
			{
				index_2 = i;
				max_2 = corners[i].x;
			}
		}
		corners[index_1].x = max_1;
		
		if (corners[index_1].y > corners[index_2].y)
		{
			_index[0] = index_1;
			_index[3] = index_2;
		}
		else
		{
			_index[0] = index_2;
			_index[3] = index_1;
		}
	
		int p = 1;
		for (i = 0; i < 4; i++)
		{
			if (i != _index[0] && i != _index[3])
			{
				_index[p] = i;
				p++;
			}
		}
		
		if (corners[_index[1]].y < corners[_index[2]].y)
		{
			int temp = _index[1];
			_index[1] = _index[2];
			_index[2] = temp;
		}
		for (i = 0; i < 4; i++)
		{
			imagePoints.push_back(corners[_index[i]]);
		}

		int ind;
		for (ind = 0; ind < 4; ind++)
		{
			circle(img, imagePoints[ind], 8, Scalar(255, 0, 0), 2, 8, 0);				
			putText(img, format( "%d",ind + 1), imagePoints[ind], FONT_HERSHEY_PLAIN, 7, Scalar(255, 0, 255));
		}				
		imshow("Contours", img);									// Shows the contour image
		imshow("Original", frame);	
		//<<------------------------------------------------------------------------------------------------------------------------------------>>//
		/* Calculation of rvec, tvec and reprojection.*/		
		Mat cameraMatrix = (Mat_<float>(3,3) << 6.1663361610854258e+02f, 0.0f, 3.2518818995799410e+02f, 0.0f, 6.1663361610854258e+02f, 2.2801010334651178e+02f, 0.0f, 0.0f, 1.0f);
			
		// Camera Matrix from Calibration from YML file
		
		Mat distCoeffs = (Mat_<float>(5,1) << ( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f));
		// DistCoeff Matrix from Calibration from YML file
		
		vector<Point3d> worldPoints;
		worldPoints.push_back(Point3d(0.0f, 0.0f, 0.0f));
		worldPoints.push_back(Point3d(50.0f, 0.0f, 0.0f));
		worldPoints.push_back(Point3d(50.0f, 50.0f, 0.0f));
    		worldPoints.push_back(Point3d(0.0f, 50.0f, 0.0f)); 			// World Points initaialization              
    		         
			
   		Mat rvec, R; 
    		Mat tvec;
			
		solvePnP(worldPoints, imagePoints, cameraMatrix, distCoeffs, rvec, tvec, false,CV_ITERATIVE);		//Return rvec, tvec
		Mat answers;			
		Rodrigues(rvec, R);
		Vec3b angles = rotationMatrixToEulerAngles(R);	// Stores Euler angles
		
		projectPoints(worldPoints, rvec, tvec, cameraMatrix, distCoeffs, answers);		/* To check the code */
		//cout << "Image_Points" << imagePoints << endl;	
		//cout << "Answer_Points" << answers << endl;
		cout << "TVEC:- " << tvec << endl;		
		cout << "Angles:-" << angles << endl;
		//<----------------------------------------------------------------------------------------------------------------------------------->//
        	
		if (waitKey(0) == 27) 		// Waits for 300seconds and escapes the loop if Esc is pressed
		{	
			break;
		}
    	}  
  
	return 0;
}


