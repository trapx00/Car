#include "GPIOlib.h"
#include <cmath>
#include <stdio.h>

using namespace std;
using namespace GPIO;

#define PRINT

class PID
{
public:
    PID(double dt, double max, double min, double Kp, double Kd, double Ki): dt(dt), max(max), min(min), Kp(Kp), Kd(Kd), Ki(Ki) { }
    double calculate(double setpoint, double current) {
        // Calculate error
        double error = setpoint - current;

        // Proportional term
        double Pout = Kp * error;

        // Integral term
        integral += error * dt;
        double Iout = Ki * integral;

        // Derivative term
        double derivative = (error - pre_error) / dt;
        double Dout = Kd * derivative;

        // Calculate total output
        double output = Pout + Iout + Dout;

        // Restrict to max/min
        if( output > max )
            output = max;
        else if( output < min )
            output = min;

        // Save error to previous error
        pre_error = error;

        return output;
    }

private:
    double dt;
    double max;
    double min;
    double Kp;
    double Kd;
    double Ki;
    double pre_error = 0;
    double integral = 0;
};


// Angles
// 最大可能的目前地点和规划路径的距离。用来根据距离偏差算转向量。用赛道宽度就可以。单位和distacen保持一致即可，为cm。
const double MAX_DISTANCE_TO_PATH = 50;
const double MAX_TURN = 25;

// PID速度预期速度。单位：cm/s
const double EXPECTED_SPEED = 7;
const double MAX_SPEED = 30;

// 速度采样延时。可以设置为0。单位：毫秒
const double SPEED_SAMPLING_DELAY_MS = 0;
// 预计图片处理时间。即获得和计划路线偏移距离这个算法所需要的时间。单位：毫秒。
const double ESTIMATED_OPENCV_MS = 100;
// PID DT。调整以上两个const以精确设置Dt。
const double PID_DT = SPEED_SAMPLING_DELAY_MS + ESTIMATED_OPENCV_MS;

// PID对象。后三个参数为调参的对象，分别为P, I, D
PID pidLeftSpeed(PID_DT, MAX_SPEED, -MAX_SPEED, 1000, 0.6, 0);
PID pidRightSpeed(PID_DT, MAX_SPEED, -MAX_SPEED, 1000, 0.6, 0);
PID pidAngle(PID_DT,MAX_TURN, -MAX_TURN, 100, 0.6, 0);

void adjustSpeed(double expectedSpeed, double leftSpeed, double rightSpeed) {
    controlLeft(FORWARD, EXPECTED_SPEED);
    controlRight(FORWARD, EXPECTED_SPEED);
//     // adjust left wheel
//     double leftResult = pidLeftSpeed.calculate(expectedSpeed, leftSpeed);

// #ifdef PRINT
//     printf("Left Wheel: Current %.2lf, PID: %.2lf\n", leftSpeed, leftResult);
// #endif
//     controlLeft(leftResult>0, abs(leftResult));

//     // adjust right wheel
//     double rightResult = pidRightSpeed.calculate(expectedSpeed, rightSpeed);
// #ifdef PRINT
//     printf("Right Wheel: Current %.2lf, PID: %.2lf\n", rightSpeed, rightResult);
// #endif
//     controlRight(rightResult>0, abs(rightResult));
}

void adjustAngle(double angle) {

    if (angle == 0) {
        return;
    }

    angle *= 57.2;

    double absAngle = abs(angle);

    double turn = 90-absAngle;

    if (turn>MAX_TURN) turn = MAX_TURN;

    double result = (angle>0 ? -1:1) * turn;

    

#ifdef PRINT
    printf("Adjust angle to %.2lf\n", angle);
    #endif
    turnTo(result);
//     // adjust the angle
//     double result = pidAngle.calculate(0, angle);

// #ifdef PRINT
//     printf("Angle: Current %.2lf, PID Result: %.2lf\n", angle, result);
// #endif
//     turnTo(result);
    // calculate the angle based on result
    // proportion should be enough？
//     double angle = result / MAX_DISTANCE_TO_PATH * MAX_TURN;

// #ifdef PRINT
//     printf("Angle: Current distance %.2lf, PID Result: %.2lf. Angle result: %.2lf", distance, result, angle);
// #endif
//     turnTo(angle);
}

// 以上代码不用看，看以下的。

// 获得左右轮的速度。单位：cm/s
// 这个函数会造成程序暂停ESTIMATED_OPENCV_MS毫秒以获得上次取得速度后的速度。
// 修改那个常量以修改本方法的延迟。
void getSpeed(double& left, double& right) {
    // stop 1000ms, read the cycles left and right wheels have taken inside the 1000ms.
    delay(SPEED_SAMPLING_DELAY_MS);
    int cyclesLeft = 0, cyclesRight = 0;
    getCounter(&cyclesLeft,&cyclesRight);
    resetCounter();
    // calculate the speed
    left=cyclesLeft*63.4*M_PI/390;
    right=cyclesRight*63.4*M_PI/390;
    #ifdef PRINT
    printf("Current speed: left: %.2lf, right: %.2lf\n", left, right);
    #endif
}


// 主方法
// 这个方法会根据传入的距离调整小车的速度和角度
// 会自动获取上次执行此方法之后这一段时间内的平均速度。
// @param double angle: 目前视线中点到边线延长线交点的角度。左为负，右为正。
void adjust(double angle) {
#ifdef PRINT
    printf("Adjustment started. Angle: %.2lf\n", angle);
#endif
    double leftSpeed = 0, rightSpeed = 0;
    getSpeed(leftSpeed, rightSpeed);
    adjustSpeed(EXPECTED_SPEED, leftSpeed, rightSpeed);
    adjustAngle(angle);
}

void startWheels() {

    init();
    resetCounter();
#ifdef PRINT
    printf("Wheel started.\n");
#endif
}

void stopWheels() {
    stopLeft();
    stopRight();
    #ifdef PRINT
    printf("Wheel stopped.\n");
#endif
}
