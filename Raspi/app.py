import sys
from PyQt6.QtWidgets import QApplication, QWidget
from mainwindow import Ui_mainwindow


class MainWindowClass(QWidget):
    def __init__(self):
        super().__init__()

        self.ui = Ui_mainwindow()       
        self.ui.setupUi(self)
        self.set_style()
        self.setup_connection()
        

    def set_style(self):
        self.ui.groupBox_4.setStyleSheet("QGroupBox { border: 1px solid red;}")
    
    def setup_connection(self):
        pass


if __name__ == '__main__':
    app = QApplication(sys.argv)
    main_window = MainWindowClass()
    main_window.show()
    sys.exit(app.exec())