import sys, json, socket, requests
from PyQt5.QtWidgets import QApplication, QMainWindow
from PyQt5 import uic, QtCore
from PyQt5.QtGui import QImage, QPixmap, QIcon
from controller import ControllerThreadClass, Controller
from camera import CameraThreadClass
from time import sleep


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

controller_thread_index = 0
front_camera_thread_index = 1
second_camera_thread_index = 2
requests_thread_index = 3

LED_front = 0
LED_second = 0

data_url = 'http://192.168.0.115:4000/fanoos/v1.0/data'
camera_1_url = 'http://192.168.0.115:4000//video_feed_1'
threads = {}

controller = Controller()


class RequestsThreadClass(QtCore.QThread):
    data_signal = QtCore.pyqtSignal(dict)
    message_signal = QtCore.pyqtSignal(dict)

    def __init__(self):
        super(RequestsThreadClass, self).__init__()
        self.is_running = True
    
    def run(self):
        data = {'status' : True, 'message' : 'Starting Requests thread...', 'data' : ''}
        self.message_signal.emit(data)
        try:
            while (True):
                data = requests.get(data_url)
                self.data_signal.emit(data.json())
                sleep(0.2)

        except Exception as e:
            data = {'status' : False, 'message' : str(e), 'data' : ''}
            self.message_signal.emit(data)
            return

    def stop(self):
        self.is_running = False
        data = {'status' : True, 'message' : 'Stopping Requests thread...', 'data' : ''}
        self.message_signal.emit(data)
        self.terminate()

