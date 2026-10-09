import argparse
import json
import pathlib
import subprocess
import sys
import urllib.error
import urllib.request


class NoRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, req, fp, code, msg, headers, newurl):
        return None


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('method', choices=['GET', 'POST'])
    parser.add_argument('route')
    parser.add_argument('output')
    parser.add_argument('--body')
    parser.add_argument('--download', action='store_true')
    parser.add_argument('--curl', action='store_true')
    args = parser.parse_args()
    output = pathlib.Path(args.output)
    if output.exists():
        raise RuntimeError('Refusing to overwrite API evidence')
    if not (args.route == '/user' or args.route.startswith('/repos/wbycloud/ui-framework/')):
        raise RuntimeError('Repository scope violation')
    credential = subprocess.run(['git', 'credential', 'fill'],
        input='protocol=https\nhost=github.com\npath=wbycloud/ui-framework.git\n\n',
        text=True, capture_output=True, check=True)
    fields = dict(line.split('=', 1) for line in credential.stdout.splitlines() if '=' in line)
    token = fields.get('password')
    if not token:
        raise RuntimeError('Git credential manager supplied no token')
    headers = {'Authorization': 'Bearer ' + token,
               'Accept': 'application/vnd.github+json',
               'X-GitHub-Api-Version': '2022-11-28',
               'User-Agent': 'ui-framework-p1-diagnostic'}
    body = pathlib.Path(args.body).read_bytes() if args.body else None
    if body is not None:
        headers['Content-Type'] = 'application/json'
    if args.curl:
        if args.method != 'GET' or body is not None:
            raise RuntimeError('curl transport is read-only retrieval')
        def quote(value):
            if '\n' in value or '\r' in value:
                raise RuntimeError('Invalid curl configuration value')
            return '"' + value.replace('\\', '\\\\').replace('"', '\\"') + '"'
        initial_output = output.with_suffix(output.suffix + '.redirect-body') if args.download else output
        response_headers = output.with_suffix(output.suffix + '.response-headers')
        config = '\n'.join(['url = ' + quote('https://api.github.com' + args.route),
            'output = ' + quote(str(initial_output.resolve())), 'write-out = "%{http_code}"',
            'max-time = 60', 'connect-timeout = 15', 'retry = 2', 'retry-delay = 2',
            *(['dump-header = ' + quote(str(response_headers.resolve()))] if args.download else []),
            *['header = ' + quote(name + ': ' + value) for name, value in headers.items()]])
        # Authentication enters curl through stdin, never its command line or a file.
        result = subprocess.run(['curl.exe', '--silent', '--show-error', '--config', '-'],
            input=config.encode('utf-8'), capture_output=True)
        status = result.stdout.decode('ascii', 'replace').strip()
        if args.download and result.returncode == 0 and status in ('301', '302', '303', '307', '308'):
            locations = [line.split(':', 1)[1].strip() for line in
                response_headers.read_text(encoding='ascii').splitlines() if line.lower().startswith('location:')]
            if not locations or not locations[-1].startswith('https://'):
                raise RuntimeError('Invalid download redirect')
            config = '\n'.join(['url = ' + quote(locations[-1]), 'output = ' + quote(str(output.resolve())),
                'write-out = "%{http_code}"', 'max-time = 300', 'connect-timeout = 15', 'retry = 2', 'retry-delay = 2'])
            result = subprocess.run(['curl.exe', '--silent', '--show-error', '--config', '-'],
                input=config.encode('utf-8'), capture_output=True)
            status = result.stdout.decode('ascii', 'replace').strip()
        if result.returncode or status != '200':
            print(json.dumps({'curl_exit': result.returncode, 'http_status': status,
                              'error': result.stderr.decode('utf-8', 'replace').strip()}))
            return 1
        print(json.dumps({'http_status': 200, 'transport': 'curl', 'bytes': output.stat().st_size,
                          'evidence': str(output)}))
        return 0
    request = urllib.request.Request('https://api.github.com' + args.route,
        data=body, headers=headers, method=args.method)
    opener = urllib.request.build_opener(NoRedirect)
    try:
        response = opener.open(request, timeout=60)
    except urllib.error.HTTPError as error:
        if args.download and error.code in (301, 302, 303, 307, 308):
            location = error.headers.get('Location', '')
            if not location.startswith('https://'):
                raise RuntimeError('Invalid artifact redirect')
            # Signed storage URL receives no GitHub Authorization header.
            response = urllib.request.urlopen(location, timeout=60)
        else:
            detail = error.read().decode('utf-8', 'replace')
            output.write_text(json.dumps({'http_status': error.code, 'error': detail}), encoding='utf-8')
            print(json.dumps({'http_status': error.code, 'evidence': str(output)}))
            return 1
    with response, output.open('xb') as stream:
        while chunk := response.read(1024 * 1024):
            stream.write(chunk)
    if output.stat().st_size == 0:
        output.write_text(json.dumps({'http_status': response.status}), encoding='utf-8')
    print(json.dumps({'http_status': response.status, 'bytes': output.stat().st_size, 'evidence': str(output)}))
    return 0


if __name__ == '__main__':
    sys.exit(main())
