//accelerometer libraries
#include <LSM9DS1_Registers.h>
#include <LSM9DS1_Types.h>
#include <SparkFunLSM9DS1.h>

//general libraries
#include <avr/interrupt.h>
#include <avr/io.h>

//motor library
#include <SparkFun_TB6612.h>

//To enable and disable the ultrasensor
#define TRIG_ON PORTB |= (1<<PINB1);
#define TRIG_OFF PORTB &= ~(1<<PINB1);

// Pins for the motor inputs
#define AIN1 A0
#define BIN1 A2
#define AIN2 A1
#define BIN2 A3
#define PWMA 2
#define PWMB 3
#define STBY 7 //22 in mega

#define SAMPLE_SPEED 250 //250ms between samples

// line up with function names like forward.  Value can be 1 or -1
const int offsetA = 1;
const int offsetB = 1;

const int STOP = 0, FORWARD = 1, SPEED_UP = 2, SLOW_DOWN = 3, LEFT = 4; //fsm states

int state = STOP; //Current state
int count = 0; //Counter for RIGHT state
int echoPin = 8;

float spd_m; //measured speed
float spd_reg = 1.0; //regular speed
float spd_d = 1.0; //desired speed
int gSpeed = 50;

long dist_m = 0; //measured distance
long dist_t = 15; //distance threshold

int counter = 0;//global counter to keep track of how long it takes for it to run in specific

Motor motor1(AIN1, AIN2, PWMA, offsetA, STBY);
Motor motor2(BIN1, BIN2, PWMB, offsetB, STBY);

LSM9DS1 imu; //9dof sensor

//Timing
unsigned long start_time;
unsigned long end_time;
unsigned long total;

long getDistance()
{
  long distance = 0;
  long duration = 0;

  TRIG_ON
  delayMicroseconds(10);
  TRIG_OFF

  duration = pulseIn(echoPin, HIGH, 150000L);
  distance = duration/58; //Convert duration to cm
  return distance; 
}

void motor_fwd(Motor motor1, Motor motor2, int speed1, int speed2)
{
  motor1.drive(speed1);
  motor2.drive(speed2);
}

void motor_back(Motor motor1, Motor motor2, int speed1, int speed2)
{
  int temp1 = abs(speed1);
  int temp2 = abs(speed2);
  motor1.drive(-temp1);
  motor2.drive(-temp2);
}

void motor_left(Motor motor1, Motor motor2, int speed1, int speed2)
{
  int temp1 = abs(speed1)/2;
  int temp2 = abs(speed2)/2;
  motor1.drive(-temp1);
  motor2.drive(temp2);
}

float fsm(long distance)
{
  float spd = 0.0;
  
  switch(state)
  {
    case STOP:
      brake(motor1, motor2);
      if(distance > dist_t)
      {
        spd = spd_reg;
        state = SPEED_UP;
      }
      else if(distance <= dist_t)
      {
        state = LEFT;
      }
      Serial.println("STOP");
      delay(500);
      break; 
      
    case FORWARD:
      if(distance <= dist_t)
      {
        spd = 0;
        state = SLOW_DOWN;
      }
      else
      {
        motor_fwd(motor1, motor2, gSpeed, gSpeed);
      }
      Serial.println("FORWARD");
      delay(50);
      break;

    case SPEED_UP:
      if(gSpeed<100){
        gSpeed += 2;
        motor_fwd(motor1, motor2, gSpeed, gSpeed);
      }
      else
      {
        state = FORWARD;
        motor_fwd(motor1, motor2, gSpeed, gSpeed);
      }
      Serial.println("SPEED UP");   
      delay(50);
      break;

    case SLOW_DOWN:
      if(gSpeed>50){
        gSpeed -= 2;
        motor_fwd(motor1, motor2, gSpeed, gSpeed);
      }
      else
      {
        state = STOP;
        motor_fwd(motor1, motor2, gSpeed, gSpeed);
      }
      Serial.println("SLOW DOWN");
      delay(50);
      break;
      
    case LEFT:
      if(((distance > dist_t) && count == 0) || (count == 2))
      {
        spd = 0;
        count = 0;
        state = STOP;
      }
      else
      {
        motor_back(motor1, motor2, gSpeed, gSpeed);
        count += 1;
      }

      Serial.println("LEFT");
      delay(50);
      break;

      default:
        spd = 0;
        state = STOP;
        brake(motor1, motor2);
        break;
  }
  return spd;
}

void setup() {
  DDRB &= ~(1<<DDB0); //Set pin B0 as input for echo 
  DDRB |= (1<<DDB1); //Set pin B1 as output for trigger
  TRIG_OFF //Set to low to initialize the sensor
  Wire.begin();
  imu.begin(); //Use pins A4 & A5 by default
  Serial.begin(9600);
}

void loop() {
  dist_m = getDistance();
  Serial.print("Distance: "); Serial.print(dist_m);
  Serial.print("  State: ");
  spd_d = fsm(dist_m);
  counter++;
  Serial.println(counter);
  if (counter == 100000)
  {
    counter=0; 
  }
}
