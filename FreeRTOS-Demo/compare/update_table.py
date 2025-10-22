import sys
import yaml
from pathlib import Path

script_dir = Path(__file__).parent
build_dir = script_dir.parent / "build" / "minsizerel"
data_file = script_dir / "data.json"

sys.path.append(str(script_dir.parent.parent / "python"))

from binutils_mapfile.utils import binary_prefix

with data_file.open() as fp:
    data = yaml.safe_load(fp)

summaries = []
for file in sorted(build_dir.glob("*.map.yaml")):
    with file.open() as fp:
        summaries.append(yaml.safe_load(fp))


def iter_sections(summary):
    for area in summary.values():
        for section in area:
            yield section


sections = set(sec["name"] for summary in summaries for sec in iter_sections(summary))
sizes = {
    sec: [
        sum(s["size"] for s in iter_sections(summary) if s["name"] == sec)
        for summary in summaries
    ]
    for sec in sections
}

links = []
table = {"Summary": [d["summary"] for d in data]}
for sec, sec_sizes in sizes.items():
    table[sec] = []
    for n, size in enumerate(sec_sizes):
        string = binary_prefix(size) + "B"
        if n > 0:
            change = size - sec_sizes[n - 1]
            if change < 0:
                string += f" (-{binary_prefix(-change)})"
            elif change > 0:
                string += f" (+{binary_prefix(change)})"
            else:
                string += " (no change)"
            if change != 0:
                string = f"[{string}][{sec}{n}]"
                link = (
                    f"https://kovirobi.github.io/comms-ccf/compare.{n-1}-{n}.svg#{sec}"
                )
                links.append(f"[{sec}{n}]: {link}")
        table[sec].append(string)

columns = [max(len(item) for item in col) for col in table.values()]

for n, col in enumerate(table.keys()):
    print("|", f"{col:{columns[n] + 1}}", end=" ")
print("|")

for n, col in enumerate(table.keys()):
    print("|", "-" * (columns[n] + 1), sep="-", end="-")
print("|")

for row in range(len(data)):
    for n, item in enumerate(table.values()):
        print("|", f"{item[row]:{columns[n] + 1}}", end=" ")
    print("|")

print()
for link in links:
    print(link)
