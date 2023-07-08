import serial
import time
import serial.tools.list_ports
import csv

'''
import serial
ports = serial.tools.list_ports.comports()

for i,port in enumerate(ports): 
    print(str(i) + ': ' + port.device)
# Change "4" to ESP32 bluetooth port
port = ports[4].device

comm_rate = 115200 #baud
fname = "data.csv" #location

#Begin communication
ser = serial.Serial(port, comm_rate)

#optional
ser.write(bytes('Hi', 'utf-8'))

file = open(fname, "a")
getData=ser.readline()
dataString = getData.decode('utf-8')[:-2]
'''

# Set up the Bluetooth serial connection
# Replace '/dev/rfcomm0' with the appropriate serial port on your system
ports = serial.tools.list_ports.comports()
port = ports[4].device

ser = serial.Serial(port, 115200)

# Wait for the Bluetooth connection to be established
time.sleep(5)

# open csv file
with open('solarData.csv', 'w', newline='') as file:
    writer = csv.writer(file)
writer.writerow(['Voltage', 'Current'])
# Main loop to receive data
while True:
    try:
        if ser.in_waiting > 0:
            # Read the incoming data
            data = ser.readline().decode().strip()
            print("Received data:", data)

            # Parse the received data
            values = data.split(",")
            sensor_data = {}
            for value in values:
                key, value = value.split("=")
                sensor_data[key] = value

            # Process the sensor data as needed
            voltage = float(sensor_data.get("VOLTAGE", 0.0))
            current = float(sensor_data.get("CURRENT", 0.0))

            # write to csv file
            writer.writerow([voltage, current])

        time.sleep(0.1)  # Small delay before checking for new data
    except KeyboardInterrupt:
        # write all the data to csv file
        writer.close()
        print('Data was saved')
