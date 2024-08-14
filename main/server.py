from functools import cached_property
from http.cookies import SimpleCookie
from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.parse import parse_qsl, urlparse

import time

class WebRequestHandler(BaseHTTPRequestHandler):
    @cached_property
    def url(self):
        return urlparse(self.path)

    @cached_property
    def query_data(self):
        return dict(parse_qsl(self.url.query))

    @cached_property
    def post_data(self):
        content_length = int(self.headers.get("Content-Length", 0))
        return self.rfile.read(content_length)

    @cached_property
    def form_data(self):
        return dict(parse_qsl(self.post_data.decode("utf-8")))

    @cached_property
    def cookies(self):
        return SimpleCookie(self.headers.get("Cookie"))

    def get_response(self):
        with open('webpages\\index.html', 'r') as file:
            data = file.read()
        return data.encode()

    def do_GET(self):
        self.send_response(200)
        if self.path=='/':
            self.end_headers()
            self.wfile.write(self.get_response())
        elif self.path.startswith('/datetime'):
            args = self.path[self.path.find('?'):]
            cmd = args[args.find('cmd=')+len('cmd='):]
            if cmd == 'req':
                print("request date")
                self.end_headers()
            elif cmd == 'get':
                print("get date")
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                now = time.localtime()
                rsp = '{"datetime":"'+str(now.tm_year)+'-'+str(now.tm_mon)+'-'+str(now.tm_mday)+' '+str(now.tm_hour)+':'+str(now.tm_min)+':'+str(now.tm_sec)+'"}'
                self.wfile.write(rsp.encode())
        elif self.path=='/info':
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            self.wfile.write(b'{"online":true}')
        else:
            print("here")
            self.end_headers()

    def do_POST(self):
        self.send_response(200)
        print(self.post_data)

if __name__ == "__main__":
    server = HTTPServer(("0.0.0.0", 8000), WebRequestHandler)
    server.serve_forever()
