#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <cmath>

#define PI 3.1415926

//Uncomment this line at run-time to skip GUI rendering
//#define _DEBUG

using namespace cv;
using namespace std;

//int erosion_elem = 0;
//int erosion_size = 0;
//int dilation_elem = 0;
//int dilation_size = 0;
//int const max_elem = 5;
//int const max_kernel_size = 21;
//
//void Erosion(int, void *);//腐蚀操作
//void Dilation(int, void *);//膨胀操作

void drawLine(Mat &picture, Point startPoint, Point endPoint);

double calculateDistance(Point point1, Point point2);

double calcAngle(double x1, double y1, double x2, double y2);

void resizeImage(Mat frame);

const string MAIN_WINDOW_NAME = "Processed Image";
const string CANNY_WINDOW_NAME = "Canny";
const string CAM_PATH = "/dev/video0";

const int CANNY_LOWER_BOUND = 50;
const int CANNY_UPPER_BOUND = 250;
const int HOUGH_THRESHOLD = 150;
const int CV_CAP_DROP_FPS = 24;

struct Line {
public:
    double length;
    double k;
    double b;
    
    Line(double length, double k, double b) : length(length), k(k), b(b) {}
};

// returns angle
void analysePicture(Mat imag, double& angle) {
    //裁剪图像以加快处理
    resizeImage(imag);
    //红色的HSV阈值
    int iLowH = 0;
    int iHighH = 10;

    int iLowS = 43;//43
    int iHighS = 255;

    int iLowV = 46;//46
    int iHighV = 255;

    //BGR转HSV并识别红色
    Mat imgHSV;
    vector<Mat> hsvSplit;
    cvtColor(imag, imgHSV, COLOR_BGR2HSV);
    split(imgHSV, hsvSplit);
    equalizeHist(hsvSplit[2], hsvSplit[2]);
    merge(hsvSplit, imgHSV);
    Mat imgThresholded;

    inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded);
    vector<vector<Point>> contours;
    findContours(imgThresholded, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE); //找轮廓
    Mat element = getStructuringElement(MORPH_RECT, Size(5, 5));
    morphologyEx(imgThresholded, imgThresholded, MORPH_OPEN, element);

    morphologyEx(imgThresholded, imgThresholded, MORPH_CLOSE, element);

    vector<vector<Point>> contours1;
    Point squareCenter = Point(0, 0);
    double squareX = 0;
    double squareY = 0;
    double numOfPoint = 0;
    for (int i = 0; i < contours.size(); ++i)
    {
        cout << contours[i].size() << endl;
        if (contours[i].size() >= 4)
        {
            for (int j = 0; j < contours[i].size(); ++j)
            {
                squareX += contours[i][j].x;
                squareY += contours[i][j].y;
                numOfPoint++;
            }
            contours1.push_back(contours[i]);
        }
    }
 
    //红色大小阈值
    if(numOfPoint<=1000){
    	contours1.clear();
    }
    else{squareCenter.x = squareX / numOfPoint;
        squareCenter.y = squareY / numOfPoint;
    }

	if(contours1.size()>0){
		if (squareCenter.x < (imag.cols / 2)){
			angle=2;
		}else{
			angle=-2;
		}
        #ifdef _DEBUG
        drawContours(imag, contours1, -1, Scalar(255, 0, 0), CV_FILLED);
        imshow("Main Window", imag);
        waitKey(1);
        #endif
		return;
	}

    //开始识别轮廓，即车道
    imag = imread("image.jpg", 0);
    Mat result = imag.clone();

    Canny(result, result, 100, 150, 3);
    std::vector<Vec4i> lines;
    std::vector<Line> leftLineTuples;
    std::vector<Line> rightLineTuples;
    
    cv::HoughLinesP(result, lines, 1, CV_PI / 180, 70, 30, 10);
    
    for (int i = 0; i < lines.size(); i++) {
        double delta_x = lines[i][2] - lines[i][0];
        double delta_y = lines[i][3] - lines[i][1];
        double length = sqrt(delta_x * delta_x + delta_y * delta_y);
        double k = delta_y / delta_x;
        double b = lines[i][1] - k * lines[i][0];
        if (k > 0) {
            cout << lines[i] << endl;
            cout << length << endl;
            rightLineTuples.push_back(Line(length, k, b));
        } else if (k < 0) {
            leftLineTuples.push_back(Line(length, k, b));
        }
    }
    
    //只看见左边或右边的线则判断为过偏
    if (rightLineTuples.size() <= 0 && leftLineTuples.size() <= 0)
    {
#ifdef _DEBUG
        imshow("Main Window", imag);
        waitKey(1);
#endif
        return;
    }
    else if (rightLineTuples.size() >= 0 && leftLineTuples.size() <= 0)
    {
#ifdef _DEBUG
        imshow("Main Window", imag);
        waitKey(1);
#endif
	angle = 1;
	return;
    }
    else if (rightLineTuples.size() <= 0 && leftLineTuples.size() >= 0)
    {
#ifdef _DEBUG
        imshow("Main Window", imag);
        waitKey(1);
#endif
	angle = -1;
        return;
    }
    
    int maxRightLengthIndex = 0;
    double maxLength = 0;
    for (int i = 0; i < rightLineTuples.size(); i++) {
        if (maxLength < rightLineTuples[i].length) {
            maxLength = rightLineTuples[i].length;
            maxRightLengthIndex = i;
        }
    }
    
    int maxLeftLengthIndex = 0;
    maxLength = 0;
    for (int i = 0; i < leftLineTuples.size(); i++) {
        if (maxLength < leftLineTuples[i].length) {
            maxLength = leftLineTuples[i].length;
            maxLeftLengthIndex = i;
        }
    }
    
    Line rightMaxLengthLine = rightLineTuples[maxRightLengthIndex];
    Line leftMaxLengthLine = leftLineTuples[maxLeftLengthIndex];
    double joinX = (leftMaxLengthLine.b - rightMaxLengthLine.b) / (rightMaxLengthLine.k - leftMaxLengthLine.k);
    double joinY = leftMaxLengthLine.k * joinX + leftMaxLengthLine.b;
    
    double deltaAngle = calcAngle(joinX, joinY, result.cols / 2, result.rows);
    
    // returns values
    angle = deltaAngle;
    
    #ifdef _DEBUG
    //画线
     double rightLineX = -rightMaxLengthLine.b / rightMaxLengthLine.k;
     double rightLineY = rightMaxLengthLine.k * imag.cols + rightMaxLengthLine.b;
     Point rightEndPoint = Point(static_cast<int>(rightLineX), 0);
     Point rightStartPoint = Point(imag.cols, static_cast<int>(rightLineY));
     
     double leftLineX = -leftMaxLengthLine.b / leftMaxLengthLine.k;
     double leftLineY = leftMaxLengthLine.b;
     Point leftEndPoint = Point(static_cast<int>(leftLineX), 0);
     Point leftStartPoint = Point(0, static_cast<int>(leftLineY));
     
     drawLine(result, leftStartPoint, leftEndPoint);
     drawLine(result, rightStartPoint, rightEndPoint);
    
    imshow("Main Window", result);
    waitKey(1);
    #endif
    
    //    vector<Point2f> corners(4);
    //    corners[0] = leftStartPoint;
    //    corners[1] = rightStartPoint;
    //    corners[2] = leftEndPoint;
    //    corners[3] = rightEndPoint;
    //    vector<Point2f> corners_trans(4);
    //    corners_trans[0] = leftStartPoint;
    //    corners_trans[1] = rightStartPoint;
    //    corners_trans[2] = Point2f(leftStartPoint.x, 10);
    //    corners_trans[3] = Point2f(rightStartPoint.x, 10);
    //    Mat transform = getPerspectiveTransform(corners, corners_trans);
    
    //    warpPerspective(imag, result, transform, cv::Size(imag.cols + 100, imag.rows), cv::INTER_LINEAR);
    //    imwrite("result.jpg", imag);
}

