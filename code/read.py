import collections
from util import *

def read_pp_data(fp):
    Entry = collections.namedtuple('Entry', ['time', 'events', 'xs', 'ys'])
    for line in fp:
        line = line.strip()
        tokens = line.split(" ")
        time = tokens[0]
        events = tokens[1]
        remaining = tokens[2:]
        
        pxs = collections.defaultdict(list)
        pys = collections.defaultdict(list)
        for (t, x, y) in grouper(3, remaining):
            tt = int(t)
            pxs[tt].append(float(x))
            pys[tt].append(float(y))

        yield Entry(time, events, pxs, pys)

def by_type(data):
    DataPoint = collections.namedtuple('DataPoint', ['time', 'xs', 'ys'])
    processed = collections.defaultdict(list)
    for entry in data:
        time, events, pxs, pys = entry
        for t in pxs.keys():
            xs = pxs[t]
            ys = pys[t]
            processed[t].append( DataPoint(time, xs, ys) )
    return processed
