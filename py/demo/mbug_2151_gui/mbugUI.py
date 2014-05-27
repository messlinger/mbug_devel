import sys
from mbug_base import *
from mbug_2151 import *
from PyQt4 import QtGui, QtCore
from PyQt4.uic import loadUi
from functools import partial

def SwitchSingleChannel(iChannel,iValue):
    global iSystemCode
    mbugSender.switch((iSystemCode,iChannel),iValue)
 #   mbugSender.switch(iChannel,iValue)

def SwitchAllChannels(iValue):
    global iSystemCode
    for iChannel in range(4):
        mbugSender.switch((iSystemCode,iChannel+1),iValue)

def SetTarget(stNewTarget):
    global mbugSender
    mbugSender = eval('mbugDevice.target.%s()'%stNewTarget)

def SetNewTarget():
    stNewTarget=str(gui.comboBox_CurrentTarget.currentText())
    #gui.debug.setText(stNewTarget)
    SetTarget(stNewTarget)

def SetNewSystemCode():
    global iSystemCode
    iSystemCode=int(gui.comboBox_SystemCode.currentText())

iSystemCode=0
stDefaultTarget='IKT201'
mbugDevice = mbug_2151(list_devices(2151)[0])
SetTarget(stDefaultTarget)
app = QtGui.QApplication(sys.argv)
gui = loadUi("..\\Qt\\SimpleRemoteControl.ui")
gui.comboBox_CurrentTarget.addItems([stEntry for stEntry in dir(mbugDevice.target) if not stEntry.startswith('_')])
gui.comboBox_CurrentTarget.setCurrentIndex(dir(mbugDevice.target).index(stDefaultTarget))
gui.comboBox_SystemCode.addItems([str(stEntry) for stEntry in range(16)])
gui.comboBox_SystemCode.setCurrentIndex(iSystemCode)

gui.Channel1_On.clicked.connect(partial(SwitchSingleChannel,iChannel=1,iValue=1))
gui.Channel1_Off.clicked.connect(partial(SwitchSingleChannel,iChannel=1,iValue=0))
gui.Channel2_On.clicked.connect(partial(SwitchSingleChannel,iChannel=2,iValue=1))
gui.Channel2_Off.clicked.connect(partial(SwitchSingleChannel,iChannel=2,iValue=0))
gui.Channel3_On.clicked.connect(partial(SwitchSingleChannel,iChannel=3,iValue=1))
gui.Channel3_Off.clicked.connect(partial(SwitchSingleChannel,iChannel=3,iValue=0))
gui.Channel4_On.clicked.connect(partial(SwitchSingleChannel,iChannel=4,iValue=1))
gui.Channel4_Off.clicked.connect(partial(SwitchSingleChannel,iChannel=4,iValue=0))
gui.AllChannels_On.clicked.connect(partial(SwitchAllChannels,iValue=1))
gui.AllChannels_Off.clicked.connect(partial(SwitchAllChannels,iValue=0))
gui.comboBox_CurrentTarget.currentIndexChanged.connect(SetNewTarget)
gui.comboBox_SystemCode.currentIndexChanged.connect(SetNewSystemCode)

gui.show()
sys.exit(app.exec_())
