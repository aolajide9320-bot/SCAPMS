import os
import sys
import time
import subprocess
import ctypes
from ctypes import wintypes
from PIL import Image

user32 = ctypes.windll.user32
gdi32 = ctypes.windll.gdi32

# Win32 Function Signatures for 64-bit
user32.GetWindowDC.argtypes = [wintypes.HWND]
user32.GetWindowDC.restype = wintypes.HDC
user32.ReleaseDC.argtypes = [wintypes.HWND, wintypes.HDC]
user32.ReleaseDC.restype = ctypes.c_int

gdi32.CreateCompatibleDC.argtypes = [wintypes.HDC]
gdi32.CreateCompatibleDC.restype = wintypes.HDC
gdi32.DeleteDC.argtypes = [wintypes.HDC]
gdi32.DeleteDC.restype = wintypes.BOOL

gdi32.CreateCompatibleBitmap.argtypes = [wintypes.HDC, ctypes.c_int, ctypes.c_int]
gdi32.CreateCompatibleBitmap.restype = wintypes.HBITMAP
gdi32.DeleteObject.argtypes = [wintypes.HGDIOBJ]
gdi32.DeleteObject.restype = wintypes.BOOL

gdi32.SelectObject.argtypes = [wintypes.HDC, wintypes.HGDIOBJ]
gdi32.SelectObject.restype = wintypes.HGDIOBJ

user32.PrintWindow.argtypes = [wintypes.HWND, wintypes.HDC, wintypes.UINT]
user32.PrintWindow.restype = wintypes.BOOL

gdi32.GetDIBits.argtypes = [
    wintypes.HDC, wintypes.HBITMAP, wintypes.UINT, wintypes.UINT,
    wintypes.LPVOID, ctypes.c_void_p, wintypes.UINT
]
gdi32.GetDIBits.restype = ctypes.c_int

user32.SendMessageW.argtypes = [wintypes.HWND, wintypes.UINT, wintypes.WPARAM, wintypes.LPARAM]
user32.SendMessageW.restype = wintypes.LPARAM

WM_CLOSE = 0x0010
WM_NOTIFY = 0x004E
ID_MAIN_TAB = 1001
TCM_SETCURSEL = 0x1300 + 12
TCN_SELCHANGE = 0xFFFFFFFF - 551 + 1 # unsigned representation of -551

class RECT(ctypes.Structure):
    _fields_ = [
        ('left', wintypes.LONG),
        ('top', wintypes.LONG),
        ('right', wintypes.LONG),
        ('bottom', wintypes.LONG)
    ]

class NMHDR(ctypes.Structure):
    _fields_ = [
        ('hwndFrom', wintypes.HWND),
        ('idFrom', ctypes.c_size_t),
        ('code', wintypes.UINT)
    ]

class BITMAPINFOHEADER(ctypes.Structure):
    _fields_ = [
        ('biSize', wintypes.DWORD),
        ('biWidth', wintypes.LONG),
        ('biHeight', wintypes.LONG),
        ('biPlanes', wintypes.WORD),
        ('biBitCount', wintypes.WORD),
        ('biCompression', wintypes.DWORD),
        ('biSizeImage', wintypes.DWORD),
        ('biXPelsPerMeter', wintypes.LONG),
        ('biYPelsPerMeter', wintypes.LONG),
        ('biClrUsed', wintypes.DWORD),
        ('biClrImportant', wintypes.DWORD)
    ]

def find_main_window():
    found_hwnd = None
    def enum_cb(hwnd, lparam):
        nonlocal found_hwnd
        length = user32.GetWindowTextLengthW(hwnd)
        if length > 0:
            buff = ctypes.create_unicode_buffer(length + 1)
            user32.GetWindowTextW(hwnd, buff, length + 1)
            if "Smart Clinic Appointment & Patient Management System" in buff.value:
                found_hwnd = hwnd
                return False
        return True

    cb_type = ctypes.WINFUNCTYPE(ctypes.c_bool, wintypes.HWND, wintypes.LPARAM)
    user32.EnumWindows(cb_type(enum_cb), 0)
    return found_hwnd

def find_tab_control(parent_hwnd):
    tab_hwnd = None
    def enum_child_cb(hwnd, lparam):
        nonlocal tab_hwnd
        class_name = ctypes.create_unicode_buffer(256)
        user32.GetClassNameW(hwnd, class_name, 256)
        if class_name.value == "SysTabControl32":
            tab_hwnd = hwnd
            return False
        return True

    cb_type = ctypes.WINFUNCTYPE(ctypes.c_bool, wintypes.HWND, wintypes.LPARAM)
    user32.EnumChildWindows(parent_hwnd, cb_type(enum_child_cb), 0)
    return tab_hwnd

def capture_window(hwnd, out_path):
    rect = RECT()
    user32.GetWindowRect(hwnd, ctypes.byref(rect))
    w = rect.right - rect.left
    h = rect.bottom - rect.top
    if w <= 0 or h <= 0:
        return False

    hwndDC = user32.GetWindowDC(hwnd)
    memDC = gdi32.CreateCompatibleDC(hwndDC)
    hBitmap = gdi32.CreateCompatibleBitmap(hwndDC, w, h)
    oldBmp = gdi32.SelectObject(memDC, hBitmap)

    user32.PrintWindow(hwnd, memDC, 2)

    bmi = BITMAPINFOHEADER()
    bmi.biSize = ctypes.sizeof(BITMAPINFOHEADER)
    bmi.biWidth = w
    bmi.biHeight = -h  # top-down
    bmi.biPlanes = 1
    bmi.biBitCount = 32
    bmi.biCompression = 0

    buf = ctypes.create_string_buffer(w * h * 4)
    lines = gdi32.GetDIBits(memDC, hBitmap, 0, h, buf, ctypes.byref(bmi), 0)

    gdi32.SelectObject(memDC, oldBmp)
    gdi32.DeleteObject(hBitmap)
    gdi32.DeleteDC(memDC)
    user32.ReleaseDC(hwnd, hwndDC)

    if lines > 0:
        img = Image.frombuffer('RGBA', (w, h), buf, 'raw', 'BGRA', 0, 1)
        img = img.convert('RGB')
        for attempt in range(5):
            try:
                img.save(out_path, format='PNG')
                return True
            except Exception as e:
                time.sleep(0.5)
        return False
    return False

