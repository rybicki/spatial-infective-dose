import sys

fs = (open(fname) for fname in sys.argv[1:])
for i,f in enumerate(fs,start=1):
    first_line = f.readline() # first line is a header; only include it for the first dataset
    if i == 1:
        sys.stdout.write("dataset,")
        sys.stdout.write(first_line)

    for l in f:
        sys.stdout.write("{},".format(i))
        sys.stdout.write(l)
