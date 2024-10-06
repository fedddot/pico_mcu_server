import serial
import time
import sys
import json
import re

def get_param_from_args(param, args):
    prefix = "--{}=".format(param)
    for arg in args:
        if not arg.startswith(prefix):
            continue
        return arg[len(prefix):]
    raise Exception("param {} is missing")

port = get_param_from_args("port", sys.argv)
baud = int(get_param_from_args("baud", sys.argv))
ser = serial.Serial(
    port,
    baud,
    timeout=100
)
print("opened port: {}, baud {}".format(port, baud))

create_request = {
    "method": "CREATE",
    "path": "gpios",
    "body": {
        "id": "25",
        "dir": 1
    }
}
update_high_request = {
    "method": "UPDATE",
    "path": "gpios/25",
    "body": {
        "state": 1
    }
}
update_low_request = {
    "method": "UPDATE",
    "path": "gpios/25",
    "body": {
        "state": 0
    }
}
delete_request = {
    "method": "DELETE",
    "path": "gpios/25",
    "body": {}
}

def run_request(ser, request):
    str_request = json.dumps(request)
    print("sending request to the server:\n{}".format(str_request))
    ser.write(("MSG_HEAD" + str_request + "MSG_TAIL").encode("utf-8"))
    response_str = ""
    while True:
        new_data = ser.read_all().decode("utf-8")
        response_str += new_data
        if "MSG_HEAD" in response_str and "MSG_TAIL" in response_str:
            break;
    extracted_response = re.search('MSG_HEAD(.*)MSG_TAIL', response_str).group(1)
    print("received extracted report: {}".format(extracted_response))
    return json.loads(extracted_response)

run_request(ser, create_request)
run_request(ser, update_high_request)
run_request(ser, update_low_request)
run_request(ser, delete_request)

ser.close()