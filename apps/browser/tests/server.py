"""Deterministic loopback HTTP/HTTPS fixtures; no external requests or trust changes."""
import argparse, base64, gzip, json, socket, ssl, struct, threading, time, zlib
import io
from PIL import Image
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlsplit, parse_qs

def png(width=32, height=24, pixels=True):
    def chunk(tag, data):
        return struct.pack('>I', len(data))+tag+data+struct.pack('>I', zlib.crc32(tag+data)&0xffffffff)
    raw = (b'\0'+b'\x20\x80\xc0\xff'*width)*height if pixels else b''
    return b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('>IIBBBBB',width,height,8,6,0,0,0))+chunk(b'IDAT',zlib.compress(raw))+chunk(b'IEND',b'')

def encoded_image(kind):
    out=io.BytesIO();image=Image.new('RGB',(32,24),(40,100,160))
    if kind=='GIF': image.save(out,format=kind,save_all=True,append_images=[Image.new('RGB',(32,24),(160,60,40))],duration=100,loop=0)
    else: image.save(out,format=kind)
    return out.getvalue()

PAGE = '''<!doctype html><html><head><meta charset="utf-8"><title>固定测试页面 中文 English</title><link rel="stylesheet" href="/style.css"></head><body>
<h1 id="top">固定页面 中文 English</h1><p id="mixed">Before <strong>bold 粗体</strong> after <em>emphasis 斜体</em><br>第二行 &amp; escaped.</p>
<a id="next" href="/next">下一页</a> <a id="fragment" href="#末尾">定位末尾</a>
<ul><li>项目一</li><li>项目二</li></ul><ol><li>First</li><li>Second</li></ol>
<img id="image" src="/good.png" alt="正常图片"><img src="/bad.png" alt="损坏图片">
<table id="wide"><tr>'''+''.join(f'<th>Column {i} 标题</th>' for i in range(12))+'''</tr><tr>'''+''.join(f'<td>Cell {i} value 中文</td>' for i in range(12))+'''</tr></table>
<pre id="code">line one\n'''+('long_code_segment_'*100)+'''\nreachable end</pre>
<form id="form" action="/form" method="get"><input id="query" name="q" value="中文 &amp; English"><input name="hidden" type="hidden" value="stable"><button id="submit">提交 GET</button></form>
<script>ui.invoke('browser.home',{});ui.postMessage({action:'home'});while(true){}</script>
<a href="javascript:ui.invoke('browser.home',{})" onclick="ui.invoke('browser.stop',{})">拒绝脚本地址</a>
<iframe src="file:///C:/Windows/win.ini"></iframe><canvas>canvas content</canvas>
'''+''.join(f'<p>Long text paragraph {i}: 中文阅读，English reading, find-target-{i}.</p>' for i in range(25))+'''<h2 id="末尾">末尾定位</h2></body></html>'''

