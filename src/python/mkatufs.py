import argparse
parser = argparse.ArgumentParser()
from pathlib import Path
parser.add_argument("file", help="Format")
parser.add_argument("kernel", help= "Kernel")
args = parser.parse_args()
import math
import io
import struct
from datetime import datetime, timezone
buffer = io.BytesIO()

r = 272
def fixcluster(totalclusters_base):
    totalclusters = totalclusters_base

    while True:
        bmp = math.ceil((totalclusters / 8) / 512)
        new_total = totalclusters_base - bmp

        if new_total == totalclusters:
            return new_total, bmp

        totalclusters = new_total

def write_superblock():
    buffer.write(struct.pack("<H", 0xF5A7)) # magic code
    buffer.write(struct.pack("<B", 0))
    buffer.write(struct.pack("<H", block)) # block size
    buffer.write(struct.pack("<H", 512)) # file size
    buffer.write(struct.pack("<H", 10)) # boot2 size in blocks
    buffer.write(struct.pack("<B", 4)) # skipped blocks
    buffer.write(struct.pack("<I", totalfiles))
    buffer.write(struct.pack("<I", totalclusters))
    buffer.write(struct.pack("<I", 3)) # alocated files
    buffer.write(struct.pack("<I", 40)) # alocated clusters
    buffer.write(struct.pack("<H", journalsize)) # size journal in blocks
    buffer.write(struct.pack("<I", startbmpfile))
    buffer.write(struct.pack("<I", startbmpcluster))
    buffer.write(struct.pack("<I", file0))
    buffer.write(struct.pack("<I", cluster0))
    buffer.write(b'ATUOS   ') # fs label
with open(args.file, "r+b") as disk:
    diskf = Path(args.file)
    disksizeb = diskf.stat().st_size
    disksizes = math.ceil(disksizeb / 512)
    totalfiles = disksizeb // 16384
    totalfiles -= 1
    block = 512
    bmpfilessize = math.ceil((totalfiles / 8) / 512)
    startbmpfile = 2 + 10 + 4
    journalsize = 2
    totalclusters = disksizes - (
    startbmpfile +
    bmpfilessize +
    journalsize +
    totalfiles
    )
    bmpclusterssize = math.ceil((totalclusters / 8) / 512)
    totalclusters -= bmpclusterssize
    totalclusters, bmpclusterssize = fixcluster(totalclusters)
    startbmpcluster = startbmpfile + bmpfilessize

    file0 = startbmpcluster + bmpclusterssize + journalsize
    cluster0 = file0 + totalfiles
    buffer.write(struct.pack("<H", 0x32EB)) # jmp to after the info
    write_superblock()
    buffer.seek(510)
    buffer.write(struct.pack("<H", 0xAA55))
    buffer.seek(512)
    buffer.write(struct.pack("<H", 0xF5A7)) # two magic code, why not?????
    write_superblock()
    buffer.seek(1024)
    buffer.seek(startbmpfile*512)
    buffer.write(struct.pack("<B", 7)) # reservar o primeiro file para o root, kernel e teste.txt

    buffer.seek(file0*512)

    dt_atual_utc = datetime.now(timezone.utc)
    unix_time = int(dt_atual_utc.timestamp())
    buffer.write(struct.pack("<I", 35)) # size low
    buffer.write(struct.pack("<H", 0)) # size high
    buffer.write(struct.pack("<I", unix_time)) # last mod in unix
    buffer.write(struct.pack("<I", unix_time)) # last access
    buffer.write(struct.pack("<I", unix_time)) # creation
    buffer.write(struct.pack("<H", 0)) # userid (0 for kernel)
    buffer.write(struct.pack("<B", 0b00000001)) # atributes
    buffer.write(struct.pack("<B", 0)) # reserved byte
    # kernel
    buffer.write(struct.pack("<I", 1)) # file apontado
    buffer.write(struct.pack("<H", 18)) # tamanho da entry
    buffer.write(struct.pack("<B", 0x20)) # tipo
    buffer.write(struct.pack("<B", 10)) # tamanho do nome
    buffer.write(b'kernel.elf') # nome

    # teste.txt
    buffer.write(struct.pack("<I", 2))
    buffer.write(struct.pack("<H", 17))
    buffer.write(struct.pack("<B", 0x20))
    buffer.write(struct.pack("<B", 9))
    buffer.write(b'teste.txt')

    # file 1
    buffer.seek((file0+1)*512)
    kernelp = Path(args.kernel)
    kernelsize = kernelp.stat().st_size
    kernelsizeinclus = math.ceil(kernelsize / 512)
    buffer.write(struct.pack("<I", kernelsize)) # size low
    dt_atual_utc = datetime.now(timezone.utc)
    unix_time = int(dt_atual_utc.timestamp())
    buffer.write(struct.pack("<H", 0)) # size high
    buffer.write(struct.pack("<I", unix_time)) # last mod in unix
    buffer.write(struct.pack("<I", unix_time)) # last access
    buffer.write(struct.pack("<I", unix_time)) # creation
    buffer.write(struct.pack("<H", 0)) # userid (0 for kernel)
    buffer.write(struct.pack("<B", 0b10000000)) # atributes
    buffer.write(struct.pack("<B", 0)) # reserved byte
    buffer.write(struct.pack("<I", 0)) # first cluster
    buffer.write(struct.pack("<I", kernelsizeinclus))
    # file 2
    buffer.seek((file0+2)*512)
    buffer.write(struct.pack("<I", 171))
    buffer.write(struct.pack("<H", 0))
    buffer.write(struct.pack("<I", unix_time)) # last mod in unix
    buffer.write(struct.pack("<I", unix_time)) # last access
    buffer.write(struct.pack("<I", unix_time)) # creation
    buffer.write(struct.pack("<H", 0)) # userid (0 for kernel)
    buffer.write(struct.pack("<B", 0b00000000)) # atributes
    buffer.write(struct.pack("<B", 0)) # reserved byte
    buffer.write(b'Paging is a system which allows each process to see a full virtual address space, without actually requiring the full amount of physical memory to be available or present.')
    buffer.seek(cluster0*512)
    with open(args.kernel, "rb") as kernel:
        buffer.write(kernel.read())
    # reservar os clusters do kernel
    buffer.seek(startbmpcluster*512)
    for i in range(kernelsizeinclus // 8):
        buffer.write(struct.pack("<B", 0xFF))
    if kernelsizeinclus % 8 > 0:
        buffer.write(struct.pack("<B", 2**(kernelsizeinclus % 8) - 1))
    disk.write(buffer.getvalue())

