# IoT Web Dashboard for ESP32 Control

## Overview

This is a real-time IoT web dashboard application designed for controlling and monitoring ESP32 devices. The system enables remote control of appliances through ESP32 relays and provides live monitoring of sensor data including current, voltage, and motion detection. The application is built with Flask backend, SQLite database, and WebSocket communication for real-time updates.

## User Preferences

Preferred communication style: Simple, everyday language.

## System Architecture

### Backend Architecture
- **Flask Framework**: Core web application framework providing REST API endpoints
- **Flask-SocketIO**: WebSocket implementation for real-time bidirectional communication
- **SQLAlchemy ORM**: Database abstraction layer with declarative base model
- **SQLite Database**: Local file-based database for persistent storage
- **ProxyFix Middleware**: Handles proxy headers for proper deployment

### Frontend Architecture
- **Server-Side Rendering**: HTML templates rendered by Flask using Jinja2
- **Bootstrap 5**: Responsive CSS framework with dark theme support
- **Vanilla JavaScript**: Client-side logic without heavy frameworks
- **Socket.IO Client**: Real-time WebSocket communication with backend
- **Font Awesome**: Icon library for UI elements

### Database Schema
- **SensorReading Model**: Stores sensor data (current, voltage, motion) with timestamps
- **DeviceCommand Model**: Tracks device commands and execution status
- **SQLite Storage**: File-based database suitable for small-scale deployments

## Key Components

### Core Application Files
- **app.py**: Main Flask application setup with database and SocketIO initialization
- **main.py**: Application entry point for running the server
- **models.py**: SQLAlchemy database models for sensor readings and device commands
- **routes.py**: API endpoints for ESP32 communication and data handling

### Frontend Components
- **templates/index.html**: Main dashboard interface with appliance control and sensor monitoring
- **static/css/custom.css**: Custom styling for status indicators and animations
- **static/js/dashboard.js**: Client-side JavaScript for real-time updates and user interactions

### Global State Management
- **latest_sensor_data**: In-memory storage for current sensor readings
- **action_log**: Circular buffer maintaining last 5 operations with timestamps

## Data Flow

### ESP32 to Dashboard Flow
1. ESP32 sends sensor data via HTTP POST to `/api/sensor-data`
2. Backend updates global state and saves to database
3. WebSocket broadcasts updates to all connected clients
4. Frontend receives real-time updates and refreshes UI

### Dashboard to ESP32 Flow
1. User clicks control buttons in web interface
2. JavaScript sends WebSocket command to backend
3. Backend logs action and broadcasts status update
4. ESP32 can poll for commands or receive via separate endpoint

### Real-Time Communication
- WebSocket connection maintains persistent bidirectional communication
- Automatic reconnection handling for network interruptions
- Status updates broadcast to all connected clients simultaneously

## External Dependencies

### Python Packages
- **Flask**: Web framework and routing
- **Flask-SocketIO**: WebSocket support
- **Flask-SQLAlchemy**: Database ORM
- **Werkzeug**: WSGI utilities and middleware

### Frontend Libraries
- **Bootstrap 5 CDN**: UI framework with dark theme
- **Font Awesome CDN**: Icon library
- **Socket.IO Client**: WebSocket client library

### ESP32 Integration
- **HTTP Client**: For sending sensor data to dashboard
- **WiFi Library**: Network connectivity management
- **ArduinoJson**: JSON serialization for data exchange

## Deployment Strategy

### Replit Deployment
- **Flask Development Server**: Built-in server suitable for demos and development
- **Environment Variables**: Support for configuration via environment variables
- **File-Based Database**: SQLite database stored locally in Replit environment
- **Port Configuration**: Configured to run on port 5000 with host binding

### Production Considerations
- Database can be upgraded to PostgreSQL for production use
- Session secrets should be configured via environment variables
- Proxy middleware configured for reverse proxy deployments
- CORS enabled for cross-origin WebSocket connections

### Mobile Optimization
- Responsive Bootstrap design for mobile demonstrations
- Touch-friendly interface elements
- Optimized for competition presentations and demos