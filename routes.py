from flask import render_template, request, jsonify
from flask_socketio import emit
from app import app, socketio, db, latest_sensor_data, action_log, add_to_action_log
from models import SensorReading, DeviceCommand
from datetime import datetime
import logging

@app.route('/')
def index():
    """Main dashboard page"""
    return render_template('index.html')

@app.route('/api/sensor-data', methods=['POST'])
def receive_sensor_data():
    """Endpoint for ESP32 to send sensor data"""
    try:
        data = request.get_json()
        
        if not data:
            return jsonify({'error': 'No data provided'}), 400
        
        # Update global sensor data
        global latest_sensor_data
        latest_sensor_data.update({
            'current': float(data.get('current', 0)),
            'voltage': float(data.get('voltage', 0)),
            'motion_detected': bool(data.get('motion_detected', False)),
            'timestamp': datetime.now().isoformat()
        })
        
        # Save to database
        reading = SensorReading(
            current=latest_sensor_data['current'],
            voltage=latest_sensor_data['voltage'],
            motion_detected=latest_sensor_data['motion_detected']
        )
        db.session.add(reading)
        db.session.commit()
        
        # Emit real-time update to all connected clients
        socketio.emit('sensor_update', latest_sensor_data)
        
        logging.info(f"Sensor data received: {latest_sensor_data}")
        
        return jsonify({'status': 'success', 'message': 'Data received'}), 200
        
    except Exception as e:
        logging.error(f"Error processing sensor data: {str(e)}")
        return jsonify({'error': 'Internal server error'}), 500

@app.route('/api/control', methods=['POST'])
def control_appliance():
    """Endpoint to control appliance via ESP32"""
    try:
        data = request.get_json()
        
        if not data or 'action' not in data:
            return jsonify({'error': 'Action not specified'}), 400
        
        action = data['action']
        appliance_on = action == 'turn_on'
        
        # Update global state
        global latest_sensor_data
        latest_sensor_data['appliance_status'] = appliance_on
        
        # Save command to database
        command = DeviceCommand(
            command='appliance_control',
            value=appliance_on
        )
        db.session.add(command)
        db.session.commit()
        
        # Add to action log
        action_text = "Appliance turned ON" if appliance_on else "Appliance turned OFF"
        add_to_action_log("Control", action_text)
        
        # Emit real-time update
        socketio.emit('appliance_status_update', {
            'appliance_status': appliance_on,
            'timestamp': datetime.now().isoformat()
        })
        
        logging.info(f"Appliance control: {action_text}")
        
        return jsonify({
            'status': 'success',
            'appliance_status': appliance_on,
            'message': action_text
        }), 200
        
    except Exception as e:
        logging.error(f"Error controlling appliance: {str(e)}")
        return jsonify({'error': 'Internal server error'}), 500

@app.route('/api/status', methods=['GET'])
def get_status():
    """Get current device status and sensor data"""
    return jsonify({
        'sensor_data': latest_sensor_data,
        'action_log': action_log
    })

@app.route('/api/esp32/commands', methods=['GET'])
def get_esp32_commands():
    """Endpoint for ESP32 to poll for pending commands"""
    try:
        # Get unexecuted commands
        commands = DeviceCommand.query.filter_by(executed=False).all()
        
        command_list = []
        for cmd in commands:
            command_list.append({
                'id': cmd.id,
                'command': cmd.command,
                'value': cmd.value,
                'timestamp': cmd.timestamp.isoformat()
            })
            
            # Mark as executed
            cmd.executed = True
        
        db.session.commit()
        
        return jsonify({'commands': command_list}), 200
        
    except Exception as e:
        logging.error(f"Error getting ESP32 commands: {str(e)}")
        return jsonify({'error': 'Internal server error'}), 500

@socketio.on('connect')
def handle_connect():
    """Handle client connection"""
    logging.info('Client connected')
    
    # Send current status to newly connected client
    emit('sensor_update', latest_sensor_data)
    emit('action_log_update', action_log)

@socketio.on('disconnect')
def handle_disconnect():
    """Handle client disconnection"""
    logging.info('Client disconnected')

@socketio.on('request_status_update')
def handle_status_request():
    """Handle request for current status"""
    emit('sensor_update', latest_sensor_data)
    emit('action_log_update', action_log)