def switch_to_tab(hwnd, hTab, tab_idx):
    if not hTab:
        return
    # 1. Set current selection in tab control
    user32.SendMessageW(hTab, TCM_SETCURSEL, tab_idx, 0)
    # 2. Notify parent of TCN_SELCHANGE
    nmhdr = NMHDR()
    nmhdr.hwndFrom = hTab
    nmhdr.idFrom = ID_MAIN_TAB
    nmhdr.code = TCN_SELCHANGE
    user32.SendMessageW(hwnd, WM_NOTIFY, ID_MAIN_TAB, ctypes.addressof(nmhdr))

def main():
    screenshot_dir = r"c:\Users\User\Documents\Antigravity\SCAPMS\screenshots"
    os.makedirs(screenshot_dir, exist_ok=True)
    exe_path = r"c:\Users\User\Documents\Antigravity\SCAPMS\bin\scapms.exe"

    print(f"Launching {exe_path}...")
    proc = subprocess.Popen([exe_path])
    time.sleep(2.0)

    hwnd = find_main_window()
    if not hwnd:
        print("Error: Could not locate main window.")
        proc.terminate()
        return 1

    print(f"Found main window handle: {hwnd}")
    user32.ShowWindow(hwnd, 9) # SW_RESTORE
    user32.SetForegroundWindow(hwnd)
    user32.SetWindowPos(hwnd, 0, 50, 50, 1080, 750, 0x0040)
    time.sleep(0.8)

    hTab = find_tab_control(hwnd)
    print(f"Found tab control handle: {hTab}")

    tab_specs = [
        (0, "1_main_dashboard.png", "Main Dashboard"),
        (1, "2_patient_registration.png", "Patient Registration & Records"),
        (2, "3_appointment_management.png", "Appointment Management"),
        (3, "4_waiting_queue.png", "Waiting Queue (FIFO)"),
        (4, "5_search_sort_interface.png", "Search & Sort Interface"),
        (5, "6_clinical_reports.png", "Clinical Reports & Analytics")
    ]

    for idx, filename, label in tab_specs:
        print(f"Switching to Tab {idx} ({label})...")
        switch_to_tab(hwnd, hTab, idx)
        time.sleep(1.0)

        out_path = os.path.join(screenshot_dir, filename)
        if capture_window(hwnd, out_path):
            print(f" -> Successfully saved: {out_path} ({os.path.getsize(out_path)} bytes)")
        else:
            print(f" -> Failed to capture: {out_path}")

    def find_window_by_class(class_name):
        found = None
        def enum_cb(h, lp):
            nonlocal found
            cname = ctypes.create_unicode_buffer(256)
            user32.GetClassNameW(h, cname, 256)
            if cname.value == class_name:
                found = h
                return False
            return True
        cb_type = ctypes.WINFUNCTYPE(ctypes.c_bool, wintypes.HWND, wintypes.LPARAM)
        user32.EnumWindows(cb_type(enum_cb), 0)
        return found

    # Capture Patient Dialog
    print("Opening Patient Registration Modal Dialog...")
    user32.PostMessageW(hwnd, 0x0111, 5002, 0) # IDC_BTN_ADD_PATIENT
    time.sleep(1.0)
    dlg_pat = find_window_by_class("SCAPMS_PatientDialogClass")
    if dlg_pat:
        print(f"Found Patient Dialog handle: {dlg_pat}")
        out_path = os.path.join(screenshot_dir, "7_patient_dialog.png")
        if capture_window(dlg_pat, out_path):
            print(f" -> Successfully saved: {out_path} ({os.path.getsize(out_path)} bytes)")
        user32.PostMessageW(dlg_pat, WM_CLOSE, 0, 0)
        time.sleep(0.6)

    # Capture Appointment Dialog
    print("Opening Appointment Booking Modal Dialog...")
    user32.PostMessageW(hwnd, 0x0111, 6002, 0) # IDC_BTN_SCHEDULE_APPOINTMENT
    time.sleep(1.0)
    dlg_apt = find_window_by_class("SCAPMS_AppointmentDialogClass")
    if dlg_apt:
        print(f"Found Appointment Dialog handle: {dlg_apt}")
        out_path = os.path.join(screenshot_dir, "8_appointment_dialog.png")
        if capture_window(dlg_apt, out_path):
            print(f" -> Successfully saved: {out_path} ({os.path.getsize(out_path)} bytes)")
        user32.PostMessageW(dlg_apt, WM_CLOSE, 0, 0)
        time.sleep(0.6)

    print("Closing application...")
    user32.PostMessageW(hwnd, WM_CLOSE, 0, 0)
    time.sleep(0.5)
    print("Done generating all required screenshots.")
    return 0

if __name__ == "__main__":
    sys.exit(main())
