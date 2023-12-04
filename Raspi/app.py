import socket, json, datetime, os, serial, cv2
from flask import Flask, render_template, Response
from time import time, sleep
from threading import Thread, main_thread

app=Flask(__name__)

try:
    camera_1 = cv2.VideoCapture(0)
    camera_2 = cv2.VideoCapture(1)
    ser = serial.Serial("/dev/ttyS0", baudrate = 115200, timeout=1)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
except Exception as e:
    print(e)

raspi_IP = '192.168.0.115'
video_Port = 4000
laptop_IP = '192.168.0.102'
data_port = 8000

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
            if len(data) <= 109:
                # ser.write(bytes(data, "ascii"))
                print(data)
        except Exception as e:
            print(e)

def serial_recived_data():
    while True:
        data = ser.readline()
        if data:
            data = data.decode("utf-8").split(",")
            if len(data) == 13:

                print(data)

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
    
    # app.run(host=raspi_IP, port=video_Port)