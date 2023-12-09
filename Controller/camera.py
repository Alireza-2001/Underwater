from PyQt5 import QtCore
import cv2
import numpy


class CameraThreadClass(QtCore.QThread):
    frame_signal = QtCore.pyqtSignal(numpy.ndarray)
    message_signal = QtCore.pyqtSignal(dict)

    def __init__(self, url):
        super(CameraThreadClass, self).__init__()
        self.url = url
        try:
            self.cap = cv2.VideoCapture(url)
            self.cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1200)
            self.cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 650)
        except Exception as e:
            data = {'status' : False, 'message' : str(e), 'data' : ''}
            self.message_signal.emit(data)           

    def run(self):
        data = {'status' : True, 'message' : 'Starting camera thread...', 'data' : ''}
        self.message_signal.emit(data)
        while (True):
            try:
                ret, frame = self.cap.read()
                if ret:
                    self.frame_signal.emit(frame)

                    if cv2.waitKey(1) & 0xFF == ord('q'):
                        break
            except Exception as e:
                data = {'status' : False, 'message' : str(e), 'data' : ''}
                self.message_signal.emit(data)

        self.cap.release()


    def stop(self):
        self.is_running = False
        data = {'status' : True, 'message' : 'Stopping camera thread...', 'data' : ''}
        self.message_signal.emit(data)
        self.terminate()
