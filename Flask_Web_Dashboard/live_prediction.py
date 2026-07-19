from flask import Flask, render_template, jsonify
from flask import send_file
from datetime import datetime
import pytz
import serial
import pickle
import pandas as pd
import threading
import serial.tools.list_ports
import sqlite3



ports = list(serial.tools.list_ports.comports())
for p in ports:
    print(p)

app = Flask(__name__)

# Load ML model
model = pickle.load(open("soldier_model.pkl", "rb"))
le = pickle.load(open("label_encoder.pkl", "rb"))

# Serial connection
ser = serial.Serial('COM4', 115200)

# Global storage for latest data
latest_data = {
    "bpm": 0,
    "spo2": 0,
    "temp": 0,
    "hum": 0,
    "ax": 0,
    "ay": 0,
    "az": 0,
    "lat": 0,
    "lon": 0,
    "status": "Loading..."
}

# Background thread function
def read_serial():
    global latest_data
    ist = pytz.timezone('Asia/Kolkata')


    while True:
        try:
            line = ser.readline().decode(errors='ignore').strip()
            print("RAW DATA:", line)

            if line.startswith("-"):
                continue

            if "Packet ID" in line:
                continue

            if "RSSI" in line:
                continue

            if "SNR" in line:
                continue

            if "Received" in line:
                continue

            if "Lost" in line:
                continue

            if line.startswith("BPM"):
                continue

            data = line.split(',')

            # New LoRa packet format:
            # PacketID,TxTime,BPM,SpO2,Temp,Humidity,AX,AY,AZ,Lat,Lon

            if len(data) != 11:
                continue

            packet_id = int(data[0])
            tx_time = int(data[1])

            bpm = float(data[2])
            spo2 = float(data[3])
            temp = float(data[4])
            hum = float(data[5])
            ax = float(data[6])
            ay = float(data[7])
            az = float(data[8])
            lat = float(data[9])
            lon = float(data[10])

            # Generate current timestamp for this packet
            timestamp = datetime.now(ist).isoformat()

            # Save reading into SQLite database
            conn = sqlite3.connect("soldier_health.db")

            cursor = conn.cursor()

            cursor.execute("""
            INSERT INTO readings
            (timestamp, bpm, spo2, temperature, humidity, ax, ay, az, latitude, longitude)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            """, (
                timestamp, 
                bpm,
                spo2,
                temp,
                hum,
                int(ax),
                int(ay),
                int(az),
                lat,
                lon
            ))

            conn.commit()
            conn.close()

            if (bpm == 0 or spo2 == 0 or temp == 0 or hum == 0 or
                ax == 0 or ay == 0 or az == 0 or lat == 0 or lon == 0):

                status = "Data_Inconsistency"

            else:
                sample = pd.DataFrame([[bpm, spo2, temp, hum, ax, ay, az]],
                                      columns=['BPM','SpO2','Temp','Humidity',
                                               'Motion(AX)','Motion(AY)','Motion(AZ)'])

                prediction = model.predict(sample)
                status = le.inverse_transform(prediction)[0]

            latest_data = {
                "packet_id": packet_id,
                "tx_time": tx_time,
                "bpm": bpm,
                "spo2": spo2,
                "temp": temp,
                "hum": hum,
                "ax": ax,
                "ay": ay,
                "az": az,
                "lat": lat,
                "lon": lon,
                "status": status
            }


        except Exception as e:
            print("Serial Error:", e)
            continue


# Routes
@app.route('/')
def index():
    return render_template("index.html")


@app.route('/data')
def data():
    return jsonify(latest_data)

@app.route('/history')
def history():

    conn = sqlite3.connect("soldier_health.db")
    cursor = conn.cursor()

    cursor.execute("""
    SELECT timestamp,
           bpm,
           spo2,
           temperature,
           humidity,
           ax,
           ay,
           az,
           latitude,
           longitude
    FROM readings
    ORDER BY id DESC
    LIMIT 10
    """)

    rows = cursor.fetchall()

    conn.close()

    return jsonify(rows)

@app.route('/download_csv')
def download_csv():

    import subprocess

    # Generate latest CSV
    subprocess.run(["python", "export_csv.py"])

    return send_file(
        "soldier_health_log.csv",
        as_attachment=True
    )


# Start thread before Flask runs
threading.Thread(target=read_serial, daemon=True).start()

if __name__ == "__main__":
    app.run(debug=False, use_reloader=False)