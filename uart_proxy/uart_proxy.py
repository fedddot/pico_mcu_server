import sys
from http_proxy_handler import HttpProxyHandler
from http_proxy_server import HttpProxyServer
from uart_proxy_handler import UartProxyHandler

def get_param_from_args(param, args):
    prefix = "--{}=".format(param)
    for arg in args:
        if not arg.startswith(prefix):
            continue
        return arg[len(prefix):]
    raise Exception("param {} is missing")

intermediate_handler = UartProxyHandler(
    port = get_param_from_args("port", sys.argv),
    baud = int(get_param_from_args("baud", sys.argv))
)

proxy_server = HttpProxyServer(
    intermediate_request_handler = intermediate_handler,
    server_address = get_param_from_args("host", sys.argv)
) 