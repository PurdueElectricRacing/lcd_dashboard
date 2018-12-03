import serial
import time

if __name__ == "__main__":
    
    while 1:
        with serial.Serial('COM1', 9600, timeout=1) as ser:
            #x = ser.readline()
            #print(x)
            #ser.write(b'sup')
            #print("writing data")
            ser.write('hello!\n'.encode('utf-8'))