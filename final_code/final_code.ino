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

const int STOP = 0, FORWARD = 1, LEFT = 2; //fsm states

int state = STOP; //Current state
int count = 0; //Counter for RIGHT state
int echoPin = 8;

float spd_m; //measured speed
float spd_reg = 10.0; //regular speed
float spd_d = 10.0; //desired speed

//PID variables
float fwd_accel = 0; //in m/s^2
float velocity = 0; //in m/s
float deltaT = (SAMPLE_SPEED/1000.0);

long dist_m = 0; //measured distance
long dist_t = 10; //distance threshold

Motor motor1(AIN1, AIN2, PWMA, offsetA, STBY);
Motor motor2(BIN1, BIN2, PWMB, offsetB, STBY);

LSM9DS1 imu; //9dof sensor

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

float getSpeed()
{
  if(imu.accelAvailable())
  {
    imu.readAccel();
  }

  fwd_accel = (imu.calcAccel(imu.ay)*9.8);

  velocity += (fwd_accel*deltaT);

  if(velocity < 0)
    velocity = 0;

  return velocity;
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
        state = FORWARD;
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
        state = STOP;
      }
      else
      {
        forward(motor1, motor2, 150);
      }
      Serial.println("FORWARD");
      delay(500);
      break;
      
    case LEFT:
      if((distance > dist_t && count == 0) || (count >= 2))
      {
        spd = 0;
        state = STOP;
      }
      else
      {
        left(motor1, motor2, 100);
        count += 1;
      }

      Serial.println("LEFT");
      delay(500);
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
  spd_d = fsm(dist_m);
  spd_m = getSpeed();
}
