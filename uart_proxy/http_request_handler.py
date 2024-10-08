from http.server import HTTPServer, BaseHTTPRequestHandler
from http import HTTPStatus
from uart_request_handler import UartRequestHandler
import serial
import json
import re

class HttpToUartProxyServer(HTTPServer):
    uart_request_handler = None

    def __init__(self, uart_request_handler, server_address, RequestHandlerClass, bind_and_activate=True):
        super().__init__(server_address, RequestHandlerClass, bind_and_activate)
        self.uart_request_handler = uart_request_handler
    
class HttpRequestHandler(BaseHTTPRequestHandler):

    def _http_method_to_uart_method(self, http_method):
        return http_method
    
    def _retrieve_http_response(self, uart_response):
        match uart_response["code"]:
            case 0:
                return HTTPStatus.OK
            case 2:
                return HTTPStatus.NOT_FOUND
            case _:
                return HTTPStatus.INTERNAL_SERVER_ERROR
    
    def _run_uart_request(self, uart_request):
        try:
            response = self.server.uart_request_handler.run_request(uart_request)
            string_response = json.dumps(response)
            self.send_response(self._retrieve_http_response(response))
            self.send_header('Content-type', 'json')
            self.end_headers()
            self.wfile.write(bytes(string_response, 'utf-8'))
        except:
            self.send_response(HTTPStatus.INTERNAL_SERVER_ERROR)
            self.send_header('Content-type', 'json')
            self.end_headers()
            self.wfile.write(b'{}')   

    def do_GET(self):
        self._run_uart_request(
            {
                "method": "READ",
                "path": self.path.strip("/"),
                "body": {}
            }
        )
    
    def _read_data(self):
        content_length = int(self.headers['Content-Length'])
        return self.rfile.read(content_length)
        
    def do_POST(self):
        body_data = self._read_data()
        body = {}
        if len(body_data) > 0:
            body = json.loads(body_data.decode("utf-8"))
        self._run_uart_request(
            {
                "method": "CREATE",
                "path": self.path.strip("/"),
                "body": body
            }
        )

    def do_PUT(self):
        body_data = self._read_data()
        body = {}
        if len(body_data) > 0:
            body = json.loads(body_data.decode("utf-8"))
        self._run_uart_request(
            {
                "method": "UPDATE",
                "path": self.path.strip("/"),
                "body": body
            }
        )

    def do_DELETE(self):
        self._run_uart_request(
            {
                "method": "DELETE",
                "path": self.path.strip("/"),
                "body": {}
            }
        )

uart_handler = UartRequestHandler()
server = HttpToUartProxyServer(uart_handler, ("127.0.0.1", 5000), HttpRequestHandler)
server.serve_forever()
