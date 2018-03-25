#!/usr/bin/env python3

import argparse
import os
from math import sqrt
from PIL import Image

parser = argparse.ArgumentParser(
    description='converts gray scale raw image to bmp image',
    formatter_class=argparse.ArgumentDefaultsHelpFormatter
)
parser.add_argument('infile', metavar='infile', help='input filename')

args = parser.parse_args()

with open(args.infile, 'rb') as fd:
    raw_data = fd.read()

height = width = int(sqrt(len(raw_data)))
assert(height * width == len(raw_data))

img = Image.frombytes('L', (height, width), raw_data)

filename, ext = os.path.splitext(args.infile)
img.save(filename + '.bmp')
