"""Minimize only the verified owned articy process, without focusing/restoring it.

This is process window management through Win32, not input injection or GUI
editing. The native project identity is checked by a live MDK query first.
"""
import ctypes
import argparse
from ctypes import wintypes
import json
import time
from articy_bridge import ArticyBridge, BridgeError


def minimize_owned():
    state = ArticyBridge().call("status")["result"]
    if state.get("projectGuid") != "cb99e03a-ae90-4443-b79f-7f8da1539c34" or not state.get("backgroundSession"):
        raise BridgeError("Not the verified background Emberfall session")
    user = ctypes.WinDLL("user32", use_last_error=True)
    callback_type = ctypes.WINFUNCTYPE(wintypes.BOOL, wintypes.HWND, wintypes.LPARAM)
    user.EnumWindows.argtypes = [callback_type, wintypes.LPARAM]
    user.GetWindowThreadProcessId.argtypes = [wintypes.HWND, ctypes.POINTER(wintypes.DWORD)]
    user.GetWindowTextW.argtypes = [wintypes.HWND, wintypes.LPWSTR, ctypes.c_int]
    user.ShowWindowAsync.argtypes = [wintypes.HWND, ctypes.c_int]
    user.IsIconic.argtypes = [wintypes.HWND]
    matches = []
    def visit(hwnd, _):
        pid = wintypes.DWORD()
        user.GetWindowThreadProcessId(hwnd, ctypes.byref(pid))
        if pid.value == state["processId"]:
            title = ctypes.create_unicode_buffer(1024)
            user.GetWindowTextW(hwnd, title, len(title))
            if title.value.startswith("articy:draft - Emberfall Dialogues ("):
                matches.append(hwnd)
        return True
    user.EnumWindows(callback_type(visit), 0)
    if len(matches) != 1:
        raise BridgeError("Expected exactly one owned main project window")
    hwnd = matches[0]
    if not user.IsIconic(hwnd):
        if not user.ShowWindowAsync(hwnd, 6):
            raise ctypes.WinError(ctypes.get_last_error())
        deadline = time.monotonic() + 3
        while time.monotonic() < deadline and not user.IsIconic(hwnd):
            time.sleep(.05)
    if not user.IsIconic(hwnd):
        raise BridgeError("Native window did not minimize")
    return {"processId": state["processId"], "minimized": True, "inputInjected": False, "restoredOrFocused": False}


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--wait-ready", type=float, default=0)
    args = parser.parse_args()
    if args.wait_ready:
        deadline = time.monotonic() + min(args.wait_ready, 180)
        while time.monotonic() < deadline:
            heartbeat = ArticyBridge().heartbeat()
            if heartbeat.get("live") and heartbeat.get("projectGuid") == "cb99e03a-ae90-4443-b79f-7f8da1539c34" and heartbeat.get("backgroundSession"):
                # The native view can finish showing after ProjectLoaded.
                time.sleep(2)
                break
            time.sleep(.5)
        else:
            raise BridgeError("Project was not opened before background watcher deadline")
    print(json.dumps(minimize_owned()))
