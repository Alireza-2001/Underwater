from PyQt5 import QtCore
import hid


class Controller():  # ps4 controller
    def __init__(self):
        self.stopThread = False
        self.gamepad = hid.device()

    def get_connected_device(self):
        devices = {}
        for device in hid.enumerate():
            devices.update({device['product_string']: {'vendor_id': device['vendor_id'],
                                                       'product_id': device['product_id']}})

        return devices

    def connect(self, vendor_id=0x79, product_id=0x06):
        self.gamepad.open(vendor_id, product_id)
        self.gamepad.set_nonblocking(True)



class ControllerThreadClass(QtCore.QThread):
	
    any_signal = QtCore.pyqtSignal(list)
    message_signal = QtCore.pyqtSignal(dict)
    def __init__(self,controller, parent=None):
        super(ControllerThreadClass, self).__init__(parent)
        self.is_running = True
        self.controller = controller
                
    def run(self):
        data = {'status' : True, 'message' : 'Starting controller thread...', 'data' : ''}
        self.message_signal.emit(data)
        try:
            while (True):
                self.report = self.controller.gamepad.read(64)
                if self.report:
                    self.any_signal.emit(self.report)

        except Exception as e:
            data = {'status' : False, 'message' : str(e), 'data' : ''}
            self.message_signal.emit(data)
            return

    def stop(self):
        self.is_running = False
        data = {'status' : True, 'message' : 'Stopping controller thread...', 'data' : ''}
        self.message_signal.emit(data)
        self.terminate()
