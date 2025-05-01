#include "CMove.h"
#include <pigpio.h>
#include <iostream>
#include <thread>
#include <chrono>

CMove::CMove()
{
    if (gpioInitialise() < 0)
    {
        std::cerr << "pigpio initialization failed" << std::endl;
        return;
    }

    // Set up DC motor pins
    gpioSetMode(LEFT_MOTOR_PWM, PI_OUTPUT);
    gpioSetMode(LEFT_MOTOR_IN1, PI_OUTPUT);
    gpioSetMode(LEFT_MOTOR_IN2, PI_OUTPUT);
    gpioSetPWMfrequency(LEFT_MOTOR_PWM, DC_PWM_FREQ);
    gpioSetPWMrange(LEFT_MOTOR_PWM, PWM_RANGE);

    gpioSetMode(RIGHT_MOTOR_PWM, PI_OUTPUT);
    gpioSetMode(RIGHT_MOTOR_IN1, PI_OUTPUT);
    gpioSetMode(RIGHT_MOTOR_IN2, PI_OUTPUT);
    gpioSetPWMfrequency(RIGHT_MOTOR_PWM, DC_PWM_FREQ);
    gpioSetPWMrange(RIGHT_MOTOR_PWM, PWM_RANGE);

    // Set up servo pin
    gpioSetMode(BOX_SERVO_PIN, PI_OUTPUT);
    gpioSetPWMfrequency(BOX_SERVO_PIN, SERVO_PWM_FREQ);
    gpioSetPWMrange(BOX_SERVO_PIN, SERVO_RANGE);

    initialized = true;

    // Initialize motors to stopped state
    Stop();
    BoxStop();

    std::cout << "Motor controller initialized" << std::endl;
}

CMove::~CMove()
{
    Stop(); // Stop DC motors
    BoxStop(); // Center servo
    gpioTerminate();
    std::cout << "Motor controller shut down" << std::endl;
}

void CMove::Forward(int speed)
{
    if (!initialized) return;

    // Limit speed to valid range
    speed = std::max(0, std::min(speed, PWM_RANGE));

    // Left motor forward
    gpioWrite(LEFT_MOTOR_IN1, 1);
    gpioWrite(LEFT_MOTOR_IN2, 0);
    gpioPWM(LEFT_MOTOR_PWM, speed);

    // Right motor forward
    gpioWrite(RIGHT_MOTOR_IN1, 1);
    gpioWrite(RIGHT_MOTOR_IN2, 0);
    gpioPWM(RIGHT_MOTOR_PWM, speed);
}

void CMove::Backward(int speed)
{
    if (!initialized) return;

    speed = std::max(0, std::min(speed, PWM_RANGE));

    // Left motor backward
    gpioWrite(LEFT_MOTOR_IN1, 0);
    gpioWrite(LEFT_MOTOR_IN2, 1);
    gpioPWM(LEFT_MOTOR_PWM, speed);

    // Right motor backward
    gpioWrite(RIGHT_MOTOR_IN1, 0);
    gpioWrite(RIGHT_MOTOR_IN2, 1);
    gpioPWM(RIGHT_MOTOR_PWM, speed);
}

void CMove::TurnRight(int speed)
{
    if (!initialized) return;

    speed = std::max(0, std::min(speed, PWM_RANGE));

    // Left motor forward
    gpioWrite(LEFT_MOTOR_IN1, 1);
    gpioWrite(LEFT_MOTOR_IN2, 0);
    gpioPWM(LEFT_MOTOR_PWM, speed);

    // Right motor backward (for pivot turn)
    gpioWrite(RIGHT_MOTOR_IN1, 0);
    gpioWrite(RIGHT_MOTOR_IN2, 1);
    gpioPWM(RIGHT_MOTOR_PWM, speed);
}

void CMove::TurnLeft(int speed)
{
    if (!initialized) return;

    speed = std::max(0, std::min(speed, PWM_RANGE));

    // Left motor backward (for pivot turn)
    gpioWrite(LEFT_MOTOR_IN1, 0);
    gpioWrite(LEFT_MOTOR_IN2, 1);
    gpioPWM(LEFT_MOTOR_PWM, speed);

    // Right motor forward
    gpioWrite(RIGHT_MOTOR_IN1, 1);
    gpioWrite(RIGHT_MOTOR_IN2, 0);
    gpioPWM(RIGHT_MOTOR_PWM, speed);
}

void CMove::Stop()
{
    if (!initialized) return;

    // Brake both motors
    gpioWrite(LEFT_MOTOR_IN1, 1);
    gpioWrite(LEFT_MOTOR_IN2, 1);
    gpioPWM(LEFT_MOTOR_PWM, 0);

    gpioWrite(RIGHT_MOTOR_IN1, 1);
    gpioWrite(RIGHT_MOTOR_IN2, 1);
    gpioPWM(RIGHT_MOTOR_PWM, 0);
}

void CMove::BoxUp()
{
    if (!initialized) return;

    // Set servo to up position (1500us pulse width)
    // pulse width in microseconds = (position * 20000) / SERVO_RANGE
    int position = (BOX_UP_POS * SERVO_RANGE) / 20000;
    gpioPWM(BOX_SERVO_PIN, position);
}

void CMove::BoxDown()
{
    if (!initialized) return;

    // Set servo to down position (1000us pulse width)
    int position = (BOX_DOWN_POS * SERVO_RANGE) / 20000;
    gpioPWM(BOX_SERVO_PIN, position);
}

void CMove::BoxStop()
{
    if (!initialized) return;

    // Center the servo (neutral position)
    int position = (1500 * SERVO_RANGE) / 20000;
    gpioPWM(BOX_SERVO_PIN, position);
}
