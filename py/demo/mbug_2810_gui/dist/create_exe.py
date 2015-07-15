from distutils.core import setup
import py2exe
import sys

sys.argv += ['py2exe']
    
setup(
    options = {'py2exe': {}},
    console = [
        {
            "script": "../mbug_2810_gui.py",
            "icon_resources": [(0, "../icon-3.ico")]
        } ]
)
