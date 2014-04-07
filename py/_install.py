#!/usr/bin/env python

import os
os.chdir('mbug_dev')
exec(compile(open('_install.py').read(), '_install.py', 'exec'))
