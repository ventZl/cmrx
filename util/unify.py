#!/usr/bin/python3

import os
import sys

ofile = open(sys.argv[1], mode = "w", encoding = "utf-8")
for source_file in sys.argv[2:]:
    ofile.write("#include \"%s\"\n" % (source_file))

ofile.close()
