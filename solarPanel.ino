#include <Wire.h>
#include <AccelStepper.h>
#include <Adafruit_INA219.h>
#include <BluetoothSerial.h>

Adafruit_INA219 ina219;
BluetoothSerial transmitter;

// Assign the appropriate pin numbers
const int photoresistorPin1 = 15; // analog input pin for the first photoresistor
const int photoresistorPin2 = 4; // analog input pin for the second photoresistor

// Threshold voltage difference (customize as needed)
float thresholdA = 100; // voltage difference for motor to start turning

// Pin numbers for the stepper motor driver
const int motorPin1 = 23;
const int motorPin2 = 19;
const int motorPin3 = 18;
const int motorPin4 = 17;

// Pin numbers for reading data from ina219
const int base = 16;

float freq = 2; // Hz
// Delay after changing state of transistor
int del = 2;

// for the transmitter callback
bool cong = false;
long dataMillis = 0;
long servoMillis = 0;

// Initialize the stepper library
AccelStepper stepper(AccelStepper::FULL4WIRE, motorPin1, motorPin3, motorPin2, motorPin4);

void BT_EventHandler(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  if (event == ESP_SPP_START_EVT)
  {
    Serial.println("Initialized SPP");
  }
  else if (event == ESP_SPP_SRV_OPEN_EVT)
  {
    Serial.println("Client connected");
  }
  else if (event == ESP_SPP_CLOSE_EVT)
  {
    Serial.println("Client disconnected");
  }
  else if (event == ESP_SPP_CONG_EVT)
  {
    cong = true;
    Serial.println("Client not listening");
  }
}

void setup()
{
  pinMode(base, OUTPUT);
  Serial.begin(115200);
  while (!Serial) {
      // will pause Zero, Leonardo, etc until serial console opens
      delay(1);
  }
    
  Serial.println("Hello!");
  
  // Initialize the INA219.
  // By default the initialization will use the largest range (32V, 2A).  However
  // you can call a setCalibration function to change this range (see comments).
  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }
  // To use a slightly lower 32V, 1A range (higher precision on amps):
  //ina219.setCalibration_32V_1A();
  // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
  ina219.setCalibration_16V_400mA();

  Serial.println("Measuring voltage and current with INA219 ...");
  // Start the I2C interface
  Wire.begin();
  /*
  // Start the Bluetooth module
  transmitter.begin('bluetoothGroup23');
  // if receiver is not ready yet
  while (!transmitter.available())
  {
    delay(50);
  }
  if (transmitter.available())
  {
    Serial.println('bluetooth ready');
  }

  transmitter.register_callback(BT_EventHandler);
  */

  // Set the maximum speed and acceleration (adjust as needed)
  stepper.setMaxSpeed(100);
  stepper.setAcceleration(20);
  stepper.moveTo(500);
}

void loop()
{
  if (millis() - servoMillis >= 10000) // runs the moving function every 45 mins
  {
    servoMove();
  }
  // reports data gathered every minute
  if (millis() - dataMillis >= 1000)
  {
    // read from ina219 power collector, voltage, current and transmit it
    digitalWrite(base, LOW);
    float current = ina219.getCurrent_mA();
    delay(del);
    digitalWrite(base, HIGH);
    float voltage = ina219.getBusVoltage_V();
    delay(del);;
    Serial.print("Panel output: ");
    Serial.print(voltage);
    Serial.print(", "); 
    Serial.println(current);
    /*if (!cong)
    {
      if (!sendDataBlueTooth(voltage, current))
      {
        Serial.println('Failed to transmit data');
      };
      dataMillis = millis();
    }*/
  }
}

void servoMove()
{
  // Read the voltage from both photoresistors
  float voltage1 = analogRead(photoresistorPin1);
  float voltage2 = analogRead(photoresistorPin2);
  Serial.print("Photoresistors: ");
  Serial.print(voltage1);
  Serial.print(", ");
  Serial.println(voltage2);

  // Calculate the difference in voltages
  float voltageDiff = voltage1 - voltage2;
  
  while (abs(voltageDiff) >= thresholdA) {
    // If the voltage difference is above thresholdA, start the motor
    if (voltageDiff >= thresholdA)
    {
      Serial.println("Moving clockwise");
      stepper.moveTo(stepper.currentPosition() + 5); // this goes clockwise, when we wire the pins, gotta make sure the one on the right of the motor is pin 25
    }
    else if (voltageDiff <= thresholdA * -1)
    {
      Serial.println("Moving counter-clockwise");
      stepper.moveTo(stepper.currentPosition() - 5); // this goes counter clockwise, make sure the photoresistor on the left is pin 27
    }
    stepper.run();

    voltage1 = analogRead(photoresistorPin1);
    voltage2 = analogRead(photoresistorPin2);
    Serial.print("Photoresistors: ");
    Serial.print(voltage1);
    Serial.print(", ");
    Serial.println(voltage2);
  
    // Calculate the difference in voltages
    voltageDiff = voltage1 - voltage2;
  }

  /*
  // TODO: test if this actually works, since this doesn't make much sense when you already have the .move function
  //  If the motor has steps to make, make one
  if (stepper.distanceToGo() != 0)
  {
    stepper.run();
  }
  */
}

bool sendDataBlueTooth(int voltage, int current)
{
  if (transmitter.available())
  {
    String dataString = "&VOLTAGE=" + String(voltage) + "," + "&CURRENT=" + String(current);
    transmitter.println(dataString);
    return true;
  }
  return false;
}