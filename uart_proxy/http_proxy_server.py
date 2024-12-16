from http.server import HTTPServer

class HttpProxyServer(HTTPServer):
    intermediate_request_handler = None

    def __init__(self, intermediate_request_handler, server_address, RequestHandlerClass, bind_and_activate=True):
        super().__init__(server_address, RequestHandlerClass, bind_and_activate)
        self.intermediate_request_handler = intermediate_request_handler