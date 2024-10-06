import serial
import time
import sys
import json
from uart_request_handler import UartRequestHandler

def get_param_from_args(param, args):
    prefix = "--{}=".format(param)
    for arg in args:
        if not arg.startswith(prefix):
            continue
        return arg[len(prefix):]
    raise Exception("param {} is missing")

uart_handler = UartRequestHandler(
    port = get_param_from_args("port", sys.argv),
    baud = int(get_param_from_args("baud", sys.argv))
)

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
read_request = {
    "method": "READ",
    "path": "gpios/25",
    "body": {}
}
delete_request = {
    "method": "DELETE",
    "path": "gpios/25",
    "body": {}
}

def run_request(handler, request):
    print("\nrunning request:\n{}".format(request))
    response = handler.run_request(request)
    print("received response:\n{}".format(response))

run_request(uart_handler, create_request)
run_request(uart_handler, read_request)
run_request(uart_handler, update_high_request)
run_request(uart_handler, read_request)
run_request(uart_handler, update_low_request)
run_request(uart_handler, read_request)
run_request(uart_handler, delete_request)