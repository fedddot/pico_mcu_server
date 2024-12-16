from http.server import BaseHTTPRequestHandler
from http import HTTPStatus
import json

class HttpProxyHandler(BaseHTTPRequestHandler):

    def _retrieve_http_response(self, intermediate_response_object):
        match intermediate_response_object["code"]:
            case 0:
                return HTTPStatus.OK
            case 1:
                return HTTPStatus.METHOD_NOT_ALLOWED
            case 2:
                return HTTPStatus.NOT_FOUND
            case 3:
                return HTTPStatus.BAD_REQUEST
            case 4:
                return HTTPStatus.UNSPECIFIED
            case _:
                raise Exception("unsupported result code received")

    def _run_intermediate_request(self, intermediate_request):
        try:
            response = self.server.intermediate_request_handler.run_request(intermediate_request)
            string_response = json.dumps(response)
            self.send_response(self._retrieve_http_response(response))
            self.send_header('Content-type', 'json')
            self.end_headers()
            self.wfile.write(bytes(string_response, 'utf-8'))
        except Exception as e:
            self.send_response(HTTPStatus.INTERNAL_SERVER_ERROR)
            self.send_header('Content-type', 'json')
            self.end_headers()
            response = {
                "what": str(e)
            }
            string_response = json.dumps(response)
            self.wfile.write(bytes(string_response, 'utf-8'))   

    def _read_data(self):
        content_length = int(self.headers['Content-Length'])
        return self.rfile.read(content_length)

    def do_GET(self):
        self._run_intermediate_request(
            {
                "method": "READ",
                "path": self.path.strip("/"),
                "body": {}
            }
        )
        
    def do_POST(self):
        body_data = self._read_data()
        body = {}
        if len(body_data) > 0:
            body = json.loads(body_data.decode("utf-8"))
        self._run_intermediate_request(
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
        self._run_intermediate_request(
            {
                "method": "UPDATE",
                "path": self.path.strip("/"),
                "body": body
            }
        )

    def do_DELETE(self):
        self._run_intermediate_request(
            {
                "method": "DELETE",
                "path": self.path.strip("/"),
                "body": {}
            }
        )