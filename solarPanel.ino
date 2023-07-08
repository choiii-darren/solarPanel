#include <Wire.h>
#include <AccelStepper.h>
#include <Adafruit_INA219.h>

 

Adafruit_INA219 ina219;

 

// Assign the appropriate pin numbers
const int photoresistorPin1 = A0; // analog input pin for the first photoresistor
const int photoresistorPin2 = A1; // analog input pin for the second photoresistor

 

// Threshold voltage difference (customize as needed)
float thresholdA = 2.0; // voltage difference for motor to start turning
float thresholdB = 1.0; // voltage difference for motor to stop turning

 

// Pin numbers for the stepper motor driver
const int motorPin1 = 2; // step pin
const int motorPin2 = 3; // direction pin

 

// Initialize the stepper library
AccelStepper stepper = AccelStepper(AccelStepper::DRIVER, motorPin1, motorPin2);

 

void setup() {
  // Start the I2C interface
  Wire.begin();

 

  // Initialize the INA219 module
  if (! ina219.begin()) {
    // If something went wrong, we will get here
    while (1);
  }

  // Set the maximum speed and acceleration (adjust as needed)
  stepper.setMaxSpeed(1000.0);
  stepper.setAcceleration(500.0);
}

 

void loop() {
  // Read the voltage from both photoresistors
  float voltage1 = analogRead(photoresistorPin1) * (5.0 / 1023.0);
  float voltage2 = analogRead(photoresistorPin2) * (5.0 / 1023.0);

 

  // Calculate the difference in voltages
  float voltageDiff = abs(voltage1 - voltage2);

 

  // If the voltage difference is above thresholdA, start the motor
  if (voltageDiff > thresholdA) {
    stepper.move(1); // change direction if needed
  }
  // If the voltage difference is below thresholdB, stop the motor
  else if (voltageDiff < thresholdB) {
    stepper.stop();
  }

 

  // If the motor has steps to make, make one
  if (stepper.distanceToGo() != 0) {
    stepper.run();
  }
}
