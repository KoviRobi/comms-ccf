#!/usr/bin/env python

import sys

from PySide6.QtCore import Qt
from PySide6.QtGui import QColor
from PySide6.QtWidgets import (
    QApplication,
    QDockWidget,
    QMainWindow,
    QTableWidget,
    QTableWidgetItem,
)
from qtconsole.manager import QtKernelManager
from qtconsole.rich_jupyter_widget import RichJupyterWidget

colors = [
    ("Red", "#FF0000"),
    ("Green", "#00FF00"),
    ("Blue", "#0000FF"),
    ("Black", "#000000"),
    ("White", "#FFFFFF"),
    ("Electric Green", "#41CD52"),
    ("Dark Blue", "#222840"),
    ("Yellow", "#F9E56d"),
]


def get_rgb_from_hex(code):
    code_hex = code.replace("#", "")
    rgb = tuple(int(code_hex[i : i + 2], 16) for i in (0, 2, 4))
    return QColor.fromRgb(rgb[0], rgb[1], rgb[2])


class MainWindow(QMainWindow):
    def __init__(self) -> None:
        super().__init__()
        self.add_logs()
        self.add_console()

    def add_logs(self):
        table = QTableWidget()
        table.setRowCount(len(colors))
        table.setColumnCount(len(colors[0]) + 1)
        table.setHorizontalHeaderLabels(["Name", "Hex Code", "Color"])

        for i, (name, code) in enumerate(colors):
            item_name = QTableWidgetItem(name)
            item_code = QTableWidgetItem(code)
            item_color = QTableWidgetItem()
            item_color.setBackground(get_rgb_from_hex(code))
            table.setItem(i, 0, item_name)
            table.setItem(i, 1, item_code)
            table.setItem(i, 2, item_color)

        self.setCentralWidget(table)

    def add_console(self):
        dock = QDockWidget("Console", self)

        kernel_manager = QtKernelManager(kernel_name="python3")
        kernel_manager.start_kernel()

        kernel_client = kernel_manager.client()
        kernel_client.start_channels()
        kernel_client.execute("x = 'hello world'", store_history=False)

        jupyter_widget = RichJupyterWidget(width=120)
        jupyter_widget.kernel_manager = kernel_manager
        jupyter_widget.kernel_client = kernel_client
        dock.setWidget(jupyter_widget)
        self.addDockWidget(Qt.DockWidgetArea.BottomDockWidgetArea, dock)
        jupyter_widget._control.setFocus()


if __name__ == "__main__":

    app = QApplication(sys.argv)
    app.setStyleSheet("* { font-size: 18pt; }")

    win = MainWindow()
    win.show()

    sys.exit(app.exec())
