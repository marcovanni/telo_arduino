#include "threeMotorsDriver.h"

// Motors are A: left motor, B: right motor, C: camera motor
// Serial0 (pins 0 (RX) and 1 (TX)) is used for programming and debugging
// Serial2 (pins 17 (RX) and 16 (TX)) is used for bluetooth comm, note that impacts the bluetooth library files
// wiring of the 33926 driver:
// if we do not care to get coasting vs braking, then we can tie IN1 HIGH and IN2 LOW and use INV to select direction
// if we want coasting in addition to braking, then set IN1 and IN2 both LOW (or HIGH) and PWM to 255 to coast
// to brake, set ENABLE LOW or PWM to 0
// we will drive PWM using (not) D2, so that when it is LOW, the outputs are in the high impedence state
// so we keep D1 permanently LOW
// we will go with PWM frequencies below 10KHz, so we can use the slow slew, which in turn means lower peak current.
// so we can leave the SLEW pin unattached.  If we want to get up to high PWM frequencies, then we need to set it HIGH.
// we make the following switchable:  enable (EN), direction (INV), PWM (not D2)
// we read the following:  status (not SF), current draw (FB)
// internal VDD is generated in the chip, the external VDD is only for the jumper lines for overriding defaults,
// which allows fewer wires to be attached.
// So our jumper overrides are as follows:
// D1 to GND
// INVERT to GND, since we are using both IN1 and IN2 (so we can coast)
// since the only jumper connections we are making are to ground, we do not need a connection to Vdd
// Also, since we will have +12V going into the large Vin port, we do not need a connection to the small Vin pin
// For feedback, When running in the forward or reverse direction, a ground-referenced 0.24% of load current
// is output to the FB pin.  Since our board uses a 200 ohm resistor to ground from this pin, we get about
// 525 mV per amp on that pin.  Since the analog range is 0 to 1023 corresponding to 0 to 5V, or 4.89 mV per analog value,
// we get a value of 107.4 per amp, so full scale 1023 corresponds to 9.52 amps
// inverting, get a count of 9.3 mA per count

// for the encoders, note that the timers on the mega:
// timer 0 pins A,B are 13,4
// timer 1 pins A,B are 11,12
// timer 2 pins A,B are 10,9  
// timer 3 pins A,B,C are 5,2,3
// timer 4 pins A,B,C are 6,7,8
// timer 5 pins A,B,C are 44,45,46

// for the battery monitor, hook it up with the + side soldered to Vout on the motor driver (+12)
// the negative side to ground and the middle to A4.  With the battery attached, 
// measure the voltage on Vout and divide it by the voltage on A4
// and enter that into the arduino sketch as the parameter VOLTAGE_DIVIDER_RATIO (assuming 22K and 10K resistors,
// the value should be 3.2)
// Also measure the fully charged battery value and enter that with the discharged value into the parameters
// FULL_BATTERY_VOLTAGE and ZERO_PERCENT_BATTERY_VOLTAGE
//
//
// wiring standards:
// red = +12
// black = ground
// yellow = +5
// white = motorA outside
// blue = motorA inside
// grey = motorB outside
// green = motor B inside
// orange = motorC outside
// violet = motorC inside


threeMotorsDriver::threeMotorsDriver()
{
    // Pin map
    // pins used are 0,1,3,4,5,6,16,17,18,19,22,24,26,30,32,34,40,42
    // the analog pins used, A5,A6,A7,A8, are set as #defines in the threeMotorsDriver.h file
    // if we later decide to use the servo library, then pins 9 and 10 will not be available for analogWrite()
    // available for future use: interrupt pins 2, PWM pins 7 - 13
    // available for I2C are pins 20 and 21 (these are also interrupt pins)
    IN1A = 24;
    IN2A = 22;
    STATUSA = 26;
    ENABLEAB = 28;
    PWMA = 4;  // PWM: 0 to 13. Provide 8-bit PWM output with the analogWrite() function.
    IN1B = 32;
    IN2B = 30;
    STATUSB = 34;
    PWMB = 5;
    IN1C = 40;
    IN2C = 42;
    STATUSC = 38;
    ENABLEC = 36;
    PWMC = 6;
    // interrupt pins on the mega are:
    // 2 (interrupt 0), 3 (interrupt 1), 18 (interrupt 5), 19 (4), 20 (3), and 21 (2)
    // we use them for motor encoders
    ENCA = 18; 
	INTERRUPTA = 5;
    ENCB = 19;
	INTERRUPTB = 4;
    ENCC = 20;
	INTERRUPTC = 3;
	

    pinMode(IN1A,OUTPUT);
    pinMode(IN2A,OUTPUT);
    pinMode(STATUSA,INPUT);
    pinMode(ENABLEAB,OUTPUT);
    pinMode(PWMA,OUTPUT);  // PWM: 0 to 13. Provide 8-bit PWM output with the analogWrite() function.
    
    pinMode(IN1B,OUTPUT);
    pinMode(IN2B,OUTPUT);
    pinMode(STATUSB,INPUT);
    pinMode(PWMB,OUTPUT);
    
    pinMode(IN1C,OUTPUT);
    pinMode(IN2C,OUTPUT);
    pinMode(STATUSC,INPUT);
    pinMode(ENABLEC,OUTPUT);
    pinMode(PWMC,OUTPUT);
    
    // interrupt pins on the mega are:
    // 2 (interrupt 0), 3 (interrupt 1), 18 (interrupt 5), 19 (4), 20 (3), and 21 (2)
    // we use them for motor encoders
    pinMode(ENCA,INPUT); 
    pinMode(ENCB, INPUT);
    pinMode(ENCC, INPUT);
	
	// turn on pullup resistors
	digitalWrite(ENCA, HIGH);
	digitalWrite(ENCB, HIGH);
	digitalWrite(ENCC, HIGH);
    
    // start disabled, with directions set to forward
    digitalWrite(ENABLEAB, LOW);
    digitalWrite(IN1A,HIGH);
    digitalWrite(IN2A,LOW);    
    analogWrite(PWMA, 0);
    
    digitalWrite(ENABLEAB, LOW);
    digitalWrite(IN1B,HIGH);
    digitalWrite(IN2B,LOW); 
    analogWrite(PWMB, 0);
    
    digitalWrite(ENABLEC, LOW);
    digitalWrite(IN1C,HIGH);
    digitalWrite(IN2C,LOW); 
    analogWrite(PWMC, 0);
}

