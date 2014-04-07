import sys
py3 = sys.version_info[0] > 2
#--------------------------------------------------------------------------------
port = 80 ##8080
datafile = 'temperature.dat'
plotfile = 'temperature.png'
webpage = 'temperature.html'
#--------------------------------------------------------------------------------
if py3: from http.server import BaseHTTPRequestHandler, HTTPServer
else: from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
BaseHTTPRequestHandler.wbufsize = -1
if py3: from urllib.parse import urlparse, parse_qs
else: from urlparse import urlparse, parse_qs
#--------------------------------------------------------------------------------
class MyHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        scheme,netloc,path,params,query,fragment = urlparse.urlparse(self.path)

        upd, displ = 10, '24h'
        query = parse_qs(query)
        if 'update' in query: upd = int(query['update'][0])
        if 'display' in query: displ = query['display'][0]

        if path.endswith('.html') or path.endswith('htm'):
            self.send_webpage(upd,displ)
        elif path.endswith('.dat'):
            f=open(datafile,'r')
            self.reply('text/plain', f.read() )
            f.close()
        elif path.endswith('.png'):
            f=open(plotfile,'rb')
            self.reply('image/png', f.read() )
            f.close()
        elif path.startswith('/temp'):
            self.reply('text/plain', '%.3f'%temp )
        elif path.startswith('/max'):
            self.reply('text/plain', '%.3f'%max )
        elif path.startswith('/min'):
            self.reply('text/plain', '%.3f'%min )
        elif path.startswith('/serial'):
            self.reply('text/plain', serial )
        else: self.send_webpage(upd,displ)
        return
    def reply(self, content_type, data):
        self.send_response(200)
        self.send_header('Content-type',content_type)
        self.end_headers()
        if isinstance(data, str): data = data.encode('utf-8')
        self.wfile.write( data )
        return
    def send_webpage(self,upd=10,displ='24h'):
        f=open(webpage,'r')
        ppage = f.read() \
                .replace('{serial}',serial) \
                .replace('{temp}','%-2.3f'%temp) \
                .replace('{min}','%-2.3f'%min) \
                .replace('{max}','%-2.3f'%max) \
                .replace('{update}',str(upd))
        if displ:
            ppage = ppage.replace('{plotfile}',plotfile+'?display='+displ) \
                        .replace('"'+displ+'"','"'+displ+'" checked')
        else: ppage = ppage.replace('{plotfile}',plotfile)
        
        self.reply('text/html', ppage)
        f.close()
        return        
#--------------------------------------------------------------------------------
from threading import Thread
from time import time, sleep
import os, sys
#--------------------------------------------------------------------------------
serial = ''
temp = 0
max = 0
min = 0
upd_read = 1
upd_store = 10
#--------------------------------------------------------------------------------
plot_cmd='''
set xdata time; set timefmt "%%s"
set xlabel "Time (GMT)"; set ylabel "Temperature (\\260C)"
set term png size 1024,400
set output "%(plotfile)s"
plot 'temperature.dat' u 1:2 w l title "%(serial)s"
set output
exit
'''

def create_plot(): 
    cmd = plot_cmd % {'serial':serial, 'plotfile':plotfile}
    f = open('cmd.gp', 'w')
    f.write(cmd)
    f.close()
    if sys.platform=='win32': 
        os.system('pgnuplot.exe cmd.gp')
    else: os.system('gnuplot cmd.gp')
    
#--------------------------------------------------------------------------------

def serve():
    try:
        import mbug_2810
        thermo = mbug_2810.open()
        serial = thermo.serial()
        global temp, max, min
        temp = max = min = thermo.read()
    except:
        print('Thermometer not found. Webserver mode only.')
        thermo = None

    server = HTTPServer(('', port), MyHandler)
    server_thread = Thread(target=server.serve_forever,args=(0.1,))

    f = open(datafile,'a')
    f.write('\n')
    f.close()
    
    try:        
        server_thread.start()
        print("Start server on port %s. Hit Ctrl+C to stop." % port)

        next_read = next_store = time()
        while(1):
            now = time()
            if now>=next_read and thermo:
                temp = thermo.read()
                if temp>max: max=temp
                if temp<min: min=temp
                next_read += upd_read
            if now>=next_store:
                f = open(datafile,'a')
                f.write("%.2f\t%.3f\n"%(now,temp))
                f.close()
                create_plot()
                next_store += upd_store
            sleep(0.1)
            
    except:
        server.shutdown()
        server.socket.close()
        if thermo: thermo.close()
        os.remove('cmd.gp')
        raise
        
#--------------------------------------------------------------------------------

if __name__ == '__main__':
    serve()
