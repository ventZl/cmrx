import sys
from pprint import pprint
import re
import math

f = open(sys.argv[1])

lines = f.readlines()

alloc = { 'data' : {}, 'bss' : {}, 'shared' : {}}
lib = None
typa = None

def set_lib_alloc(_lib, kind):
    global lib
    global typa
    lib = _lib
    if lib not in alloc[kind]:
        alloc[kind][lib] = Section()

    typa = kind
    print("== %s %s:" % (lib, typa.upper()))

class Section:
    def __init__(self):
        self.size = 0
        self.fill = 0

    def add_alloc(self, size):
        if self.fill > 0:
            self.size = self.size + self.fill
            self.fill = 0

        self.size = self.size + size

    def add_fill(self, fill):
        if self.fill > 0:
            self.size = self.size + self.fill
        
        self.fill = fill

    def get_size(self):
        return self.size

    def __str__(self):
        return "0x%04X allocated" % self.get_size()

    def __desc__(self):
        return self.__str__()

for l in lines:
    if  l[0] != ' ':
        lib = None
        typa = None
    elif l[0] == ' ' and l[1] != ' ' and l[1] != '.':
        z1 = re.match("^ lib([a-zA-Z0-0]+)\.a\(\.data", l)
        z2 = re.match("^ lib([a-zA-Z0-0]+)\.a\(\.bss", l)
        z3 = re.match(" \*fill\* +(0x[0-9a-f]{16}) +(0x[0-9a-f]+)", l)
        z4 = re.match("^ lib([a-zA-Z0-0]+)\.a\(\.shared", l)
        if z1:
            set_lib_alloc(z1.groups()[0], 'data')

        elif z2:
            set_lib_alloc(z2.groups()[0], 'bss')

        elif z4:
            set_lib_alloc(z4.groups()[0], 'shared')

        elif z3 and lib is not None:
            alloc[typa][lib].add_fill(int(z3.groups()[1], 16))
            print("Fill: %s" % (z3.groups()[1]))
        else:
            lib = None
            typa = None

    else:
        z = re.search(" +(0x[0-9a-f]{16}) +(0x[0-9a-f]+)", l)
        if z and lib is not None:
            alloc[typa][lib].add_alloc(int(z.groups()[1], 16))
            print("Alloc: %s" % (z.groups()[1]))

def sort_by_size(container):
    out = []
    for k in container.keys():
        out.append((k, container[k]))

    out.sort(key = lambda item : item[1].get_size(), reverse = True)

    return out

def write_linker_script(container, fname, sname):
    f = open(fname, "w")
    
    for lib in container:
        align = int(math.pow(2, lib[1].get_size().bit_length()))
        if align < 0x100:
            align = 0x100

        f.write("/* Application name: %s */\n" % (lib[0]))
        f.write("\t. = ALIGN(0x%X);\n" % (align))
        f.write("\t%s_%s_start = .;\n" % (lib[0], sname))
        f.write("\tlib%s.a(.%s .%s.*)\n" % (lib[0], sname, sname))
        f.write("\t. = ALIGN(0x%X);\n" % (align))
        f.write("\t%s_%s_end = .;\n\n" % (lib[0], sname))

    f.close()

data = sort_by_size(alloc['data'])
write_linker_script(data, sys.argv[2], 'data')

bss = sort_by_size(alloc['bss'])
write_linker_script(bss, sys.argv[3], 'bss')

shared = sort_by_size(alloc['shared'])
write_linker_script(shared, sys.argv[4], 'shared')

