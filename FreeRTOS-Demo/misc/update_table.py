import yaml
from pathlib import Path

build_dir = Path(__file__).parent.parent / "build" / "minsizerel"

for file in sorted(build_dir.glob("*.map.yaml")):
    print(file.name, end="\t")
    with file.open() as fp:
        data = yaml.safe_load(fp)
    for area in data.values():
        for section in area:
            print(section["name"], section["size"], sep=" ", end="\t")
    print()
