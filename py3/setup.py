from distutils.core import setup
from distutils.dir_util import remove_tree
import sys

setup(name='mbug_dev',
    version='1.2.0',
    author='Stephan Messlinger',
    packages=['mbug_dev'],
    )

remove_tree('build')
if int(sys.version[0]) >= 3:
    print('### Warning: python3 support may still be buggy.')