double calcAngle(double x1, double y1, double x2, double y2) {
    double angle_temp;
    double xx, yy;
    xx = x2 - x1;
    yy = y2 - y1;
    if (xx == 0.0)
        angle_temp = PI / 2.0;
    else
        angle_temp = atan(yy / xx);
    
    return angle_temp;
}

void resizeImage(Mat frame) {
    Rect rect(0, frame.rows / 2, frame.cols, frame.rows / 2 - 72);
    Mat image_roi = frame(rect);
    Mat mask = image_roi.clone();
    // 将帧转成图片输出
    imwrite("image.jpg", image_roi);
}

void getOneShot() {
    // 获取视频文件
    VideoCapture cap("video.avi");
    // 获取视频总帧数
    Mat frame;
    cap.read(frame);
    Rect rect(0, frame.rows / 2, frame.cols, frame.rows / 2 - 72);
    Mat image_roi = frame(rect);
    Mat mask = image_roi.clone();
    // 将帧转成图片输出
    imwrite("image.jpg", image_roi);
}

void record() {
    VideoCapture capture(CAM_PATH);
    VideoWriter write;
    //If this fails, try to open as a video camera, through the use of an integer param
    if (!capture.isOpened()) {
        capture.open(atoi(CAM_PATH.c_str()));
    }
    
    int dWidth = static_cast<int>(capture.get(CAP_PROP_FRAME_WIDTH)); //the width of frames of the video
    int dHeight = static_cast<int>(capture.get(CAP_PROP_FRAME_HEIGHT));
    Size S(dWidth, dHeight);
    //the height of frames of the video
    clog << "Frame Size: " << dWidth << "x" << dHeight << endl;
    capture.get(CV_CAP_DROP_FPS);
    string outFile = "./video.avi";
    write.open(outFile, VideoWriter::fourcc('M', 'J', 'P', 'G'), CV_CAP_DROP_FPS, S, true);
    
    bool stop = false;
    Mat frame;
    while (!stop) {
        if (!capture.read(frame))
            break;
        imshow("Video", frame);
        write.write(frame);
        if (waitKey(10) > 0) {
            stop = true;
        }
    }
    capture.release();
    write.release();
    cv::destroyWindow("Video");
}

void drawLine(Mat &picture, Point startPoint, Point endPoint) {
    line(picture, startPoint, endPoint, Scalar(255, 0, 0), 10);
}
