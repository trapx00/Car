#include <cstdlib>
#include <iostream>
#include <vector>
#include "view.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#define PI 3.1415926

//Uncomment this line at run-time to skip GUI rendering
#define _DEBUG

using namespace cv;
using namespace std;

const string CAM_PATH = "/dev/video0";
const string MAIN_WINDOW_NAME = "Processed Image";
const string CANNY_WINDOW_NAME = "Canny";

const int CANNY_LOWER_BOUND = 50;
const int CANNY_UPPER_BOUND = 250;
const int HOUGH_THRESHOLD = 150;

int main() {
    VideoCapture capture(CAM_PATH);
    //If this fails, try to open as a video camera, through the use of an integer param
    if (!capture.isOpened()) {
        capture.open(atoi(CAM_PATH.c_str()));
    }

    double dWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH);            //the width of frames of the video
    double dHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT);        //the height of frames of the video
    clog << "Frame Size: " << dWidth << "x" << dHeight << endl;

    Mat image;
    while (true) {
        capture >> image;
        if (image.empty())
            break;
        analysePicture(image);
    }
    return 0;
}