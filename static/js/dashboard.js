// Initialize Socket.IO connection
const socket = io();

// DOM elements
const connectionStatus = document.getElementById('connectionStatus');
const applianceIndicator = document.getElementById('applianceIndicator');
const applianceStatusText = document.getElementById('applianceStatusText');
const turnOnBtn = document.getElementById('turnOnBtn');
const turnOffBtn = document.getElementById('turnOffBtn');
const currentValue = document.getElementById('currentValue');
const voltageValue = document.getElementById('voltageValue');
const currentTimestamp = document.getElementById('currentTimestamp');
const voltageTimestamp = document.getElementById('voltageTimestamp');
const motionIndicator = document.getElementById('motionIndicator');
const motionStatusText = document.getElementById('motionStatusText');
const actionLog = document.getElementById('actionLog');
const loadingOverlay = document.getElementById('loadingOverlay');

// Connection status handlers
socket.on('connect', function() {
    console.log('Connected to server');
    updateConnectionStatus(true);
    
    // Request current status
    socket.emit('request_status_update');
});

socket.on('disconnect', function() {
    console.log('Disconnected from server');
    updateConnectionStatus(false);
});

// Real-time data handlers
socket.on('sensor_update', function(data) {
    console.log('Sensor update received:', data);
    updateSensorData(data);
});

socket.on('appliance_status_update', function(data) {
    console.log('Appliance status update:', data);
    updateApplianceStatus(data.appliance_status);
});

socket.on('action_logged', function(logEntry) {
    console.log('New action logged:', logEntry);
    addActionToLog(logEntry);
});

socket.on('action_log_update', function(logs) {
    console.log('Action log update:', logs);
    updateActionLog(logs);
});

// Control functions
async function controlAppliance(action) {
    try {
        showLoading(true);
        
        const response = await fetch('/api/control', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({ action: action })
        });
        
        const result = await response.json();
        
        if (response.ok) {
            console.log('Control command successful:', result);
            updateApplianceStatus(result.appliance_status);
            showNotification('success', result.message);
        } else {
            console.error('Control command failed:', result);
            showNotification('error', result.error || 'Command failed');
        }
    } catch (error) {
        console.error('Error sending control command:', error);
        showNotification('error', 'Failed to send command');
    } finally {
        showLoading(false);
    }
}

// UI update functions
function updateConnectionStatus(connected) {
    if (connected) {
        connectionStatus.innerHTML = '<i class="fas fa-wifi me-1"></i>Connected';
        connectionStatus.className = 'badge bg-success';
    } else {
        connectionStatus.innerHTML = '<i class="fas fa-wifi-slash me-1"></i>Disconnected';
        connectionStatus.className = 'badge bg-danger';
    }
}

function updateSensorData(data) {
    // Update current
    currentValue.textContent = parseFloat(data.current || 0).toFixed(2);
    
    // Update voltage
    voltageValue.textContent = parseFloat(data.voltage || 0).toFixed(2);
    
    // Update timestamps
    const timestamp = new Date(data.timestamp).toLocaleTimeString();
    currentTimestamp.textContent = timestamp;
    voltageTimestamp.textContent = timestamp;
    
    // Update motion detection
    updateMotionStatus(data.motion_detected);
    
    // Update appliance status if provided
    if (data.hasOwnProperty('appliance_status')) {
        updateApplianceStatus(data.appliance_status);
    }
}

function updateApplianceStatus(isOn) {
    const indicator = applianceIndicator;
    const statusText = applianceStatusText;
    
    if (isOn) {
        indicator.className = 'status-indicator status-on';
        indicator.innerHTML = '<i class="fas fa-power-off fa-3x text-success"></i>';
        statusText.textContent = 'ON';
        statusText.className = 'mt-2 mb-0 fw-bold text-success';
        
        turnOnBtn.disabled = true;
        turnOffBtn.disabled = false;
    } else {
        indicator.className = 'status-indicator status-off';
        indicator.innerHTML = '<i class="fas fa-power-off fa-3x text-muted"></i>';
        statusText.textContent = 'OFF';
        statusText.className = 'mt-2 mb-0 fw-bold text-muted';
        
        turnOnBtn.disabled = false;
        turnOffBtn.disabled = true;
    }
}

