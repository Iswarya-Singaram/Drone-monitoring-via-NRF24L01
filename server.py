import eventlet
eventlet.monkey_patch()

import serial
import serial.tools.list_ports
from flask import Flask, render_template
from flask_socketio import SocketIO
from datetime import datetime
from collections import deque
import time

app = Flask(__name__)
app.config['SECRET_KEY'] = 'your-secret-key'
socketio = SocketIO(app, cors_allowed_origins="*", async_mode='eventlet')

class TelemetryServer:
    def __init__(self):
        self.serial_port = None
        self.latest_data = {
            'distance': 0.0, 'temperature': 0.0, 'humidity': 0.0,
            'latitude': 0.0, 'longitude': 0.0, 'altitude': 0.0,
            'gps_fix': False, 'heartbeat': False, 'timestamp': datetime.now().isoformat()
        }
        self.gps_history = deque(maxlen=100)
        self.running = True

    def find_arduino_port(self):
        ports = serial.tools.list_ports.comports()
        for port in ports:
            if 'Arduino' in port.description or 'CH340' in port.description:
                return port.device
        return None

    def connect_serial(self):
        port = self.find_arduino_port()
        if not port:
            print("No Arduino found! Check your connection.")
            return False
            
        try:
            self.serial_port = serial.Serial(
                port=port,
                baudrate=9600,
                timeout=1,
                write_timeout=1
            )
            self.serial_port.reset_input_buffer()
            self.serial_port.reset_output_buffer()
            print(f"Connected to {self.serial_port.name} at 9600 baud")
            return True
        except Exception as e:
            print(f"Connection failed: {str(e)}")
            return False

    def read_serial(self):
        buffer = ""
        while self.running:
            try:
                if self.serial_port and self.serial_port.in_waiting:
                    raw_data = self.serial_port.read(self.serial_port.in_waiting)
                    try:
                        buffer += raw_data.decode('ascii')
                    except UnicodeDecodeError:
                        continue
                    
                    while '\n' in buffer:
                        line, buffer = buffer.split('\n', 1)
                        line = line.strip()
                        if line and line.count(',') == 7:
                            self.process_data(line)
                            
            except serial.SerialException as e:
                print(f"Serial error: {str(e)}")
                self.reconnect_serial()
            except Exception as e:
                print(f"Unexpected error: {str(e)}")
            
            time.sleep(0.01)

    def process_data(self, line):
        try:
            # Skip any line that starts with non-numeric characters
            if not line[0].isdigit() and line[0] != '-' and line[0] != '.':
                return
                
            parts = [part.strip() for part in line.split(',')]
            if len(parts) != 8:
                return
                
            new_data = {
                'distance': float(parts[0]),
                'temperature': float(parts[1]),
                'humidity': float(parts[2]),
                'latitude': float(parts[3]),
                'longitude': float(parts[4]),
                'altitude': float(parts[5]),
                'gps_fix': bool(int(float(parts[6]))),
                'heartbeat': bool(int(float(parts[7]))),
                'timestamp': datetime.now().isoformat()
            }
            
            # Validate GPS coordinates
            if not (-90 <= new_data['latitude'] <= 90) or not (-180 <= new_data['longitude'] <= 180):
                new_data.update({
                    'latitude': 0.0,
                    'longitude': 0.0,
                    'gps_fix': False
                })
            
            self.latest_data = new_data
            
            if new_data['gps_fix']:
                self.gps_history.append({
                    'lat': new_data['latitude'],
                    'lng': new_data['longitude'],
                    'alt': new_data['altitude'],
                    'time': new_data['timestamp']
                })
            
            # Emit to all clients
            socketio.emit('telemetry_update', {
                'current': self.latest_data,
                'history': list(self.gps_history)
            })
            
        except ValueError as e:
            print(f"Data format error in line '{line}': {str(e)}")

telemetry = TelemetryServer()

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/data')
def get_data():
    return {
        'current': telemetry.latest_data,
        'history': list(telemetry.gps_history)
    }

if __name__ == '__main__':
    if telemetry.connect_serial():
        socketio.start_background_task(telemetry.read_serial)
    
    socketio.run(app,
                host='0.0.0.0',
                port=5000,
                debug=False,
                use_reloader=False,
                log_output=True)