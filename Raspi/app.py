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

raspi_IP = ''
data_port = 8000
laptop_IP = ''

def socket_config():
    global raspi_IP
    global data_port
    try:
        s.connect((raspi_IP, data_port))
    except Exception as e:
        print(e)


def main():
    pass

if __name__ == '__main__':
    main()