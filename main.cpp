#include <iostream>
#include <windows.h>
#include <winerror.h>
#include <winuser.h>
HHOOK kHook;
bool capsPressed = false;
bool dualKey = false;

void input(WORD vkCode, WORD scanCode, bool keyup) {
  INPUT input = {0};
  input.type = INPUT_KEYBOARD;
  input.ki.wVk = vkCode;
  input.ki.wScan=scanCode;
  input.ki.dwExtraInfo = (ULONG_PTR)0x12345678;
  bool is_extended_key = scanCode >> 8 == 0xE0;
  input.ki.dwFlags = (keyup ? KEYEVENTF_KEYUP : 0) |
                     (is_extended_key ? KEYEVENTF_EXTENDEDKEY : 0);

  SendInput(1, &input, sizeof(input));
}
LRESULT CALLBACK hookKeyboard(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION) {
    bool keydown = false;
    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
      keydown = true;
    }

    KBDLLHOOKSTRUCT *key = (KBDLLHOOKSTRUCT *)lParam;

    if (key->dwExtraInfo != 0x12345678) {
      if (key->vkCode == VK_CAPITAL) {
        if (keydown) {
          capsPressed = true;
        } else {
          capsPressed = false;
          if (dualKey) {
            input(VK_CONTROL, 0, 1);
            dualKey = false;
          }
          else{
            // Send Esc key
            input(VK_ESCAPE, 0, 0);
            input(VK_ESCAPE, 0, 1);
          }
        }
        return 1;
      } else {
        if (keydown) {
          if (capsPressed) {
            // Send Ctrl+Key
            dualKey = true;
            input(VK_CONTROL, 0, 0);
            input(key->vkCode, key->scanCode, 0);
            return 1;
          }
        } else {
          if (dualKey) {
            // Send Ctrl+Key
            input(key->vkCode, key->scanCode, 1);
            return 1;
          }
        }
      }
    }
  }
  return CallNextHookEx(kHook, nCode, wParam, lParam);
}

int main() {
  HANDLE mutex = CreateMutex(NULL, TRUE, "key-remap.single");
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    MessageBox(NULL,"Program is already running!","Alert",MB_OK);
    return -1;
  }

  kHook = SetWindowsHookEx(WH_KEYBOARD_LL, hookKeyboard, NULL, 0);
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}