function updateMotionStatus(motionDetected) {
    const indicator = motionIndicator;
    const statusText = motionStatusText;
    
    if (motionDetected) {
        indicator.className = 'motion-indicator motion-detected';
        indicator.innerHTML = '<i class="fas fa-eye fa-2x text-warning"></i>';
        statusText.textContent = 'Motion Detected!';
        statusText.className = 'mt-2 mb-0 text-warning fw-bold';
    } else {
        indicator.className = 'motion-indicator motion-clear';
        indicator.innerHTML = '<i class="fas fa-eye-slash fa-2x text-muted"></i>';
        statusText.textContent = 'No Motion Detected';
        statusText.className = 'mt-2 mb-0 text-muted';
    }
}

function updateActionLog(logs) {
    const logContainer = actionLog;
    
    if (!logs || logs.length === 0) {
        logContainer.innerHTML = `
            <div class="text-muted text-center py-3">
                <i class="fas fa-clock me-2"></i>
                No actions recorded yet
            </div>
        `;
        return;
    }
    
    logContainer.innerHTML = '';
    
    logs.forEach(log => {
        const logEntry = createLogEntry(log);
        logContainer.appendChild(logEntry);
    });
}

function addActionToLog(logEntry) {
    const logContainer = actionLog;
    
    // Remove "no actions" message if present
    const noActionsMsg = logContainer.querySelector('.text-muted');
    if (noActionsMsg) {
        logContainer.innerHTML = '';
    }
    
    // Create and prepend new log entry
    const newEntry = createLogEntry(logEntry);
    logContainer.insertBefore(newEntry, logContainer.firstChild);
    
    // Keep only the last 5 entries
    const entries = logContainer.children;
    while (entries.length > 5) {
        logContainer.removeChild(entries[entries.length - 1]);
    }
}

function createLogEntry(log) {
    const logEntry = document.createElement('div');
    logEntry.className = 'log-entry d-flex justify-content-between align-items-center py-2 border-bottom';
    
    logEntry.innerHTML = `
        <div>
            <span class="badge bg-secondary me-2">${log.action}</span>
            <span>${log.details}</span>
        </div>
        <small class="text-muted">${log.timestamp}</small>
    `;
    
    return logEntry;
}

function showLoading(show) {
    loadingOverlay.style.display = show ? 'flex' : 'none';
}

function showNotification(type, message) {
    // Create notification element
    const notification = document.createElement('div');
    notification.className = `alert alert-${type === 'success' ? 'success' : 'danger'} alert-dismissible fade show position-fixed`;
    notification.style.cssText = 'top: 20px; right: 20px; z-index: 9999; min-width: 300px;';
    
    notification.innerHTML = `
        <i class="fas fa-${type === 'success' ? 'check-circle' : 'exclamation-triangle'} me-2"></i>
        ${message}
        <button type="button" class="btn-close" data-bs-dismiss="alert"></button>
    `;
    
    document.body.appendChild(notification);
    
    // Auto-remove after 5 seconds
    setTimeout(() => {
        if (notification.parentNode) {
            notification.parentNode.removeChild(notification);
        }
    }, 5000);
}

// Initialize dashboard on page load
document.addEventListener('DOMContentLoaded', function() {
    console.log('Dashboard initialized');
    
    // Load initial status via HTTP
    fetch('/api/status')
        .then(response => response.json())
        .then(data => {
            console.log('Initial status loaded:', data);
            updateSensorData(data.sensor_data);
            updateActionLog(data.action_log);
        })
        .catch(error => {
            console.error('Error loading initial status:', error);
        });
});

// Periodic status refresh (fallback for WebSocket)
setInterval(() => {
    if (!socket.connected) {
        fetch('/api/status')
            .then(response => response.json())
            .then(data => {
                updateSensorData(data.sensor_data);
                updateActionLog(data.action_log);
            })
            .catch(error => {
                console.error('Error refreshing status:', error);
            });
    }
}, 5000);
