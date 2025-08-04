from flask import Flask, request, jsonify

app = Flask(__name__)

# Store latest sensor data and command
latest_data = {
    "voltage": 0.0,
    "current": 0.0,
    "motion": False,
    "relay_status": "OFF"
}


@app.route('/')
def home():
    return "Smart Energy API is Running!"


# Endpoint: ESP32 sends sensor readings here
@app.route('/update', methods=['POST'])
def update_data():
    global latest_data
    data = request.json
    if not data:
        return jsonify({"error": "No JSON data received"}), 400

    latest_data["voltage"] = data.get("voltage", latest_data["voltage"])
    latest_data["current"] = data.get("current", latest_data["current"])
    latest_data["motion"] = data.get("motion", latest_data["motion"])
    return jsonify({
        "message": "Data updated successfully",
        "latest": latest_data
    })


# Endpoint: Dashboard fetches latest readings
@app.route('/data', methods=['GET'])
def get_data():
    return jsonify(latest_data)


# Endpoint: Dashboard sends ON/OFF command
@app.route('/control', methods=['POST'])
def control_relay():
    global latest_data
    command = request.json.get("relay")
    if command in ["ON", "OFF"]:
        latest_data["relay_status"] = command
        return jsonify({"message": f"Relay set to {command}"})
    return jsonify({"error": "Invalid command"}), 400


# Endpoint: ESP32 checks for command
@app.route('/command', methods=['GET'])
def get_command():
    return jsonify({"relay": latest_data["relay_status"]})


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
