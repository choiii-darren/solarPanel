#include <Wire.h>
#include <AccelStepper.h>
#include <Adafruit_INA219.h>
#include <BluetoothSerial.h>

Adafruit_INA219 ina219;
BluetoothSerial transmitter;

// Assign the appropriate pin numbers
const int photoresistorPin1 = 25; // analog input pin for the first photoresistor
const int photoresistorPin2 = 27; // analog input pin for the second photoresistor

// Threshold voltage difference (customize as needed)
float thresholdA = 2.0; // voltage difference for motor to start turning
float thresholdB = 1.0; // voltage difference for motor to stop turning

// Pin numbers for the stepper motor driver
const int motorPin1 = 2; // step pin
const int motorPin2 = 3; // direction pin

// Pin numbers for reading data from ina219
const int voltagePin = 16;
const int currentPin = 17;

// for the transmitter callback
bool cong = false;
long start = 0;

// Initialize the stepper library
AccelStepper stepper = AccelStepper(AccelStepper::DRIVER, motorPin1, motorPin2);

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
  // Start the I2C interface
  Wire.begin();

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

  // Initialize the INA219 module
  if (!ina219.begin())
  {
    Serial.println('INA219 module crashed');
    // If something went wrong, we will get here
    while (1)
      ;
  }

  // Set the maximum speed and acceleration (adjust as needed)
  stepper.setMaxSpeed(1000.0);
  stepper.setAcceleration(500.0);
}

void loop()
{
  // Read the voltage from both photoresistors
  float voltage1 = analogRead(photoresistorPin1) * (5.0 / 1023.0);
  float voltage2 = analogRead(photoresistorPin2) * (5.0 / 1023.0);

  // Calculate the difference in voltages
  float voltageDiff = voltage1 - voltage2;

  // If the voltage difference is above thresholdA, start the motor
  if (voltageDiff >= thresholdA)
  {
    stepper.move(1); // change direction if needed
  }
  else if (voltageDiff <= thresholdA * -1)
  {
    stepper.move(-1);
  }
  // If the voltage difference is below thresholdB, stop the motor
  else if (voltageDiff <= thresholdB || voltageDiff >= thresholdB * -1)
  {
    stepper.stop();
  }

  // If the motor has steps to make, make one
  if (stepper.distanceToGo() != 0)
  {
    stepper.run();
  }

  // reports data gathered every 60 seconds
  if (millis() - start >= 60000)
  {
    // read from ina219 power collector, voltage, current and transmit it
    float voltage = digitalRead(voltagePin) * (5.0/1023.0);
    float current = digitalRead(currentPin) * (5.0/1023.0);
    if (!cong)
    {
      if (!sendDataBlueTooth(voltage, current))
      {
        Serial.println('Failed to transmit data');
      };
      start = millis();
    }
  }
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