class Handler(BaseHTTPRequestHandler):
    protocol_version='HTTP/1.1'
    counts={}
    lock=threading.Lock()
    log_path=None
    def log_message(self, *_): pass
    def do_GET(self):
        target=urlsplit(self.path); path=target.path
        with self.lock:
            self.counts[self.path]=self.counts.get(self.path,0)+1
            if self.log_path: self.log_path.write_text(json.dumps(self.counts,ensure_ascii=False),encoding='utf-8')
        status=200; headers={}; mime='text/html; charset=utf-8'; body=b''
        if path=='/stats': body=json.dumps(self.counts).encode(); mime='application/json'
        elif path=='/page': body=PAGE.encode()
        elif path in ('/m2-reading','/docs/m2-reading'):
            text=(Path(__file__).parent/'fixtures/m2-reading.html').read_text(encoding='utf-8')
            text=text.replace('LONG_CODE_PLACEHOLDER','m2_long_code_segment_'*100).replace('LONG_PARAGRAPHS_PLACEHOLDER',''.join(f'<p>M2 reading paragraph {i}: 中文阅读 English stable text m2-find-{i}.</p>' for i in range(25)))
            body=text.encode()
        elif path=='/m2-style.css': mime='text/css; charset=utf-8';body=b'#cascade{background:#ff0000} p{margin-top:8px} .hidden{display:none}'
        elif path=='/m2-next':body='<title>M2 Next</title><h1>下一页</h1><a href="/m2-reading">返回</a>'.encode()
        elif path=='/m2-bom-conflict':mime='text/html; charset=gb18030';body=b'\xef\xbb\xbf'+'<meta charset="utf-16"><title>BOM中文优先</title><p>正文中文</p>'.encode()
        elif path=='/m2-http-conflict':body='<meta charset="gb18030"><title>HTTP中文优先</title><p>正文中文</p>'.encode()
        elif path=='/m2-meta-utf16':mime='text/html';body='<meta charset="utf-16"><title>Meta中文UTF8</title><p>正文中文</p>'.encode()
        elif path=='/m2-unknown':mime='text/html; charset=unknown';body=b'<p>Unsupported declaration</p>'
        elif path=='/m2-odd-utf16':mime='text/html';body=b'\xff\xfeA'
        elif path=='/m2-converted-budget':mime='text/html; charset=gb18030';body=('<p>'+'中'*90000+'</p>').encode('gb18030')
        elif path=='/m2-table-limit':body=b'<table><tr><td colspan="65">Explicit grid limit</table>'
        elif path=='/m2-css-limit':body=('<style>'+''.join(f'.x{i}{{color:#123456}}' for i in range(257))+'</style><p>CSS budget</p>').encode()
        elif path=='/m2-form':
            import html
            from urllib.parse import parse_qsl
            entries=parse_qsl(target.query,keep_blank_values=True)
            expected=[('q','中文 & + English'),('same','first'),('same','second'),('empty',''),('submitter','提交')]
            ok=entries==expected
            body=('<title>M2 GET '+('Passed' if ok else 'Mismatch')+'</title><h1 id="结果">GET结构化结果</h1><pre>'+html.escape(json.dumps(entries,ensure_ascii=False))+'</pre>').encode()
        elif path.startswith('/m2-image/'):
            file=Path(__file__).parent.parent/'.work/m2/images'/path.removeprefix('/m2-image/')
            if file.name!=path.removeprefix('/m2-image/') or not file.is_file():status=404;body=b'fixture missing'
            else:mime='image/jpeg' if file.suffix=='.jpg' else 'image/gif' if file.suffix=='.gif' else 'image/png' if file.suffix=='.png' else 'image/bmp';body=file.read_bytes()
        elif path=='/next': body='<title>Next 第二页</title><h1>Next page</h1><a href="/page">Back</a>'.encode()
        elif path=='/gzip': body=gzip.compress('<title>Gzip 页面</title><p>压缩文本</p>'.encode());headers['Content-Encoding']='gzip'
        elif path=='/invalid-encoding': body=b'<p>\xff\xfe\x80</p>'
        elif path=='/font-limit': body=b'<p style="font-size:10000px">Excessive font</p>'
        elif path=='/disconnect':
            self.close_connection=True
            self.connection.shutdown(socket.SHUT_RDWR);self.connection.close();return
        elif path=='/long-title': body=('<title>'+'长标题 '+('x'*4080)+'</title><p>Long title is reachable through the host title UI.</p>').encode()
        elif path=='/form': body=('<title>GET result</title><h1>GET result</h1><pre>'+str(parse_qs(target.query))+'</pre>').encode()
        elif path=='/redirect': status=302; headers['Location']='/page'
        elif path=='/cross': status=302; headers['Location']='http://localhost:8765/page'
        elif path=='/protocol': status=302; headers['Location']='file:///C:/Windows/win.ini'
        elif path=='/credentials-redirect': status=302;headers['Location']='https://user:SuperSecret@localhost:8766/page'
        elif path=='/redirect-missing': status=302
        elif path=='/redirect-secret': status=302;headers['Location']='/next?token=SuperSecret#private'
        elif path=='/wrong-type': mime='text/plain';body=b'not an HTML document'
        elif path=='/attachment': headers['Content-Disposition']='attachment; filename="example.html"';body=b'<p>attachment</p>'
        elif path=='/empty': body=b'<script>document.body.textContent="SPA"</script>'
        elif path=='/diag-resources': body=b'<title>Partial resource diagnostics</title><h1>Readable main text</h1><link rel="stylesheet" href="/500.css?token=SuperSecret"><link rel="stylesheet" href="http://localhost:8765/style.css?token=SuperSecret"><img src="/bad.png?token=SuperSecret"><img src="/huge.png">'
        elif path=='/500.css': status=500;mime='text/css';body=b'CSS failure'
        elif path=='/policy-attack': body=b'<title>Isolated policy attack</title><p>Readable text</p><script>ui.postMessage({action:"settings-save",value:"https://evil.test"});ui.invoke("browser.settings-reset",{});</script><p onclick="ui.invoke(\'browser.settings-reset\',{})">No application bridge</p>'
        elif path=='/loop': status=302; headers['Location']='/loop'
        elif path=='/404': status=404; body=b'<h1>missing</h1>'
        elif path=='/500': status=500; body=b'<h1>server error</h1>'
        elif path=='/big': body=b'<p>'+b'x'*262144+b'</p>'
        elif path=='/nodes': body=b'<p>x</p>'*1100
        elif path=='/longtext': body=b'<p>'+b'x'*4096+b'</p>'
        elif path=='/deep': body=b'<div>'*70+b'x'+b'</div>'*70
        elif path=='/gbk': mime='text/html; charset=gb18030'; body='<title>编码页面</title><p>中文编码验证</p>'.encode('gb18030')
        elif path=='/utf16': body=b'\xff\xfe'+'<title>UTF16 页面</title><p>中文字符</p>'.encode('utf-16-le')
        elif path=='/style.css': mime='text/css'; body=b'p { color:#123456; } h1 { color:#285c93; }'
        elif path=='/good.png': mime='image/png'; body=png()
        elif path=='/jpeg': mime='image/jpeg'; body=encoded_image('JPEG')
        elif path=='/gif': mime='image/gif'; body=encoded_image('GIF')
        elif path=='/bmp': mime='image/bmp'; body=encoded_image('BMP')
        elif path=='/formats': body=b'<title>Common image formats</title><img src="/good.png"><img src="/jpeg"><img src="/gif"><img src="/bmp">'
        elif path=='/bad.png': mime='image/png'; body=b'broken-image'
        elif path=='/huge.png': mime='image/png'; body=png(6000,6000,False)
        elif path=='/image-limit': body=b'<title>Image limit</title><img src="/huge.png" alt="too large">'
        elif path=='/decode-page': body=b'<title>Decode close</title><img src="/decode.png" alt="large bounded image">'
        elif path=='/decode.png': mime='image/png';body=png(2048,1020)
        elif path=='/resource-limit': body=b'<title>Resource limit</title>'+b''.join(f'<img src="/good.png?n={i}">'.encode() for i in range(42))
        elif path=='/slow':
            body=b'<title>Slow old result</title><p>late</p>';self.send_response(200);self.send_header('Content-Type',mime);self.send_header('Content-Length',str(len(body)));self.end_headers()
            try:self.wfile.write(body[:8]);self.wfile.flush();time.sleep(2);self.wfile.write(body[8:])
            except (BrokenPipeError,ConnectionResetError,ssl.SSLError):pass
            return
        elif path=='/slow-headers': time.sleep(5);body=b'<title>Slow headers</title>'
        else: status=404;body=b'<h1>Fixture not found</h1>'
        self.send_response(status);self.send_header('Content-Type',mime);self.send_header('Content-Length',str(len(body)))
        for key,value in headers.items():self.send_header(key,value)
        self.end_headers()
        try:self.wfile.write(body)
        except (BrokenPipeError,ConnectionResetError,ssl.SSLError):pass

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--log',type=Path);parser.add_argument('--tls',type=Path);args=parser.parse_args();Handler.log_path=args.log
    server=ThreadingHTTPServer(('127.0.0.1',8765),Handler);server.daemon_threads=True
    if args.tls:
        tls=ThreadingHTTPServer(('127.0.0.1',8766),Handler);tls.daemon_threads=True;context=ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        context.load_cert_chain(args.tls/'cert.pem',args.tls/'key.pem');tls.socket=context.wrap_socket(tls.socket,server_side=True);threading.Thread(target=tls.serve_forever,daemon=True).start()
    print('FIXTURE_READY',flush=True)
    try:server.serve_forever()
    except KeyboardInterrupt:pass
    finally:server.server_close()
if __name__=='__main__':main()
