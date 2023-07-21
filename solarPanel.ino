//Include necessary libraries
#include <Wire.h>
#include <AccelStepper.h>
#include <Adafruit_INA219.h>

//Initialize an object for INA219 power sensor
Adafruit_INA219 ina219;

//Define the pins for photoresistors
const int photoresistorPin1 = A1;
const int photoresistorPin2 = A0;

//Define the threshold for voltage difference
float thresholdA = 1;
float normalizationDifference = 12;

//Define the pins for the stepper motor
const int motorPin1 = 13;
const int motorPin2 = 12;
const int motorPin3 = 11;
const int motorPin4 = 10;

//Define the base pin
const int base = 7;

float freq = 2;
int del = 200;

//Variables to track time
long dataMillis = 0;
long servoMillis = 0;

//Initialize the stepper motor object
AccelStepper stepper(AccelStepper::FULL4WIRE, motorPin1, motorPin3, motorPin2, motorPin4);

//Setup function
void setup()
{
  //Define the pins as Input or Output
  pinMode(base, OUTPUT);
  pinMode(photoresistorPin1, INPUT);
  pinMode(photoresistorPin2, INPUT);

  //Start Serial communication
  Serial.begin(115200);
  while (!Serial) {
      delay(1); //wait until the serial communication is started
  }

  //Start INA219 sensor
  if (!ina219.begin()) {
    while (1) { delay(10); } //halt execution if INA219 is not found
  }
  ina219.setCalibration_16V_400mA();
  
  Wire.begin();
  
  //Setup stepper motor
  stepper.setMaxSpeed(100);
  stepper.setAcceleration(20);
  stepper.moveTo(500);
  
  servoMove(); //Move the servo at the beginning
}

//Main loop function
void loop()
{
  //Check if 15 minutes have passed to move the servo
  if (millis() - servoMillis >= 900000)
  {
    servoMove();
    servoMillis = millis(); //update the time
  }

  //Check if 60 seconds have passed to send data
  if (millis() - dataMillis >= 60000)
  {
    char result[50];
    getPanelData(result); //get the panel data
    if (!sendData(result)) //send data
    {
      Serial.println('Failed to transmit data');
    };
    dataMillis = millis(); //update the time
  }
}

//Function to move the servo
void servoMove()
{
  //Read voltages from photoresistors
  float voltage1 = analogRead(photoresistorPin1);
  float voltage2 = analogRead(photoresistorPin2) + normalizationDifference;

  float voltageDiff = voltage1 - voltage2;

  //Keep adjusting the stepper motor position until the voltage difference is below the threshold
  while (abs(voltageDiff) >= thresholdA) {
    if (voltageDiff >= thresholdA)
    {
      stepper.moveTo(stepper.currentPosition() + 5); //move stepper clockwise
    }
    else if (voltageDiff <= thresholdA * -1)
    {
      stepper.moveTo(stepper.currentPosition() - 5); //move stepper counterclockwise
    }
    stepper.run(); //execute the stepper command

    //Read voltages from photoresistors again
    voltage1 = analogRead(photoresistorPin1);
    voltage2 = analogRead(photoresistorPin2) + normalizationDifference;

    voltageDiff = voltage1 - voltage2;  
  }
  stepper.stop(); //stop the stepper motor
} 

//Function to get the voltage and current data from the panel
void getPanelData(char *result) {
  digitalWrite(base, LOW);
  delay(del);
  float voltage = ina219.getBusVoltage_V(); //read voltage
  
  digitalWrite(base, HIGH);
  delay(del);
  float current = ina219.getCurrent_mA(); //read current
  
  delay(del);

  //Convert voltage and current to int to get integer and fractional part
  int voltageInt = (int)voltage;
  int voltageFraction = (voltage - voltageInt) * 100;
  int currentInt = (int)current;
  int currentFraction = (current - currentInt) * 100;

  //Compose the result string
  sprintf(result, "&VOLTAGE=%d.%02d,&CURRENT=%d.%02d", voltageInt, abs(voltageFraction), currentInt, abs(currentFraction));
}

//Function to send the data
bool sendData(char* string)
{
   Serial.println(string); //Print the data to the serial port
   return true; //return true if successful
}
