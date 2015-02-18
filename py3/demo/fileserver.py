import sys
py3 = sys.version_info[0] > 2
#--------------------------------------------------------------------------------
port = 80
document_root = "."
#--------------------------------------------------------------------------------
if py3: from http.server import BaseHTTPRequestHandler, HTTPServer
else: from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
BaseHTTPRequestHandler.wbufsize = -1
if py3: from urllib.parse import urlparse
else: from urlparse import urlparse
#--------------------------------------------------------------------------------
class MyHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        scheme,netloc,path,params,query,fragment = urlparse(self.path)
        self.send_file(document_root+path)
        return
    def reply(self, content_type, data):
        self.send_response(200)
        self.send_header('Content-type',content_type)
        self.end_headers()
        self.wfile.write( data )
        return
    def send_file(self, path):
        try:
            f = open(path,'rb')
            data = f.read()
            content_type = 'text/html' if path.endswith('.html') \
                           else 'text/plain'
            self.reply(content_type, data)
            f.close()
        except IOError:
            self.send_error(404, path+' not found')
            
#--------------------------------------------------------------------------------
if __name__ == '__main__':
    try:
        server = HTTPServer(('', port), MyHandler)
        server.serve_forever(0.02)
    except KeyboardInterrupt:
        print("Shutting down server...")
        server.socket.close()
