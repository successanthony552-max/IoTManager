import os
import logging
from flask import Flask, render_template, request, jsonify
from flask_socketio import SocketIO, emit
from flask_sqlalchemy import SQLAlchemy
from sqlalchemy.orm import DeclarativeBase
from werkzeug.middleware.proxy_fix import ProxyFix
from datetime import datetime
import threading
import time

# Configure logging
logging.basicConfig(level=logging.DEBUG)

class Base(DeclarativeBase):
    pass

db = SQLAlchemy(model_class=Base)

# Create the app
app = Flask(__name__)
app.secret_key = os.environ.get("SESSION_SECRET", "dev-secret-key-change-in-production")
app.wsgi_app = ProxyFix(app.wsgi_app, x_proto=1, x_host=1)

# Configure the database
app.config["SQLALCHEMY_DATABASE_URI"] = os.environ.get("DATABASE_URL", "sqlite:///iot_dashboard.db")
app.config["SQLALCHEMY_ENGINE_OPTIONS"] = {
    "pool_recycle": 300,
    "pool_pre_ping": True,
}

# Initialize extensions
db.init_app(app)
socketio = SocketIO(app, cors_allowed_origins="*")

# Global variables to store latest sensor data and device state
latest_sensor_data = {
    'current': 0.0,
    'voltage': 0.0,
    'motion_detected': False,
    'appliance_status': False,
    'timestamp': datetime.now().isoformat()
}

action_log = []

def add_to_action_log(action, details):
    """Add an action to the log and keep only the last 5 entries"""
    global action_log
    log_entry = {
        'timestamp': datetime.now().strftime('%H:%M:%S'),
        'action': action,
        'details': details
    }
    action_log.insert(0, log_entry)
    action_log = action_log[:5]  # Keep only last 5 entries
    
    # Emit to all connected clients
    socketio.emit('action_logged', log_entry)

# Import routes after app initialization
from routes import *

with app.app_context():
    from models import *
    db.create_all()

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000, debug=True)
