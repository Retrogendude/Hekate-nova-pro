"""Embed a verified-size AArch64 diagnostic stub in the ARM Nyx image."""
from pathlib import Path
import sys
source, target = map(Path, sys.argv[1:])
data = source.read_bytes()
if not data or len(data) > 0x10000:
    raise SystemExit('AArch64 diagnostic stub is missing or exceeds64KiB')
body = ','.join(f'0x{value:02x}' for value in data)
target.write_text('const unsigned char cpu_stub_bin[]={' + body + '};\n'
                  'const unsigned int cpu_stub_bin_size=sizeof(cpu_stub_bin);\n',
                  encoding='utf-8')
