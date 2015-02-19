import sys
import  mbug_dev
from PyQt4 import QtGui, QtCore
from PyQt4.uic import loadUi
from functools import partial

def SwitchSingleChannel(channel,value):
    mbugSender.switch((16,channel),value)

def SwitchAllChannels(value):
    for channel in range(4):
        mbugSender.switch((16,channel+1),value)

mbugDeviceList = mbug_dev.list_devices(2151)
if not mbugDeviceList:
    print('No MBUG-2151 device found.')
    exit()
mbugDevice = mbug_dev.mbug_2151()
mbugSender = mbugDevice.target.IKT201()

app = QtGui.QApplication(sys.argv)
gui = loadUi("SimpleRemoteControl.ui")
gui.Channel1_On.clicked.connect(partial(SwitchSingleChannel,channel=1,value=1))
gui.Channel1_Off.clicked.connect(partial(SwitchSingleChannel,channel=1,value=0))
gui.Channel2_On.clicked.connect(partial(SwitchSingleChannel,channel=2,value=1))
gui.Channel2_Off.clicked.connect(partial(SwitchSingleChannel,channel=2,value=0))
gui.Channel3_On.clicked.connect(partial(SwitchSingleChannel,channel=3,value=1))
gui.Channel3_Off.clicked.connect(partial(SwitchSingleChannel,channel=3,value=0))
gui.Channel4_On.clicked.connect(partial(SwitchSingleChannel,channel=4,value=1))
gui.Channel4_Off.clicked.connect(partial(SwitchSingleChannel,channel=4,value=0))
gui.AllChannels_On.clicked.connect(partial(SwitchAllChannels,value=1))
gui.AllChannels_Off.clicked.connect(partial(SwitchAllChannels,value=0))
gui.show()
sys.exit(app.exec_())