void threeMotorsDriver::setSpeedA(int speed)
{
    digitalWrite(ENABLEAB, HIGH);
    if (speed < 0)
    {
        digitalWrite(IN1A,LOW);
        digitalWrite(IN2A,HIGH);   // turn reverse
        speed = -speed;
    }
    else
    {
        digitalWrite(IN1A,HIGH);
        digitalWrite(IN2A,LOW);   // turn forward
    }
    if (speed > 255) speed = 255;
    analogWrite(PWMA, speed);
}

void threeMotorsDriver::setSpeedB(int speed)
{
    digitalWrite(ENABLEAB, HIGH);
    if (speed < 0)
    {
        digitalWrite(IN1B,LOW);
        digitalWrite(IN2B,HIGH);   // turn reverse
        speed = -speed;
    }
    else
    {
        digitalWrite(IN1B,HIGH);
        digitalWrite(IN2B,LOW);   // turn forward
    }
    if (speed > 255) speed = 255;
    analogWrite(PWMB, speed);
}   

void threeMotorsDriver::setSpeedC(int speed)
{
    digitalWrite(ENABLEC, HIGH);
    if (speed < 0)
    {
        digitalWrite(IN1C,LOW);
        digitalWrite(IN2C,HIGH);   
        speed = -speed;
    }
    else
    {
        digitalWrite(IN1C,HIGH);
        digitalWrite(IN2C,LOW);  
    }
    if (speed > 255) speed = 255;
    analogWrite(PWMC, speed);
}

void threeMotorsDriver::setSpeedAB(int speedA, int speedB)
{
    digitalWrite(ENABLEAB, HIGH);
    if (speedA < 0)
    {
        digitalWrite(IN1A,LOW);
        digitalWrite(IN2A,HIGH);   // move backwards
        speedA = -speedA;
    }
    else
    {
        digitalWrite(IN1A,HIGH);
        digitalWrite(IN2A,LOW);   // move forward
    }
    if (speedA > 255) speedA = 255;
    
    if (speedB < 0)
    {
        digitalWrite(IN1B,LOW);
        digitalWrite(IN2B,HIGH);   // turn reverse
        speedB = -speedB;
    }
    else
    {
        digitalWrite(IN1B,HIGH);
        digitalWrite(IN2B,LOW);   // turn forward
    }
    analogWrite(PWMA, speedA);
    analogWrite(PWMB, speedB);
}

void threeMotorsDriver::setBrakesAB()
{
    digitalWrite(ENABLEAB, LOW);
    analogWrite(PWMA, 0);
    analogWrite(PWMB, 0);
}

void threeMotorsDriver::setBrakesC()
{
    digitalWrite(ENABLEC, LOW);
    analogWrite(PWMC, 0);
}

void threeMotorsDriver::setCoastA()
{
    digitalWrite(ENABLEAB, HIGH);
    digitalWrite(IN1A, LOW);
    digitalWrite(IN2A, LOW);
    analogWrite(PWMA, 255);   
}
    
void threeMotorsDriver::setCoastB()
{
    digitalWrite(ENABLEAB, HIGH);
    digitalWrite(IN1B, LOW);
    digitalWrite(IN2B, LOW);
    analogWrite(PWMB, 255);   
}

void threeMotorsDriver::setCoastC()
{
    digitalWrite(ENABLEC, HIGH);
    digitalWrite(IN1C, LOW);
    digitalWrite(IN2C, LOW);
    analogWrite(PWMC, 255);   
}

void threeMotorsDriver::setCoastAB()
{
    digitalWrite(ENABLEAB, HIGH);
    digitalWrite(IN1A, LOW);
    digitalWrite(IN2A, LOW);
    digitalWrite(IN1B, LOW);
    digitalWrite(IN2B, LOW);
    analogWrite(PWMA, 255); 
    analogWrite(PWMB, 255);    
}

int threeMotorsDriver::getCurrentA()  // returns mA
{
    // 9.3 ma per count, we'll use 10 to stick with integer arithmetic
    return (analogRead(FBA) * 10);
}

int threeMotorsDriver::getCurrentB()
{
    return (analogRead(FBB) * 10);
}

int threeMotorsDriver::getCurrentC()
{
    return (analogRead(FBC) * 10);
}

unsigned char threeMotorsDriver::getStatusA()
{
    return(digitalRead(STATUSA));
}

unsigned char threeMotorsDriver::getStatusB()
{
    return(digitalRead(STATUSB));
}

unsigned char threeMotorsDriver::getStatusC()
{
    return(digitalRead(STATUSC));
}



