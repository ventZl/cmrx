import sys
from pprint import pprint
import re
import math
import os

f = open(sys.argv[1])

lines = f.readlines()

alloc = { 'data' : {}, 'bss' : {}, 'shared' : {}}
lib = None
typa = None

def set_lib_alloc(_lib, kind, location):
    global lib
    global typa
    global libpath
    lib = _lib
    if lib not in alloc[kind]:
        alloc[kind][lib] = Section(location)

    typa = kind
    libpath = location
#    print("== %s %s:" % (lib, typa.upper()))

class Section:
    def __init__(self, location):
        self._location = location
        self.size = 0
        self.fill = 0

    def location(self):
        return self._location

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
#    print(l)
    if  l[0] != ' ':
 #       print("!! Dropping")
        lib = None
        typa = None
    elif l[0] == ' ':
        z1 = re.match("^ (.*lib([a-zA-Z0-9_]+)\.a)\(\.data \.data\.\*\)$", l)
        z2 = re.match("^ (.*lib([a-zA-Z0-9_]+)\.a)\(\.bss \.bss\.\*\)$", l)
        z3 = re.match(" \*fill\* +(0x[0-9a-f]{16}) +(0x[0-9a-f]+)", l)
        z4 = re.match("^ (.*lib([a-zA-Z0-9_]+)\.a)\(\.shared \.shared\.\*\)$", l)
        z5 = re.search("(0x[0-9a-f]{16}) +(0x[0-9a-f]+) (.*)\(", l)
        if z1:
            set_lib_alloc(z1.groups()[1], 'data', z1.groups()[0])

        elif z2:
            set_lib_alloc(z2.groups()[1], 'bss', z2.groups()[0])

        elif z4:
            set_lib_alloc(z4.groups()[1], 'shared', z4.groups()[0])

        elif z3 and lib is not None:
            alloc[typa][lib].add_fill(int(z3.groups()[1], 16))
#            print("Fill: %s" % (z3.groups()[1]))
        elif l[1] == '*':
  #          print("!!! Dropping")
            lib = None
            typa = None
        elif z5 and lib is not None:
            if (libpath == z5.groups()[2]):
                alloc[typa][lib].add_alloc(int(z5.groups()[1], 16))
#                print("Alloc: %s" % (z5.groups()[1]))
            else:
                print("Something found, but ignored because libpath is %s but should be %s" % (z5.groups()[2], libpath))

def sort_by_size(container):
    out = []
    for k in container.keys():
        out.append((k, container[k]))

    out.sort(key = lambda item : item[1].get_size(), reverse = True)

    return out

def write_linker_script(container, fname, sname):
    f = open(fname, "r")
    old_content = f.read()
    f.close()

    new_content = ""
    
    for lib in container:
        align = int(math.pow(2, lib[1].get_size().bit_length()))
        if align < 0x100:
            align = 0x100

        new_content += "/* Application name: %s */\n" % (lib[0])
        new_content += "\t. = ALIGN(0x%X);\n" % (align)
        new_content += "\t%s_%s_start = .;\n" % (lib[0], sname)
        new_content += "\t%s(.%s .%s.*)\n" % (lib[1].location(), sname, sname)
        new_content += "\t. = ALIGN(0x%X);\n" % (align)
        new_content += "\t%s_%s_end = .;\n\n" % (lib[0], sname)

    if (new_content != old_content):
        f = open(fname, "w")
        f.write(new_content)
        f.close()
        return True

    return False

deleteBinary = False

data = sort_by_size(alloc['data'])
if write_linker_script(data, sys.argv[2], 'data'):
    deleteBinary = True

bss = sort_by_size(alloc['bss'])
if write_linker_script(bss, sys.argv[3], 'bss'):
    deleteBinary = True

shared = sort_by_size(alloc['shared'])
if write_linker_script(shared, sys.argv[4], 'shared'):
    deleteBinary = True

if deleteBinary:
    binary = sys.argv[1].replace(".map", ".elf")
    os.remove(binary)
    print("Removing... %s" % (binary))

    print("Linker script updated, please rebuild target...")
else:
    print("Target up to date");
