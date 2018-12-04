import serial
import time

if __name__ == "__main__":
    while 1:
        time.sleep(10)
        with serial.Serial('COM11', 9600, timeout=1) as ser:
            # x = ser.read_all()
            # print(x)

            # print writing data
            # print ('charge.val=20'.encode('ascii'))
            ser.write("Charge.val=20".encode('ascii'))
