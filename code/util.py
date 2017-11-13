import os
import time
import errno
import subprocess
import gzip

try:
    # Python 3
    from itertools import zip_longest
except ImportError:
    # Python 2
    from itertools import izip_longest as zip_longest

def distance(p,q):
    s = sum(((i-j)**2 for i,j in zip(p,q)))
    return s**0.5

def torus_distance(p,q,U):
    ss = []
    for i,j in zip(p,q):
        d = abs(i-j)
        s = min(d, U-d)
        ss.append(s**2)
    return sum(ss)**0.5

def new_dir(path):
    try:
        os.makedirs(path)
    except OSError as exc:  # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise

def grouper(n, iterable, fillvalue=None):
    "grouper(3, 'ABCDEFG', 'x') --> ABC DEF Gxx"
    args = [iter(iterable)] * n
    return zip_longest(fillvalue=fillvalue, *args)

start_time = time.time()

def log(s):
    print("{:7.1f} -- {}".format(time.time()-start_time, s))

def run(cmd, logfile=None):
    p = subprocess.Popen(cmd, shell=True, universal_newlines=True, stdout=logfile)
    ret_code = p.wait()
    if logfile is not None:
        logfile.flush()
    return ret_code

def open_file(filename):
    """ Open a file for reading. Detect if gzipped and return GzipFile object if yes """
    f = open(filename, 'r')
    if (f.read(2) == '\x1f\x8b'):
        # Check for GZIP magic number
        f.seek(0)
        return gzip.GzipFile(fileobj=f)
    else:
        f.seek(0)
        return f
