import sys, json, socket
from PyQt5.QtWidgets import QApplication, QMainWindow
from PyQt5 import uic
from controller import ControllerThreadClass, Controller

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

controller_thread_index = 0
front_camera_thread_index = 1
second_camera_thread_index = 2

LED_front = 0
LED_second = 0

threads = {}

controller = Controller()

class MainWindowClass(QMainWindow):
    def __init__(self):
        super(MainWindowClass, self).__init__()
        uic.loadUi('Controller/mainwindow.ui', self)
        self.set_style()
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
        print(value)

    def sl_second_led_changeValue(self, value):
        print(value)

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
            # self.show_messages(data)
            return
        try:
            threads[controller_thread_index] = ControllerThreadClass(controller, parent=None)
            threads[controller_thread_index].start()
            threads[controller_thread_index].any_signal.connect(self.motion_control)
            # threads[controller_thread_index].message_signal.connect(self.show_messages)
        except Exception as e:
            data = {'status' : False, 'message' : 'Problem to starting thread. ' + str(e), 'data' : ''}
            # self.show_messages(data)
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
    
    def closeEvent(self, event):
        # if controller_thread_index in thread:
        #     thread[controller_thread_index].stop()
        #     thread.pop(controller_thread_index)
        #     controller.gamepad.close()

        event.accept()
    
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
                          '9' : str(LED_front).zfill(1), '10' : str(LED_second).zfill(1)}
            # print(data_send)

            try:
                clientsocket.send(bytes(json.dumps(data_send) + ">", "ascii"))
            except Exception as e:
                print(e)

        except Exception as e:
            self.show_messages({'status' : False, 'message' : str(e), 'data' : ''})


if __name__ == '__main__':
    app = QApplication(sys.argv)
    mainWindow = MainWindowClass()
    mainWindow.show()
    sys.exit(app.exec_())
