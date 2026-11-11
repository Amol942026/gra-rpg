"""Embed Common Controls v6 in our generated GNU unit-test executable.

Tauri adds this resource to its application binary, while Cargo's lib-test
target lacks it. This only edits owned build output, not any system files.
"""
import argparse
import ctypes
from ctypes import wintypes
from pathlib import Path

MANIFEST = b'''<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
<dependency><dependentAssembly><assemblyIdentity type="win32"
name="Microsoft.Windows.Common-Controls" version="6.0.0.0"
processorArchitecture="*" publicKeyToken="6595b64144ccf1df" language="*" />
</dependentAssembly></dependency></assembly>'''


def embed(path):
    target = Path(path).resolve(strict=True)
    build_root = Path("C:/AI2D/Tools/SpriteStudio-v0.3.3/rust-target").resolve()
    if not target.is_relative_to(build_root) or not target.name.startswith("sprite_maker_lib-") or target.suffix != ".exe":
        raise ValueError("Only the owned Sprite Studio Cargo lib-test binary may be edited")
    kernel = ctypes.WinDLL("kernel32", use_last_error=True)
    kernel.BeginUpdateResourceW.argtypes = [wintypes.LPCWSTR, wintypes.BOOL]
    kernel.BeginUpdateResourceW.restype = wintypes.HANDLE
    kernel.UpdateResourceW.argtypes = [wintypes.HANDLE, ctypes.c_void_p, ctypes.c_void_p,
                                      wintypes.WORD, ctypes.c_void_p, wintypes.DWORD]
    kernel.UpdateResourceW.restype = wintypes.BOOL
    kernel.EndUpdateResourceW.argtypes = [wintypes.HANDLE, wintypes.BOOL]
    kernel.EndUpdateResourceW.restype = wintypes.BOOL
    handle = kernel.BeginUpdateResourceW(str(target), False)
    if not handle:
        raise ctypes.WinError(ctypes.get_last_error())
    data = ctypes.create_string_buffer(MANIFEST)
    if not kernel.UpdateResourceW(handle, 24, 1, 0x409, data, len(MANIFEST)):
        error = ctypes.get_last_error()
        kernel.EndUpdateResourceW(handle, True)
        raise ctypes.WinError(error)
    if not kernel.EndUpdateResourceW(handle, False):
        raise ctypes.WinError(ctypes.get_last_error())
    print("Embedded Common Controls v6 in generated unit-test binary")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("target")
    embed(parser.parse_args().target)
