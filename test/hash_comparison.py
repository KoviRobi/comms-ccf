"""
An extremely simple, and probably naive, check of the hash: how does
it differ from the expected distribution. In theory you want the value
of collisions to be as close to the birthday problem given value as
possible.

Fewer collisions actually mean there are more collisions for some other
values: we can't get around the fact that for a 32 bit integer there
are only 2**32 spaces we can end up in.

As an example, if our hash is just considering the first 4 bytes of
the data, then this will not have any collisions for the numbers `0` to
`2**32 - 1`, but will have a lot more for the same number of random data.

The results for size=30 and iterations=10_000_000 are:

    Expected       11633

    Lose-lose 32   3:4 2:11941 1:9976106
     `-distances   153:1 151:1 149:1 148:1 147:1 145:3 144:3 143:8 142:11
                   141:15 140:16 139:39 138:36 137:72 136:77 135:108
                   134:124 133:150 132:206 131:195 130:268 129:330 128:369
                   127:408 126:422 125:507 124:518 123:535 122:575 121:590
                   120:633 119:563 118:567 117:626 116:534 115:496 114:470
                   113:427 112:370 111:320 110:269 109:218 108:189 107:169
                   106:126 105:98 104:77 103:63 102:40 101:39 100:17 99:14
                   98:10 97:8 96:9 95:3 94:1 93:3 91:1 90:1 88:1 86:1

    CRC32          3:8 2:11431 1:9977114
     `-distances   148:2 147:2 146:3 145:6 144:8 143:9 142:5 141:12
                   140:18 139:24 138:35 137:49 136:68 135:97 134:111
                   133:135 132:172 131:218 130:248 129:279 128:365
                   127:420 126:474 125:467 124:533 123:558 122:553
                   121:597 120:586 119:622 118:530 117:555 116:491 115:490
                   114:418 113:404 112:351 111:286 110:245 109:210 108:174
                   107:144 106:105 105:104 104:66 103:64 102:41 101:30
                   100:15 99:14 98:15 97:12 96:5 95:4 94:3 93:2 92:1

    FNV-1A         3:11 2:11683 1:9976601
     `-distances   151:1 150:1 149:2 148:1 147:2 146:2 145:3 144:5
                   143:9 142:8 141:15 140:14 139:35 138:37 137:48
                   136:80 135:92 134:133 133:169 132:174 131:238 130:277
                   129:296 128:374 127:408 126:454 125:465 124:558 123:574
                   122:577 121:603 120:556 119:573 118:547 117:554 116:515
                   115:507 114:455 113:417 112:382 111:294 110:228 109:240
                   108:185 107:136 106:114 105:85 104:70 103:56 102:37
                   101:28 100:28 99:21 98:7 97:8 96:5 95:6 94:4 93:2 91:1

And for numbers up to 2**22

    Expected       8187
    Lose-lose 32   3:127 2:32513 1:8323200
     `-distances   17:32512 10:127 9:255
    CRC32          1:8388607
     `-distances
    FNV-1A         1:8388607
     `-distances


which just shows our testing is rather naive, as two bit-flips can easily
defeat the lose-lose hash.
"""

import shutil
from binascii import crc32
from collections import defaultdict
from random import Random

from fnvhash import fnv1a_32

cols, lines = shutil.get_terminal_size()


def lose32(data: bytes) -> int:
    "The simplest 'hash' from the K&R book -- never intended to be used"
    sum = 0
    for start in range(0, len(data), 4):
        end = min(start + 4, len(data))
        sum += int.from_bytes(bytes=data[start:end], byteorder="little")
        sum = sum & 0xFFFF_FFFF
    return sum


if True:
    ## Random data generation
    # Seed for reproducibility
    r = Random(1234)
    iterations = 10_000_000
    size = 30
    seen = set()
    def data():
        for _iter in range(iterations):
            datum = r.randbytes(size)
            if datum not in seen:
                yield datum
else:
    ## Or comprehensive data generation, takes a minute and a lot of RAM
    bits = 23
    iterations = 2**bits - 1
    def data():
        for i in range(1, bits + 1):
            for j in range(2**i):
                if int.bit_length(j) == i:
                    yield int.to_bytes(j, length=(i+7)//8)

def bar(filled, width):
    line = "=" * filled + " " * (width - filled)
    mid = width // 2
    line = line[: mid - 4] + f"{100 * progress:5.1f}" + line[mid + 4 :]
    return "[" + line + "]"


lose32_count = defaultdict(list)
crc32_count = defaultdict(list)
fnv1a_count = defaultdict(list)

last_progress = 0
for iter, datum in enumerate(data()):
    lose32_count[lose32(datum)].append(datum)
    crc32_count[crc32(datum)].append(datum)
    fnv1a_count[fnv1a_32(datum)].append(datum)

    progress = round(iter / iterations, ndigits=4)
    if progress != last_progress:
        print(bar(int(cols * progress), cols), end="\r")
        last_progress = progress

print()


def birthday_problem_expected(samples, space):
    """
    Expected statistical distribution of collisions -- from the birthday
    problem. The samples is the number of iterations, and the space is
    the number of possible values each sample can take
    """
    return samples - space + space * ((space - 1) / space) ** samples


def hamming_distance(a: bytes, b: bytes) -> int:
    distance = abs(len(a) - len(b))
    for x, y in zip(a, b):
        # XOR gives us the bits in a byte that are not the same
        distance += int.bit_count(~(x ^ y) & 0xFF)
    return distance


print(f"{'Expected':14}", round(birthday_problem_expected(iterations, 2**32)))

for name, count in (
    ("Lose-lose 32", lose32_count),
    ("CRC32", crc32_count),
    ("FNV-1A", fnv1a_count),
):
    histogram = defaultdict(lambda: 0)
    for k, v in count.items():
        histogram[len(v)] += 1
    distances = defaultdict(lambda: 0)
    collisions = {k: v for k, v in count.items() if len(v) > 1}
    for hash, values in collisions.items():
        for i in range(1, len(values)):
            for j in range(i):
                distance = hamming_distance(values[i], values[j])
                distances[distance] += 1
    print(f"{name:14}", *(f"{k}:{v}" for k, v in sorted(histogram.items(), reverse=True)))
    print(f"{' `-distances':14}", *(f"{k}:{v}" for k, v in sorted(distances.items(), reverse=True)))
