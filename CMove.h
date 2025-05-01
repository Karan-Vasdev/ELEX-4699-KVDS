#pragma once


class CMove
{
private:
    // DC Motor pins (H-bridge control)
    const int LEFT_MOTOR_PWM = 12; // GPIO for left motor speed
    const int LEFT_MOTOR_IN1 = 5; // GPIO for left motor direction 1
    const int LEFT_MOTOR_IN2 = 6; // GPIO for left motor direction 2

    const int RIGHT_MOTOR_PWM = 13; // GPIO for right motor speed
    const int RIGHT_MOTOR_IN1 = 19; // GPIO for right motor direction 1
    const int RIGHT_MOTOR_IN2 = 26; // GPIO for right motor direction 2

    // Servo motor pin for box mechanism
    const int BOX_SERVO_PIN = 16; // GPIO for servo control

    // PWM settings
    const int DC_PWM_FREQ = 10000; // 10 kHz for DC motors
    const int SERVO_PWM_FREQ = 50; // 50 Hz for servo
    const int PWM_RANGE = 255; // 8-bit resolution for DC motors
    const int SERVO_RANGE = 20000; // Standard for servo control (20ms period)

    // Servo position limits (adjust for your servo)
    const int BOX_UP_POS = 1500; // 1500us pulse (neutral position)
    const int BOX_DOWN_POS = 1000; // 1000us pulse

    bool initialized = false;

public:
    CMove(); // Constructor
    ~CMove(); // Destructor

    void Forward(int speed = 255); // Default full speed
    void Backward(int speed = 255);
    void TurnRight(int speed = 200); // Default turning speed
    void TurnLeft(int speed = 200);
    void Stop();

    void BoxUp();
    void BoxDown();
    void BoxStop();
};
