# Secure IoT Sensor Node - Web Visualization

Interactive web dashboard for demonstrating Secure IoT Sensor Node functionality with real-time data visualization.

## Features

### Real-time Monitoring
- **Temperature**: Daily fluctuation charts with current values
- **Heart Rate**: Cardiac rhythm monitoring
- **CPU**: Processor and memory usage
- **Battery**: Charge level and runtime

### Data Visualization
- Interactive temperature charts (24h)
- System performance charts (CPU, memory)
- Animated status indicators
- Real-time event log

### Device Control
- LED indicator on/off toggle
- Device reboot
- Test command sending
- Various state simulation

### Security Status
- Encryption status monitoring
- Authentication information
- Firmware update history
- System version

### Network Information
- IP and MAC addresses
- WiFi signal strength
- Uptime
- Connection status

## Technologies

- **Frontend**: HTML5, CSS3, JavaScript (ES6+)
- **UI Framework**: Tailwind CSS
- **Charts**: Chart.js
- **Icons**: Lucide Icons
- **Animation**: CSS animations + JavaScript

## Responsiveness

Panel is fully responsive and works on:
- Desktops (1920x1080+)
- Tablets (768x1024)
- Mobile devices (375x667+)

## Control

### Hot Keys
- **Space**: Start/stop simulation

### Interactive Elements
- Device control buttons
- Simulation toggle
- Automatic data update (every 2 seconds)

## Data and Simulation

### Generated Data
- **Temperature**: 15-35°C with daily fluctuations
- **CPU**: 30-70% with variable load
- **Memory**: 40-70% usage
- **Heart Rate**: 65-85 bpm
- **Battery**: Gradual discharge 95-20%

### System Events
- Automatic events (5% update probability)
- User command responses
- Operation status

## Launch

1. Open `index.html` in web browser
2. Panel will automatically start simulation
3. Use control buttons for interaction

## Screenshots

Panel includes:
- Sensor metric cards
- Interactive charts
- Control panel
- Event log
- Security and network status

## Configuration

### Changing Simulation Parameters
In file `app.js` you can configure:
- Update frequency (`setTimeout(startSimulation, 2000)`)
- Sensor value ranges
- Event probability (`Math.random() < 0.05`)
- Chart point count (`maxPoints = 24`)

### Customization
- Change color scheme in Tailwind CSS
- Add new event types
- Extend control command set
- Integrate with real API

## Demo Scenarios

1. **Basic Monitoring**: Real-time sensor observation
2. **Device Control**: Send commands and observe responses
3. **Data Analysis**: Study charts and trends
4. **Events**: Monitor system event log
5. **Security**: Check protection mechanism status

## Notes

- This is a demo application simulating IoT device operation
- All data is randomly generated
- Interface optimized for capability demonstration
- Can be used as template for real projects
