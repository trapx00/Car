#include <cstdlib>
#include <iostream>
#include <vector>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include "view.cpp"
#include "control.cpp"

#define PI 3.1415926

//Uncomment this line at run-time to skip GUI rendering
#define _DEBUG

using namespace cv;
using namespace std;

int main() {
    VideoCapture capture(0);
    //If this fails, try to open as a video camera, through the use of an integer param
    if (!capture.isOpened()) {
        capture.open(0);
    }
    
    double dWidth = capture.get(CAP_PROP_FRAME_WIDTH);            //the width of frames of the video
    double dHeight = capture.get(CAP_PROP_FRAME_HEIGHT);        //the height of frames of the video
    

    startWheels();

    Mat image;
    while (true) {
        capture >> image;
        if (image.empty())
            break;

        #ifdef _DEBUG
        imshow("Main Window", image);
        waitKey(1);
        #endif

        double angle = 0;
        analysePicture(image), angle);
        adjust(angle);
    }

    stopWheels();
    
    return 0;
}
