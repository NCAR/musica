import json
import sys

filename = sys.argv[1]
with open(filename) as f:
    nb = json.load(f)

errors = []
for i, cell in enumerate(nb["cells"]):
    if cell["cell_type"] == "code":
        for output in cell.get("outputs", []):
            if output["output_type"] == "error":
                errors.append((i, output))

print(f"Found {len(errors)} error(s)")
for i, err in errors:
    print(f"Cell {i}: {err.get('ename')}: {err.get('evalue')}")
