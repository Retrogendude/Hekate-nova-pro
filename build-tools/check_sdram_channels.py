"""Check actual source byte-extraction expressions; no console access required."""
import ast
from pathlib import Path
import re

source = Path(__file__).parent / "hekate-6.5.4/bdk/mem/sdram.c"
text = source.read_text(encoding="utf-8")
assignments = dict(re.findall(
    r"data\.(chip[01]\.rank[01]_ch[01])\s*=\s*([^;]+);", text
))
assert len(assignments) == 8, f"Expected eight extractions, found {len(assignments)}"
allowed = (ast.Expression, ast.BinOp, ast.BitAnd, ast.RShift,
           ast.Name, ast.Load, ast.Constant)
samples = (0xA15C, 0x12FE, 0xFF00, 0x00FF)
checked = 0
for field, expression in assignments.items():
    tree = ast.parse(expression, mode="eval")
    assert all(isinstance(node, allowed) for node in ast.walk(tree)), expression
    assert all(node.id == "mrr" for node in ast.walk(tree) if isinstance(node, ast.Name))
    for mrr in samples:
        actual = eval(compile(tree, str(source), "eval"), {"__builtins__": {}}, {"mrr": mrr})
        expected = mrr.to_bytes(4, "little")[int(field[-1])]
        assert actual == expected, (field, expression, hex(mrr), actual, expected)
        checked += 1
print(f"PASS: {len(assignments)} source extractions, {checked} distinct-byte cases")
