import sys
import socket, json, datetime, os, serial, cv2
from flask import Flask, render_template, Response
from time import time, sleep
from PyQt6 import QtCore
from PyQt6.QtWidgets import QApplication, QWidget
from mainwindow import Ui_mainwindow


try:
    camera = cv2.VideoCapture(0)
    ser = serial.Serial("/dev/ttyS0", baudrate = 115200, timeout=1)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
except Exception as e:
     print(e)


rsspi_ip = ""
laptop_ip = ""


class ControllerThreadClass(QtCore.QThread):	
    data_signal = QtCore.pyqtSignal(list)
    def __init__(self,controller, parent=None):
        super(ControllerThreadClass, self).__init__(parent)
        self.is_running = True
                
    def run(self):
        while True:
            data = ser.readline()
            if data:
                data = data.decode("utf-8").split(",")
                if len(data) == 13:

                    print(data)
                    self.data_signal.emit(data)

    def stop(self):
        self.is_running = False
        data = {'status' : True, 'message' : 'Stopping controller thread...', 'data' : ''}
        self.message_signal.emit(data)
        self.terminate()


class MainWindowClass(QWidget):
    def __init__(self):
        super().__init__()

        self.ui = Ui_mainwindow()
        self.ui.setupUi(self)
        self.set_style()
        self.setup_connection()
        

    def set_style(self):
        self.ui.gp1.setStyleSheet("QGroupBox { border: 1px solid blue;}")
        self.ui.gp2.setStyleSheet("QGroupBox { border: 1px solid blue;}")
        self.ui.gp3.setStyleSheet("QGroupBox { border: 1px solid blue;}")
        self.ui.gp4.setStyleSheet("QGroupBox { border: 1px solid blue;}")

    
    def setup_connection(self):
        self.ui.btn_connect.clicked.connect(self.socket_connect)

    def socket_connect(self):
        print(self.ui.txt_raspi_ip.text())

if __name__ == '__main__':
    app = QApplication(sys.argv)
    main_window = MainWindowClass()
    main_window.show()
    sys.exit(app.exec())