#ifndef triMotorDriver_h
#define triMotorDriver_h

#include <Arduino.h>

// analog inputs
#define BATTERY_MONITOR A5
#define FBA A6
#define FBB A7
#define FBC A8
    
class triMotorDriver
{
  public:  
    // CONSTRUCTOR
    triMotorDriver(); // pin selection an initial config
    
    // PUBLIC METHODS
    void setSpeedA(int speed); // Set speed for left motor
    void setSpeedB(int speed); // Set speed for right motor
    void setSpeedC(int speed); // Set speed for top motor
    void setSpeedAB(int speedA, int speedB); // Set speed for left and right motors
    void setBrakesAB();
    void setBrakesC();
    void setCoastA();
    void setCoastB();
    void setCoastC();
    void setCoastAB();
    int getCurrentA();
    int getCurrentB();
    int getCurrentC();
    unsigned char getStatusA(); // Get status of left motor
    unsigned char getStatusB(); // Get status of right motor
    unsigned char getStatusC(); // Get status of top motor
    int getBatteryMonitor();
    
  private:
    unsigned char IN1A;
    unsigned char IN2A;
    unsigned char STATUSA;
    unsigned char ENABLEAB;
    unsigned char PWMA;  // PWM: 0 to 13. Provide 8-bit PWM output with the analogWrite() function.
    
    unsigned char IN1B;
    unsigned char IN2B;
    unsigned char STATUSB;
    unsigned char PWMB;
    
    unsigned char IN1C;
    unsigned char IN2C;
    unsigned char STATUSC;
    unsigned char ENABLEC;
    unsigned char PWMC;
    // interrupt pins on the mega are:
    // 2 (interrupt 0), 3 (interrupt 1), 18 (interrupt 5), 19 (4), 20 (3), and 21 (2)
    // we use them for motor encoders
    unsigned char ENCA; 
    unsigned char ENCB;
    unsigned char ENCC;
    
};

#endif
