# Secure IoT Sensor Node - Demo Applications

Complete set of demo applications for visualization and demonstration of Secure IoT Sensor Node functionality.

## Structure

```
demo/
├── web_visualization/     # Web dashboard with visualization
│   ├── index.html          # Main page
│   ├── app.js              # JavaScript logic
│   ├── api_demo.js         # API client with integration
│   ├── server.py           # Simple HTTP server
│   ├── package.json        # Project metadata
│   └── README.md           # Web dashboard documentation
├── api_server.py          # Flask API server
└── requirements.txt        # Python dependencies
```

## Features

### Web Visualization (`web_visualization/`)
- **Real-time**: Sensor monitoring with 2-second updates
- **Interactive charts**: Temperature, CPU, memory with Chart.js
- **Device control**: LED, reboot, calibration
- **Security status**: Encryption, authentication, updates
- **Responsive design**: Works on all devices
- **English interface**: Full localization

### API Server (`api_server.py`)
- **REST API**: Complete set of endpoints for interaction
- **Device emulation**: Realistic sensor data
- **Data history**: Storage of last 100 measurements
- **Event log**: Automatic event generation
- **Statistics**: Min/max/average values
- **Background tasks**: Real device operation simulation

## Quick Start

### Option 1: Simple web panel (no server)

```bash
cd demo/web_visualization
python server.py
# Open http://localhost:8080
```

### Option 2: Full API + web panel

```bash
# Install dependencies
pip install -r requirements.txt

# Start API server
python api_server.py
# API available at http://localhost:5000

# In another terminal - start web panel
cd demo/web_visualization
python server.py
# Open http://localhost:8080 with API integration
```

### Option 3: API server only

```bash
python api_server.py
```

Available endpoints:
- `GET  /api/status` - Device status
- `GET  /api/sensors` - Current sensor data
- `GET  /api/sensors/history` - Data history
- `GET  /api/events` - Event log
- `POST /api/command` - Device control
- `GET  /api/health` - Health check
- `GET  /api/stats` - Statistics

## Control

### Web Interface
- **Control buttons**: LED, reboot, calibration
- **Hot keys**: Space to pause simulation
- **Interactive charts**: Hover for details

### API Commands
```bash
# Turn on LED
curl -X POST http://localhost:5000/api/command \
  -H "Content-Type: application/json" \
  -d '{"command": "led_on"}'

# Reboot
curl -X POST http://localhost:5000/api/command \
  -H "Content-Type: application/json" \
  -d '{"command": "reboot"}'
```

## Demo Scenarios

### 1. Basic Monitoring
- Temperature observation with daily cycles
- CPU load and memory monitoring
- Battery level tracking

### 2. Device Control
- LED indicator on/off
- Device reboot
- Sensor calibration

### 3. Data Analysis
- Measurement history viewing
- Period statistics
- Anomaly detection

### 4. Security
- Encryption status monitoring
- Authentication check
- Update history

## Technical Details

### Web Panel
- **Frontend**: HTML5, CSS3, JavaScript ES6+
- **UI**: Tailwind CSS + Lucide Icons
- **Charts**: Chart.js with animations
- **Responsive**: Mobile-first design

### API Server
- **Backend**: Flask + Flask-CORS
- **Data**: JSON REST API
- **Simulation**: Python threading
- **Storage**: In-memory (Redis for production)

### Data Emulation
- **Temperature**: Daily sinusoidal fluctuations
- **CPU**: Periodic load peaks
- **Events**: Random generation with realistic frequency
- **Battery**: Gradual discharge

## Screenshots

Demo includes:
- 📈 Interactive sensor charts
- 🎛️ Device control panel
- 📋 System event log
- 🔒 Security status
- 🌐 Network information

## Development

### Local Development
```bash
# Start with hot-reload
pip install flask-cors
python api_server.py
```

### API Testing
```bash
# Health check
curl http://localhost:5000/api/health

# Get sensor data
curl http://localhost:5000/api/sensors

# Send command
curl -X POST http://localhost:5000/api/command \
  -H "Content-Type: application/json" \
  -d '{"command": "led_on"}'
```

### Customization
- Change sensor ranges in `generate_sensor_data()`
- Add new commands in `process_command()`
- Extend event types in `background_events()`
- Configure update frequency in JavaScript

## Production

### Docker Deployment
```dockerfile
FROM python:3.9-slim
WORKDIR /app
COPY requirements.txt .
RUN pip install -r requirements.txt
COPY . .
EXPOSE 5000
CMD ["python", "api_server.py"]
```

### Nginx Configuration
```nginx
server {
    listen 80;
    location /api/ {
        proxy_pass http://localhost:5000;
    }
    location / {
        root /var/www/html;
        try_files $uri $uri/ /index.html;
    }
}
```

## License

MIT License - free use and modification

## Contributing

1. Fork project
2. Create feature branch
3. Make changes
4. Submit Pull Request

----

🎉 **Ready for demonstration!** Launch any option and enjoy full visualization of Secure IoT Sensor Node operation.
