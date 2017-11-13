#!/usr/bin/env python
import sys
import numpy as np
from collections import defaultdict
import itertools as it
import matplotlib
matplotlib.use('TKAgg')
import matplotlib.animation
import pylab
import os
import argparse
import errno
import gzip
import json
import copy

from matplotlib.patches import Rectangle, Circle
from util import *
from read import *

DEFAULT_STYLE = {
 "marker" : '+',
 "s" : 1
}

parser = argparse.ArgumentParser(description='Animate point process')
parser.add_argument('--input', '-i', required=True, help='Input point file')
parser.add_argument('--output', '-o', required=True, help='Output file')
parser.add_argument('--dimension', '-U', type=int,  required=True, help='Length of a single dimension of the universe')
parser.add_argument('--tmpdir', required=True, help='Temporary directory for storing files')
parser.add_argument('--delay', default=0.1, help='Temporary directory for storing files')
parser.add_argument('--dpi', type=float, help='DPI of single framge')
parser.add_argument('--size', nargs=2, type=int, help='Size of figure')
parser.add_argument('--skip', type=int, default=0, help='Skip frames')
parser.add_argument('--mint', type=float, default=0, help='Start animation from this time')
parser.add_argument('--maxt', type=float, default=-1, help='Stop animation by this time')
parser.add_argument('--style', '-s', default=None, help="Style JSON file")
parser.add_argument('--use_legend', '-l', default=False, help="Add legend file")
parser.add_argument('--donotremove', default=False, help="Do not remove temp files")
parser.add_argument('--filetype', default='png', help="Filetype")
parser.add_argument('--noanimation', default=0, help="Run imagemagick to make an animation")
parser.add_argument('--nolabel', default=0, help="Do not draw label")
parser.add_argument('--noaxis', default=0, help="Do not draw axis labels")
parser.add_argument('--embedlabel', default=0, help="Embedded time label")

args = parser.parse_args()

new_dir(args.tmpdir)
TMP_IMG_FILE = "{}/tmp-img%09d.{}".format(args.tmpdir,args.filetype)

data = list(read_pp_data(open_file(args.input)))

style_data = {}
if args.style is not None:
    sd = json.load(open(args.style))
    for (key,val) in sd.items():
        style_data[int(key)] = val


sys.stdout.write("Plotting\n")

files = []
to_skip = args.skip
for (i,D) in enumerate(data):
    if to_skip > 0:
        to_skip -= 1
        continue
    time, events, pxs, pys = D
    if time < args.mint:
        continue
    if args.maxt >= 0 and time > args.maxt:
        continue

    if args.size is not None:
        fig = pylab.figure(figsize=args.size)

    ax = pylab.gca()

    ax.set_aspect('equal')
    pylab.xlim((0,args.dimension))
    pylab.ylim((0,args.dimension))

    if not args.nolabel:
        pylab.title("time = {}".format(time))

    if args.noaxis:
        #pylab.axis('off')
        pylab.tick_params(labelbottom='off',labelleft='off') 
        pylab.xticks([])
        pylab.yticks([])

    m = '.'
    for t in pxs.keys():
        if t in style_data.keys():
            style = copy.deepcopy(style_data[t])
        else:
            style = copy.deepcopy(DEFAULT_STYLE)

        if "circle" in style:
            radius, color =style["circle"]

            del style["circle"]
            for loc in zip(pxs[t],pys[t]):
                ax.add_patch(Circle(loc, radius, facecolor=color,edgecolor="none",alpha=0.1,zorder=3))

        if "omit" in style and style["omit"]:
            pass
        else:
            if "omit" in style: del style["omit"]
            sct = pylab.scatter(pxs[t], pys[t], **style) # =STYLES["markers"][t], 

    if args.embedlabel:
        t = pylab.text(1.5, 3, r'$t={:.1f}$'.format(float(time)),zorder=200)
        t.set_bbox(dict(facecolor='white', alpha=0.75, edgecolor='none'))

    if args.use_legend:
        legend = pylab.legend(loc=3)
        legend.get_frame().set_facecolor('white')
        legend.get_frame().set_alpha(0.95)

    fname = TMP_IMG_FILE % i
    pylab.savefig(fname, dpi=args.dpi,bbox_inches='tight', pad_inches=0)
    files.append(fname)
    sys.stdout.write("{}/{} plotted\r".format(i,len(data)))
    sys.stdout.flush()
    to_skip = args.skip
    pylab.close()


if not args.noanimation:
    sys.stdout.write("\nMaking video\n")
    os.system("convert +map -delay {} -loop 0 {} {}".format(args.delay, " ".join(files), args.output))

if not args.donotremove:
    for f in files:
        os.remove(f)
print(args.output)
