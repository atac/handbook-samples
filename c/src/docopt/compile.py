#!/usr/bin/env python

import glob
import os
import sys


if __name__ == '__main__':
    for filename in glob.glob('docopt/*.docopt'):
        name = os.path.splitext(os.path.basename(filename))[0]
        os.system('{} {}/docopt.c/docopt_c.py {} -o {}_args.c'.format(
            sys.executable, os.path.dirname(__file__), filename, name))
