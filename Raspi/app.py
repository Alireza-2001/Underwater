import socket, serial, cv2
from flask import Flask, render_template, Response, jsonify
from threading import Thread, main_thread



app=Flask(__name__)

try:
    camera_1 = cv2.VideoCapture(0)
    camera_2 = cv2.VideoCapture(1)
    ser = serial.Serial("/dev/ttyS0", baudrate = 230400, timeout=1)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
except Exception as e:
    print(e)

raspi_IP = '192.168.0.101'
video_Port = 4000
laptop_IP = '192.168.0.100'
data_port = 8000

top_motor = 1500
bottom_motor = 1500
right_motor = 1500
left_motor = 1500
front_motor = 1500
back_motor = 1500

roll = 0
pitch = 0
yaw = 0
jyro_state = 0

lat = 0.0
lon = 0.0
satellite = 0
speed = 0
distance = 0.0
angle = 0.0
gear = 0
battery_voltage = 0.0


def socket_config():
    try:
        s.connect((laptop_IP, data_port))
    except Exception as e:
        print(e)

def main():
    print("ready")
    socket_config()
    print("Connected")

    while True:
        try:
            data = s.recv(120).decode("ascii")
            if len(data) == 114:
                # ser.write(bytes(data, "ascii"))
                print(data)
        except Exception as e:
            print(e)

def serial_recived_data():
    global top_motor, bottom_motor, right_motor, left_motor, front_motor, back_motor
    global roll, pitch, yaw, jyro_state, lat, lon, satellite, speed, distance, angle, gear, battery_voltage
    while True:
        try:
            data = ser.readline()
            if data:
                data = data.decode("utf-8").split(",")
                if len(data) == 18:
                    top_motor = data[0]
                    bottom_motor = data[1]
                    right_motor = data[2]
                    left_motor = data[3]
                    front_motor = data[4]
                    back_motor = data[5]

                    roll = data[6]
                    pitch = data[7]
                    yaw = data[8]
                    jyro_state = data[9]

                    lat = data[10]
                    lon = data[11]
                    satellite = data[12]
                    speed = data[13]
                    distance = data[14]
                    angle = data[15]

                    gear = data[16]
                    battery_voltage = data[17]

                    # print(data)
        except Exception as e:
            print(e)

def gen_frames_1():  
    while True:
        success, frame = camera_1.read()  # read the camera frame
        if not success:
            break

        ret, buffer = cv2.imencode('.jpg', frame)
        frame = buffer.tobytes()
        yield (b'--frame\r\n'
                b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

def gen_frames_2():  
    while True:
        success, frame = camera_2.read()  # read the camera frame
        if not success:
            break

        ret, buffer = cv2.imencode('.jpg', frame)
        frame = buffer.tobytes()
        yield (b'--frame\r\n'
                b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

@app.route("/fanoos/v1.0/data", methods=["GET"])
def get_books():
    data = {
        'motor' : {
            'top' : top_motor,
            'bottom' : bottom_motor,
            'right' : right_motor,
            'left' : left_motor,
            'front' : front_motor,
            'back' : back_motor
        },
        'jyro' : {
            'roll' : roll,
            'pitch' : pitch,
            'yaw' : yaw,
            'state' : jyro_state
        },
        'gps' : {
            'lat' : lat,
            'lon' : lon,
            'satellite' : satellite,
            'speed' : speed,
            'dis' : distance,
            'angle' : angle
        },
        'gear' : gear,
        'voltage' : battery_voltage
    }
    return jsonify({"data": data})

@app.route('/1')
def index_1():
    return render_template('index_1.html')

@app.route('/2')
def index_2():
    return render_template('index_2.html')

@app.route('/video_feed_1')
def video_feed_1():
    return Response(gen_frames_1(), mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/video_feed_2')
def video_feed_2():
    return Response(gen_frames_2(), mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    main_thread = Thread(target=main)
    serial_recived_thread = Thread(target = serial_recived_data)

    main_thread.start()
    serial_recived_thread.start()
    
    app.run(host=raspi_IP, port=video_Port)