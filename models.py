from app import db
from datetime import datetime

class SensorReading(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    current = db.Column(db.Float, nullable=False, default=0.0)
    voltage = db.Column(db.Float, nullable=False, default=0.0)
    motion_detected = db.Column(db.Boolean, nullable=False, default=False)
    timestamp = db.Column(db.DateTime, nullable=False, default=datetime.utcnow)

class DeviceCommand(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    command = db.Column(db.String(50), nullable=False)
    value = db.Column(db.Boolean, nullable=False)
    timestamp = db.Column(db.DateTime, nullable=False, default=datetime.utcnow)
    executed = db.Column(db.Boolean, nullable=False, default=False)
