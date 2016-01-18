#!/usr/bin/python

import struct

#for x in range(0, 0x80):
#    print struct.pack("<H", x | 0x100).encode("hex")


for reg in range(0, 16):
    for x in range(0, 4):
        s = struct.pack("<H", (x << 4) | 0x100 | reg).encode("hex")
        if(x == 1):
            s += "0000"
        print s



