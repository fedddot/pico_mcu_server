import os
from http_proxy_handler import HttpProxyHandler
from http_proxy_server import HttpProxyServer
from uart_proxy_handler import UartProxyHandler

def get_param(name):
    val = os.environ[name]
    if None == val:
        raise Exception("failed to get {} from the environment".format(name))
    return val

intermediate_handler = UartProxyHandler(
    port = get_param("UART_PORT"),
    baud = int(get_param("UART_BAUD"))
)

host = get_param("PROXY_HOST")
port = int(get_param("PROXY_PORT"))
proxy_server = HttpProxyServer(
    intermediate_request_handler = intermediate_handler,
    server_address = (host, port),
    RequestHandlerClass = HttpProxyHandler
)

proxy_server.serve_forever()