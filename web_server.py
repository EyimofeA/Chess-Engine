#!/usr/bin/env python3
"""
Simple web server for Chess Engine GUI
Serves the HTML interface and handles engine move requests
"""

from http.server import HTTPServer, SimpleHTTPRequestHandler
import json
import subprocess
import time
import os

class ChessEngineHandler(SimpleHTTPRequestHandler):
    def do_GET(self):
        """Serve the HTML GUI"""
        if self.path == '/' or self.path == '/index.html':
            self.path = '/web_gui.html'
        return SimpleHTTPRequestHandler.do_GET(self)

    def do_POST(self):
        """Handle engine move requests"""
        if self.path == '/engine_move':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            data = json.loads(post_data.decode('utf-8'))

            fen = data.get('fen')
            depth = data.get('depth', 5)

            # Call engine
            try:
                start_time = time.time()
                result = subprocess.run(
                    ['./engine', fen, str(depth)],
                    capture_output=True,
                    text=True,
                    timeout=30
                )
                elapsed = time.time() - start_time

                move = result.stdout.strip()

                # Parse nodes if available (optional enhancement)
                nodes = 10000  # Placeholder

                response = {
                    'move': move,
                    'nodes': nodes,
                    'time': elapsed,
                    'depth': depth
                }

                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(json.dumps(response).encode())

            except subprocess.TimeoutExpired:
                self.send_error(500, "Engine timeout")
            except Exception as e:
                self.send_error(500, f"Engine error: {str(e)}")
        else:
            self.send_error(404)

def run_server(port=8000):
    # Change to script directory
    os.chdir(os.path.dirname(os.path.abspath(__file__)))

    server_address = ('', port)
    httpd = HTTPServer(server_address, ChessEngineHandler)

    print(f"=" * 60)
    print(f"Chess Engine Web GUI Server")
    print(f"=" * 60)
    print(f"Server running on http://localhost:{port}")
    print(f"Open your browser and navigate to: http://localhost:{port}")
    print(f"Press Ctrl+C to stop")
    print(f"=" * 60)

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n\nShutting down server...")
        httpd.server_close()

if __name__ == '__main__':
    import sys
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8000
    run_server(port)