class MainWindowClass(QMainWindow):
    def __init__(self):
        super(MainWindowClass, self).__init__()
        uic.loadUi('Controller/mainwindow.ui', self)
        # self.set_style()
        self.setWindowIcon("p_app.png")
        self.setup_connection()

    def set_style(self):
        self.gp1.setStyleSheet("QGroupBox { border: 1px solid blue;}")
        self.gp2.setStyleSheet("QGroupBox { border: 1px solid blue;}")
        self.gp3.setStyleSheet("QGroupBox { border: 1px solid blue;}")
        self.gp4.setStyleSheet("QGroupBox { border: 1px solid blue;}")
        self.gp5.setStyleSheet("QGroupBox { border: 1px solid blue;}")
        self.gp6.setStyleSheet("QGroupBox { border: 1px solid blue;}")

    def setup_connection(self):
        #  Buttons click
        self.btn_controller_connect.clicked.connect(self.controller_connect)
        self.btn_controller_disconnect.clicked.connect(self.controller_disconnect)
        self.btn_socket_connect.clicked.connect(self.socket_connect)
        self.chb_front_camera.toggled.connect(self.chb_front_camera_toggled)
        self.chb_second_camera.toggled.connect(self.chb_second_camera_toggled)
        self.sl_front_led.valueChanged[int].connect(self.sl_front_led_changeValue)
        self.sl_second_led.valueChanged[int].connect(self.sl_second_led_changeValue)
    
    def sl_front_led_changeValue(self, value):
        global LED_front
        LED_front = value

    def sl_second_led_changeValue(self, value):
        global LED_second
        LED_second = value

    def chb_front_camera_toggled(self):
        cbutton = self.sender()
        print(str(cbutton.isChecked()))

    def chb_second_camera_toggled(self):
        cbutton = self.sender()
        print(str(cbutton.isChecked()))

    def controller_connect(self):
        devices = controller.get_connected_device()
        try:
            for item in controller.get_connected_device():
                if 'Joystick' in item:
                    controller_device = item
            
            tmp = devices[controller_device]
        except Exception as e:
            data = {'status' : False, 'message' : e, 'data' : ''}
            self.show_messages(data)
            return

        vendor_id = tmp['vendor_id']
        product_id = tmp['product_id']
        try:
            controller.connect(vendor_id=vendor_id, product_id=product_id)
            data = {'status' : True, 'message' : 'Connected.', 'data' : ''}
            self.show_messages(data)
        except Exception as e:
            data = {'status' : False, 'message' : 'Not connected. ' + str(e), 'data' : ''}
            self.show_messages(data)
            return

        if controller_thread_index in threads:
            data = {'status' : False, 'message' : 'Controll is connected.', 'data' : ''}
            self.show_messages(data)
            return
        try:
            threads[controller_thread_index] = ControllerThreadClass(controller, parent=None)
            threads[controller_thread_index].start()
            threads[controller_thread_index].any_signal.connect(self.motion_control)
            threads[controller_thread_index].message_signal.connect(self.show_messages)
        except Exception as e:
            data = {'status' : False, 'message' : 'Problem to starting thread. ' + str(e), 'data' : ''}
            self.show_messages(data)
            return

    def controller_disconnect(self):
        if controller_thread_index in threads:
            try:
                threads[controller_thread_index].stop()
                controller.gamepad.close()
                data = {'status' : True, 'message' : 'Disconnected.', 'data' : ''}
                self.show_messages(data)
                threads.pop(controller_thread_index)
            except Exception as e:
                data = {'status' : False, 'message' : str(e), 'data' : ''}
                self.show_messages(data)
        else:
            data = {'status' : False, 'message' : 'Controller not open.', 'data' : ''}
            self.show_messages(data)

    def socket_connect(self):
        global address, clientsocket, s
        ip = ""
        port = 8000
        host = (ip, port)
        try:
            s.bind(host)
            s.listen()
            clientsocket, address = s.accept()
            data = {'status' : True, 'message' : f'connected to ip : {address[0]} and Port : {address[1]}', 'data' : ''}
            self.show_messages(data)

        except Exception as e:
            data = {'status' : False, 'message' : str(e), 'data' : ''}
            self.show_messages(data)
        
        if requests_thread_index in threads:
            data = {'status' : False, 'message' : 'Controll is connected.', 'data' : ''}
            self.show_messages(data)

        try:
            threads[requests_thread_index] = RequestsThreadClass()
            threads[requests_thread_index].start()
            threads[requests_thread_index].data_signal.connect(self.show_data)
            threads[requests_thread_index].message_signal.connect(self.show_messages)
        except Exception as e:
            data = {'status' : False, 'message' : 'Problem to starting thread. ' + str(e), 'data' : ''}
            self.show_messages(data)

        if front_camera_thread_index in threads:
            data = {'status' : False, 'message' : 'camera thread is active.', 'data' : ''}
            self.show_messages(data)

        try:
            threads[front_camera_thread_index] = CameraThreadClass(url=camera_1_url)
            threads[front_camera_thread_index].start()
            threads[front_camera_thread_index].frame_signal.connect(self.display_main_video)
            threads[front_camera_thread_index].message_signal.connect(self.show_messages)
        except Exception as e:
            data = {'status' : False, 'message' : 'Problem to starting thread. ' + str(e), 'data' : ''}
            self.show_messages(data)
    
    def closeEvent(self, event):
        if controller_thread_index in threads:
            threads[controller_thread_index].stop()
            threads.pop(controller_thread_index)
            controller.gamepad.close()
        
        if front_camera_thread_index in threads:
            threads[front_camera_thread_index].stop()
            threads.pop(front_camera_thread_index)
        
        if second_camera_thread_index in threads:
            threads[second_camera_thread_index].stop()
            threads.pop(second_camera_thread_index)
        
        if requests_thread_index in threads:
            threads[requests_thread_index].stop()
            threads.pop(requests_thread_index)

        event.accept()
    
    def show_data(self, data):
        try:
            data = data['data']
            self.lbl_right_motor.setText(str(data['motor']['right']))
            self.lbl_left_motor.setText(str(data['motor']['left']))
            self.lbl_top_motor.setText(str(data['motor']['top']))
            self.lbl_bottom_motor.setText(str(data['motor']['bottom']))
            self.lbl_back_motor.setText(str(data['motor']['back']))
            self.lbl_front_motor.setText(str(data['motor']['front']))

            self.lbl_roll.setText(str(data['jyro']['roll']))
            self.lbl_pitch.setText(str(data['jyro']['pitch']))
            self.lbl_yaw.setText(str(data['jyro']['yaw']))
            if str(data['jyro']['state']) == "30":
                self.lbl_jyro_state.setText("Enable")
            elif str(data['jyro']['state']) == "31":
                self.lbl_jyro_state.setText("Disable")
            self.lbl_lat.setText(str(float(data['gps']['lat']) / 10000000))
            self.lbl_long.setText(str(float(data['gps']['lon']) / 10000000))
            self.lbl_satallite.setText(str(data['gps']['satellite']))
            self.lbl_speed.setText(str(data['gps']['speed']))
            self.lbl_distance.setText(str(data['gps']['dis']))
            self.lbl_angle.setText(str(data['gps']['angle']))

            self.lbl_dande.setText(str(data['gear']))
            self.lbl_voltage.setText(str(data['voltage']))
            
        except Exception as e:
            self.show_messages({'status' : False, 'message' : str(e), 'data' : ''})
                        
    def show_messages(self, data:dict):
        status = data['status']
        message = str(data['message'])

        if status:
            self.txt_show_status.setStyleSheet("background : lightyellow; color : green")
            self.txt_show_status.setText(message)
        else:
            self.txt_show_status.setStyleSheet("background : lightyellow; color : red")
            self.txt_show_status.setText(message)

    def motion_control(self, data):
        global LED_front, LED_second
        try:
            # print(data)

            left_joy_1 = data[0]
            left_joy_2 = data[1]
            right_joy_1 = data[3]
            right_joy_2 = data[4]

            dandeh_key = 0
            arrow_key = 0
            start_select_key = 0
            L_and_R = 0
            
            match data[5]:
                case 31:
                    dandeh_key = 1
                case 47:
                    dandeh_key = 2
                case 79:
                    dandeh_key = 3
                case 143:
                    dandeh_key = 4
                case 0:
                    arrow_key = 1
                case 2:
                    arrow_key = 2
                case 4:
                    arrow_key = 3
                case 6:
                    arrow_key = 4
                case _:
                    dandeh_key = 0
                    arrow_key = 0
                
            match data[6]:
                case 32:
                    start_select_key = 1
                case 16:
                    start_select_key = 2
                case 2:
                    L_and_R = 1
                case 8:
                    L_and_R = 2
                case 1:
                    L_and_R = 3
                case 4:
                    L_and_R = 4
                case 12:
                    L_and_R = 5
                case 3:
                    L_and_R = 6
                case 64:
                    start_select_key = 3
                case 128:
                    start_select_key = 4
                

            data_send = {'1' : str(left_joy_1).zfill(3), '2' : str(left_joy_2).zfill(3),
                          '3' : str(right_joy_1).zfill(3), '4' : str(right_joy_2).zfill(3),
                          '5' : str(dandeh_key).zfill(1), '6' : str(arrow_key).zfill(1),
                          '7' : str(start_select_key).zfill(1), '8' : str(L_and_R).zfill(1),
                          '9' : str(LED_front).zfill(3), '10' : str(LED_second).zfill(3)}


            # print(data_send)

            clientsocket.send(bytes(json.dumps(data_send ) + ">", "ascii"))

        except Exception as e:
            self.show_messages({'status' : False, 'message' : str(e), 'data' : ''})
    
    def display_main_video(self, img):
        qformat = QImage.Format_Indexed8

        if len(img.shape) == 3:
            if (img.shape[2]) == 4:
                qformat = QImage.Format_RGBA888
            else:
                qformat = QImage.Format_RGB888
        
        img = QImage(img, img.shape[1], img.shape[0], qformat)
        img = img.rgbSwapped()

        self.lbl_video_1.setPixmap(QPixmap.fromImage(img))
    
if __name__ == '__main__':
    app = QApplication(sys.argv)
    mainWindow = MainWindowClass()
    mainWindow.show()
    sys.exit(app.exec_())
