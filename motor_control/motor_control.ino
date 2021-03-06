#include <SparkFun_TB6612.h>

// Pins for all inputs, keep in mind the PWM defines must be on PWM pins
#define AIN1 A0
#define BIN1 A2
#define AIN2 A1
#define BIN2 A3
#define PWMA 2
#define PWMB 3
#define STBY 7

// these constants are used to allow you to make your motor configuration 


// line up with function names like forward.  Value can be 1 or -1
const int offsetA = 1;
const int offsetB = 1;

Motor motor1(AIN1, AIN2, PWMA, offsetA, STBY); //Left motor
Motor motor2(BIN1, BIN2, PWMB, offsetB, STBY); //Right motor

void motor_fwd(Motor motor_left, Motor motor_right, int speed1, int speed2)
{
  motor_left.drive(speed1);
  motor_right.drive(speed2);
}

void motor_back(Motor motor_left, Motor motor_right, int speed1, int speed2)
{
  int temp1 = abs(speed1);
  int temp2 = abs(speed2);
  motor_left.drive(-temp1);
  motor_right.drive(-temp2);
}

void motor_left(Motor motor_left, Motor motor_right, int speed1, int speed2)
{
  int temp1 = abs(speed1)/2;
  int temp2 = abs(speed2)/2;
  motor1.drive(-temp1);
  motor2.drive(temp2);
}

void setup() {
}

void loop()
{
//motor 2/right - 150
//  forward(motor1, motor2, 150);
   motor_fwd(motor1, motor2, -150, -255);
   delay(500);
//   back(motor1, motor2, 150);
//   motor_back(motor1, motor2, 150, 100);
   delay(500);
}
