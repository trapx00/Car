#include <GPIOlib.h>
#include <cmath>
#include <stdio.h>

using namespace std;
using namespace GPIO;

#define PRINT

class PID
{
public:
    PID(double dt, double Kp, double Kd, double Ki): dt(dt), Kp(Kp), Kd(Kd), Ki(Ki) { }
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
        // if( output > max )
        //     output = max;
        // else if( output < min )
        //     output = min;

        // Save error to previous error
        pre_error = error;

        return output;
    }

private:
    double dt;
    // double max;
    // double min;
    double Kp;
    double Kd;
    double Ki;
    double pre_error = 0;
    double integral = 0;
};


// Angles
// 最大可能的目前地点和规划路径的距离。用来根据距离偏差算转向量。用赛道宽度就可以。单位和distacen保持一致即可，为cm。
const double MAX_DISTANCE_TO_PATH = 50;
const double MAX_TURN = 45;

// PID速度预期速度。单位：cm/s
const double EXPECTED_SPEED = 100;

// 速度采样延时。可以设置为0。单位：毫秒
const double SPEED_SAMPLING_DELAY_MS = 1000;
// 预计图片处理时间。即获得和计划路线偏移距离这个算法所需要的时间。单位：毫秒。
const double ESTIMATED_OPENCV_MS = 100;
// PID DT。调整以上两个const以精确设置Dt。
const double PID_DT = SPEED_SAMPLING_DELAY_MS + ESTIMATED_OPENCV_MS;

// PID对象。后三个参数为调参的对象，分别为P, I, D
PID pidLeftSpeed(PID_DT, 1000, 0.6, 0);
PID pidRightSpeed(PID_DT, 1000, 0.6, 0);
PID pidAngle(PID_DT, 100, 0.6, 0);

void adjustSpeed(double expectedSpeed, double leftSpeed, double rightSpeed) {
    // adjust left wheel
    double leftResult = pidLeftSpeed.calculate(expectedSpeed, leftSpeed);

#ifdef PRINT
    printf("PID Left Wheel Speed: Expected Speed %.2lf, Current Speed %.2lf, PID Result: %.2lf", expectedSpeed, leftSpeed, leftResult);
#endif
    controlLeft(leftResult>0, abs(leftResult));

    // adjust right wheel
    double rightResult = pidRightSpeed.calculate(expectedSpeed, rightSpeed);
#ifdef PRINT
    printf("PID Right Wheel Speed: Expected Speed %.2lf, Current Speed %.2lf, PID Result: %.2lf", expectedSpeed, rightSpeed, rightResult);
#endif
    controlRight(rightResult>0, abs(rightResult));
}

void adjustAngle(double distance) {
    // adjust the angle
    double result = pidAngle.calculate(0, distance);


    // calculate the angle based on result
    // proportion should be enough？
    double angle = result / MAX_DISTANCE_TO_PATH * MAX_TURN;

#ifdef PRINT
    printf("PID Angle: Current distance %.2lf, PID Result: %.2lf. Angle result: %.2lf", distance, result, angle);
#endif
    turnTo(angle);
}

// 以上代码不用看，看以下的。

// 获得左右轮的速度。单位：cm/s
// 这个函数会造成程序暂停ESTIMATED_OPENCV_MS毫秒以获得上次取得速度后的速度。
// 修改那个常量以修改本方法的延迟。
void getSpeed(double& left, double& right) {
    // stop 1000ms, read the cycles left and right wheels have taken inside the 1000ms.
    delay(SPEED_SAMPLING_MS);
    int cyclesLeft = 0, cyclesRight = 0;
    getCounter(&cyclesLeft,&cyclesRight);
    resetCounter();
    // calculate the speed
    left=cyclesLeft*63.4*M_PI/390;
    right=cyclesRight*63.4*M_PI/390;
}


// 主方法
// 这个方法会根据传入的距离调整小车的速度和角度
// 会自动获取上次执行此方法之后这一段时间内的平均速度。
// @param double distance: 目前的地点和规划的路线的距离。在左边为负，在右边为正。
void adjust(double distance) {
    double leftSpeed = 0, rightSpeed = 0;
    getSpeed(leftSpeed, rightSpeed);
    adjustSpeed(EXPECTED_SPEED, leftSpeed, rightSpeed);
    adjustAngle(distance);
}

