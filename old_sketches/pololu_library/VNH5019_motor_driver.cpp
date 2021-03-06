#include "VNH5019_motor_driver.h"

// Constructors ////////////////////////////////////////////////////////////////

VNH5019_motor_driver::VNH5019_motor_driver()
{
  //Pin map
  _INA1 = 2;
  _INB1 = 4;
  _EN1DIAG1 = 6;
  _CS1 = A0; 
  _INA2 = 7;
  _INB2 = 8;
  _EN2DIAG2 = 12;
  _CS2 = A1;
}

VNH5019_motor_driver::VNH5019_motor_driver(unsigned char INA1, unsigned char INB1, unsigned char EN1DIAG1, unsigned char CS1, 
                                               unsigned char INA2, unsigned char INB2, unsigned char EN2DIAG2, unsigned char CS2)
{
  //Pin map
  //PWM1 and PWM2 cannot be remapped because the board is wired assuming
  // we use the library defaults, with PWM on timer1,
  // so PWM1 is pin 9, PWM2 is pin 10
  // Unfortunately, with an Uno, that means we cannot use the servo library
  // since it wipes out the analogWrite functionality of pins 9 and 10
  // That's because, it also uses timer1
  // The servo library uses timer5 on the mega (unless there are a lot of servos)
  
  _INA1 = INA1;
  _INB1 = INB1;
  _EN1DIAG1 = EN1DIAG1;
  _CS1 = CS1;
  _INA2 = INA2;
  _INB2 = INB2;
  _EN2DIAG2 = EN2DIAG2;
  _CS2 = CS2;
}

// Public Methods //////////////////////////////////////////////////////////////
void VNH5019_motor_driver::init()
{
// Define pinMode for the pins and set the frequency for timer1.

  pinMode(_INA1,OUTPUT);
  pinMode(_INB1,OUTPUT);
  pinMode(_PWM1,OUTPUT);
  pinMode(_EN1DIAG1,INPUT);
  pinMode(_CS1,INPUT);
  pinMode(_INA2,OUTPUT);
  pinMode(_INB2,OUTPUT);
  pinMode(_PWM2,OUTPUT);
  pinMode(_EN2DIAG2,INPUT);
  pinMode(_CS2,INPUT);
  
  // Timer 1 configuration
  // prescaler: clockI/O / 1
  // outputs enabled
  // phase-correct PWM
  // timer resets when it hits ICR1
  //
  // PWM frequency calculation
  // clock freq / prescaler / phase-correct / ICR1
  // 16MHz / 1 (prescaler) / 2 (phase-correct) / 510 (ICR1)  = 15.7kHz
  TCCR1A = 0b10100000;
  TCCR1B = 0b00010001;
  ICR1 = 510;   // this sets the top of the timer, which both determines the PWM frequency (see formula above)
                // and also sets the range of the analog input, as we get 100% duty cycle when we send
                // analogWrite (_PWM1, ICR1).  Since we want to send serial data as a single byte (0 to 255), we use
                // ICR1 = 510 and then just multiply the input value by 2 when we call analogWrite
}
// Set speed for motor 1, speed is a number betwenn -255 and 255
void VNH5019_motor_driver::setM1Speed(int speed)
{
  unsigned char reverse = 0;
  
  if (speed < 0)
  {
    speed = -speed;  // Make speed a positive quantity
    reverse = 1;  // Preserve the direction
  }
  if (speed > 255)  // Max PWM dutycycle
    speed = 255;
  analogWrite(_PWM1,speed * 2); // note the *2 due to using ICR1 = 510
  if (reverse)
  {
    digitalWrite(_INA1,LOW);
    digitalWrite(_INB1,HIGH);
  }
  else
  {
    digitalWrite(_INA1,HIGH);
    digitalWrite(_INB1,LOW);
  }
}

// Set speed for motor 2, speed is a number betwenn -255 and 255
void VNH5019_motor_driver::setM2Speed(int speed)
{
  unsigned char reverse = 0;
  
  if (speed < 0)
  {
    speed = -speed;  // make speed a positive quantity
    reverse = 1;  // preserve the direction
  }
  if (speed > 255) speed = 255;
  
  analogWrite(_PWM2,speed*2); // note the *2 due to using ICR1 = 510
  
  if (reverse)
  {
    digitalWrite(_INA2,LOW);
    digitalWrite(_INB2,HIGH);
  }
  else
  {
    digitalWrite(_INA2,HIGH);
    digitalWrite(_INB2,LOW);
  }
}

// Set speed for motor 1 and 2
void VNH5019_motor_driver::setSpeeds(int m1Speed, int m2Speed)
{
  setM1Speed(m1Speed);
  setM2Speed(m2Speed);
}

// Brake motor 1, brake is a number between 0 and 255
void VNH5019_motor_driver::setM1Brake(int brake)
{
  // normalize brake
  if (brake < 0)
  {
    brake = -brake;
  }
  if (brake > 255)  // Max brake
    brake = 255;
  digitalWrite(_INA1, LOW);
  digitalWrite(_INB1, LOW);
  analogWrite(_PWM1,brake * 2); // note the *2 due to using ICR1 = 510
}

// Brake motor 2, brake is a number between 0 and 255
void VNH5019_motor_driver::setM2Brake(int brake)
{
  // normalize brake
  if (brake < 0)
  {
    brake = -brake;
  }
  if (brake > 255)  // Max brake
    brake = 255;
  digitalWrite(_INA2, LOW);
  digitalWrite(_INB2, LOW);
  analogWrite(_PWM2,brake*2); // note the *2 due to using ICR1 = 510
}

// Brake motor 1 and 2, brake is a number between 0 and 255
void VNH5019_motor_driver::setBrakes(int m1Brake, int m2Brake)
{
  setM1Brake(m1Brake);
  setM2Brake(m2Brake);
}

// Return motor 1 current value in milliamps.
unsigned int VNH5019_motor_driver::getM1CurrentMilliamps()
{
  // 5V / 1024 ADC counts / 144 mV per A = 34 mA per count
  return analogRead(_CS1) * 34;
}

// Return motor 2 current value in milliamps.
unsigned int VNH5019_motor_driver::getM2CurrentMilliamps()
{
  // 5V / 1024 ADC counts / 144 mV per A = 34 mA per count
  return analogRead(_CS2) * 34;
}

// Return error status for motor 1 
unsigned char VNH5019_motor_driver::getM1Fault()
{
  return !digitalRead(_EN1DIAG1);
}

// Return error status for motor 2 
unsigned char VNH5019_motor_driver::getM2Fault()
{
  return !digitalRead(_EN2DIAG2);
}
