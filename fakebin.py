#!/usr/bin/python

hashbin = '''
                                   0e4f
0f43 0b3c 6d4e 8d11 0d5f 0f4d 0f5f 0f5f
0f5f 0f5f 0f5f 0f8d 1e53 ce93 0000 f223
3041 0b12 0a12 0912 0b4f 0a4e 094d 1e4f'''


buff = hashbin.replace(" ", "").replace("\n", "")


with open("fake.bin", "wb") as f:
    f.write(buff.decode("hex"))
