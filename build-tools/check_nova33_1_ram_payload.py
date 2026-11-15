"""Validate generated RAM image using the same C decoder as its ARM loader."""
from pathlib import Path
import ctypes
import subprocess

work = Path(__file__).resolve().parent
hwtest = work / 'HWTest-RCM-nova33_1'
reference = work / 'HWTest-RCM-0.15'
out = hwtest / 'build-ram'
gcc = work / 'toolchains/w64devkit/bin/gcc.exe'
dll = work / 'check_nova33_1_ram_decoder.dll'
subprocess.run([str(gcc), '-O2', '-shared', str(reference/'third_party/hekate/bdk/libs/compr/lz.c'), '-o', str(dll)], check=True)
decoder = ctypes.CDLL(str(dll)).LZ_Uncompress
decoder.argtypes = [ctypes.c_void_p, ctypes.c_void_p, ctypes.c_uint]
decoder.restype = ctypes.c_uint
packed = (out/'payload.lz').read_bytes()
raw = (out/'hwtest_ram_unc.bin').read_bytes()
source = ctypes.create_string_buffer(packed)
dest = ctypes.create_string_buffer(len(raw))
size = decoder(source, dest, len(packed))
assert size == len(raw)
assert dest.raw == raw
assert (out/'hwtest_ram.bin').stat().st_size <= 130392
arm_oc = Path('C:/devkitPro/devkitARM/bin/arm-none-eabi-objcopy.exe')
verified = work/'verify-nova33_1-ram-loader.bin'
subprocess.run([str(arm_oc), '-O', 'binary', str(out/'loader.elf'), str(verified)], check=True)
assert verified.read_bytes() == (out/'hwtest_ram.bin').read_bytes()
print('PASS: actual C loader decoder reproduces full uncompressed RAM image; ARM loader matches ELF; payload within limit.')
print('RAM payload bytes:',(out/'hwtest_ram.bin').stat().st_size,'uncompressed bytes:',len(raw))
