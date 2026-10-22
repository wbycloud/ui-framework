"""Confirm only a pre-authorized fixture certificate dialog for the given PID.
No keyboard/mouse input and no unrelated window text is read.
"""
import ctypes as c
from ctypes import wintypes as w
import sys,time
pid=int(sys.argv[1]);thumb=sys.argv[2].upper();phase=sys.argv[3]
assert len(thumb)==40 and all(ch in '0123456789ABCDEF' for ch in thumb)
titles=('安全警告','Security Warning') if phase=='add' else ('根证书存储','Root Certificate Store')
user=c.WinDLL('user32');callback=c.WINFUNCTYPE(w.BOOL,w.HWND,w.LPARAM)
user.GetWindowThreadProcessId.argtypes=[w.HWND,c.POINTER(w.DWORD)]
user.GetWindowTextW.argtypes=[w.HWND,w.LPWSTR,c.c_int]
user.PostMessageW.argtypes=[w.HWND,w.UINT,w.WPARAM,w.LPARAM]
user.GetDlgItem.argtypes=[w.HWND,c.c_int];user.GetDlgItem.restype=w.HWND
user.EnumChildWindows.argtypes=[w.HWND,callback,w.LPARAM]
def text(hwnd):
    value=c.create_unicode_buffer(4096);user.GetWindowTextW(hwnd,value,len(value));return value.value
confirmed=[]
@callback
def top(hwnd,_):
    owner=w.DWORD();user.GetWindowThreadProcessId(hwnd,c.byref(owner))
    if owner.value!=pid or text(hwnd) not in titles:return True
    lines=[]
    @callback
    def child(handle,_):lines.append(text(handle));return True
    user.EnumChildWindows(hwnd,child,0);content=''.join(lines)
    assert 'localhost' in content and thumb in ''.join(content.split()).upper(),'Fixture certificate identity mismatch'
    button=user.GetDlgItem(hwnd,6)
    assert button and any(label in text(button).lower() for label in ('yes','是')),'Expected certificate Yes button'
    assert user.PostMessageW(button,0xF5,0,0) # BM_CLICK, with the real control notification.
    confirmed.append(hwnd);return True
end=time.monotonic()+20
while not confirmed and time.monotonic()<end:user.EnumWindows(top,0);time.sleep(.05)
assert len(confirmed)==1,'Expected exactly one known certificate dialog'
print('Authorized fixture '+phase+' confirmed; SHA1='+thumb)
