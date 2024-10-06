import serial
import time

ser = serial.Serial(port='/dev/ttyACM0', baudrate=115200, timeout=100)
print(ser.name)
ser.write(b'MSG_HEAD{\"method\": \"CREATE\", \"path\":\"gpios\", \"body\": {\"id\": \"10\", \"dir\": 0}}MSG_TAIL')
time.sleep(5)
print(ser.read_all())
ser.close()