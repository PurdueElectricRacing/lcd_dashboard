import serial
import time

if __name__ == "__main__":
    
    with serial.Serial('COM1', 9600, timeout=1) as ser:
        # x = ser.readline()
        # print(x)
        # print writing data
        ser.write('Charge.val=20'.encode('utf-8'))
        ser.write(0xFF)
        ser.write(0xFF)
        ser.write(0xFF)