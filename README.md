# IoT Web Dashboard for ESP32 Control

A complete IoT solution featuring a web dashboard for controlling and monitoring ESP32 devices. Perfect for competition demonstrations with mobile-friendly interface.

## Features

### Web Dashboard
- **Real-time Control**: Turn appliances ON/OFF via ESP32 relay
- **Live Monitoring**: View current, voltage, and motion sensor data
- **Action Logging**: Track the last 5 operations with timestamps
- **Mobile-Friendly**: Responsive design optimized for phone demonstrations
- **WebSocket Support**: Real-time updates without page refresh

### ESP32 Integration
- **WiFi Connectivity**: Seamless communication with web dashboard
- **Sensor Reading**: Current, voltage, and motion detection
- **Relay Control**: Appliance switching capability
- **Local Web Server**: Device status accessible via local IP
- **Auto-Reconnection**: Robust WiFi connection handling

## Tech Stack

### Backend
- **Flask**: Web application framework
- **Flask-SocketIO**: WebSocket support for real-time updates
- **SQLAlchemy**: Database management
- **SQLite**: Local database storage

### Frontend
- **HTML5**: Modern web standards
- **Bootstrap 5**: Mobile-responsive UI framework
- **Vanilla JavaScript**: Client-side functionality
- **Socket.IO**: Real-time communication
- **Font Awesome**: Professional icons

### ESP32
- **Arduino IDE**: Development environment
- **WiFi Library**: Network connectivity
- **ArduinoJson**: JSON data handling
- **HTTPClient**: Web communication

## Setup Instructions

### 1. Replit Deployment

1. **Create New Repl**:
   - Go to [Replit](https://replit.com)
   - Create a new Python Flask repl
   - Upload all project files

2. **Install Dependencies**:
   ```bash
   pip install flask flask-sqlalchemy flask-socketio
   ```

3. **Set Environment Variables**:
   - Add `SESSION_SECRET` in Replit's Secrets tab
   - Set a secure random string value

4. **Run the Application**:
   - Click "Run" button
   - The app will start on port 5000
   - Note your Replit app URL (e.g., `https://your-app-name.replit.app`)

### 2. ESP32 Configuration

1. **Hardware Setup**:
   