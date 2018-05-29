#!/usr/bin/env python3

import argparse
import os
from math import sqrt
from PIL import Image
import numpy as np

parser = argparse.ArgumentParser(
    description='converts gray scale raw image to png image',
    formatter_class=argparse.ArgumentDefaultsHelpFormatter
)
parser.add_argument('infile', nargs='+', help='input filename')
parser.add_argument('--width', type=int)
parser.add_argument('--height', type=int)

args = parser.parse_args()

for file in args.infile:
    with open(file, 'rb') as fd:
        raw_data = fd.read()

    if args.width is not None:
        width = args.width
        height = args.height
    else:
        height = width = int(sqrt(len(raw_data)))
    if height * width == len(raw_data):
        img = Image.frombytes('L', (width, height), raw_data)
    else:
        height = width = int(sqrt(len(raw_data) / 3))
        assert(height * width * 3 == len(raw_data))
        raw_data = np.frombuffer(raw_data, dtype='ubyte')
        raw_data = np.reshape(raw_data, (3, width, height))
        img = Image.fromarray(np.transpose(raw_data, (1, 2, 0)), 'RGB')

    filename, ext = os.path.splitext(file)
    img.save(filename + '.png')
