import argparse
import struct
parser = argparse.ArgumentParser()
parser.add_argument("file", help="Format")
parser.add_argument("build", help="aaaaa")
args = parser.parse_args()

boot1 = bytes.fromhex(
    "FC FA 8C C8 8E D8 8E C0 8E D0 BC FF 7B FB "
    "52 BB 00 02 A1 05 7C 31 D2 F7 F3 89 C3 A1 "
    "09 7C F7 E3 5A 31 C0 8E C0 BB 00 80 B4 02 "
    "B0 05 B6 00 B5 00 B1 03 CD 13 72 05 EA 00 "
    "80 00 00 B4 0E B0 61 CD 10 EB F8"
)
with open(args.file, "r+b") as disk:
    disk.seek(1024)
    with open(args.build+"/boot2.bin", "rb") as boot2:
        disk.write(boot2.read())
        for i in range(256):
            disk.write(struct.pack("<I", 0))
    disk.seek(54)
    disk.write(boot1)