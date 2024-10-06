import serial
import json
import re

class UartRequestHandler:
    _port = None
    _baud = None
    _head = None
    _tail = None

    def __init__(self, port = "/dev/ttyACM0", baud = 115200, head = "MSG_HEAD", tail = "MSG_TAIL"):
        self._port = port
        self._baud = baud
        self._head = head
        self._tail = tail
    
    def run_request(self, request):
        ser = serial.Serial(
            port = self._port,
            baudrate = self._baud,
            timeout = 100
        )
        str_request = self._head + json.dumps(request) + self._tail
        ser.write(str_request.encode("utf-8"))
        response_str = ""
        while True:
            new_data = ser.read_all().decode("utf-8")
            response_str += new_data
            search_result = re.search("{}(.*){}".format(self._head, self._tail), response_str)
            if None == search_result:
                continue
            return json.loads(search_result.group(1))        