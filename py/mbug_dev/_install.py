#----------------------------------------------------------
"""Copy python modules from the current directory to the python library directory.
files: list of files to copy.
subdir: subdirectory to copy to."""

subdir = 'mbug_dev'
files = [
    '__init__.py',
    'libusb0.py',
    'mbug_base.py',
    'mbug_2110.py',
    'mbug_2151.py',
    'mbug_2151_target.py',
    'mbug_2165.py',
    'mbug_2810.py',
    'mbug_2811.py',
    'mbug_2818.py',
    'mbug_2820.py',
    ]
#----------------------------------------------------------

import sys
from os import sep, mkdir, chmod, stat
from os.path import isdir, isfile
from shutil import copy

def error(str):
    print('####', str)
    input()
    exit(-1)

if sys.version_info[0] > 2:
    print('### Note: Python 3 support is still experimental.')
print('Python version: ', sys.version)
 
libpath = [p for p in sys.path if p.endswith('site-packages')] # Preferred location
if not libpath:
    libpath = [p for p in sys.path if p.endswith('dist-packages')]
if not libpath:
    error('Error resolving library path')
else:
    libdir = libpath[0]           

destdir = libdir
if 'subdir' in dir() and subdir!='':
    destdir += sep + subdir        
    if not isfile(destdir+sep+'__init__.py'):
        files += ['__init__.py']
        
if not isdir(destdir):
    print('Creating directory:', destdir)
    mkdir(destdir)
    if not isdir(destdir):
        error('Error creating directory')

for f in files:
    destf = destdir + sep + f
    print('Copying file:', f, 'to', destf)
    copy(f, destf)
    if not isfile(destf):
        error('Error copying file')
    chmod(destf, 0o644)
    if not stat(destf).st_mode&0o444:
        error('Error setting file permissions')

if len(sys.argv)>1 and not '-q' in sys.argv[1]:
    print('Done. Press key to quit.')
    input()
