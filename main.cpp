#include <iostream>
#include <string.h>

#include <windows.h>
#include <winerror.h>
#include <winuser.h>

#define TRAY_WINAPI 1
#include "tray.h"

#define UNUSED [[maybe_unused]]


DWORD mainThreadID = GetCurrentThreadId();
HHOOK kHook;
bool capsPressed = false;
bool dualKey = false;

/// Keyboard stuff ///
void input(WORD vkCode, WORD scanCode, bool keyup) {
  bool is_extended_key = scanCode >> 8 == 0xE0;

  INPUT input = {
    .type = INPUT_KEYBOARD,
    .ki   = {
      .wVk          = vkCode,
      .wScan        = scanCode,
      .dwFlags      = (DWORD)(
                        (keyup ? KEYEVENTF_KEYUP : 0) |
                        (is_extended_key ? KEYEVENTF_EXTENDEDKEY : 0)
                      ),
      .time         = 0,
      .dwExtraInfo  = (ULONG_PTR)0x12345678
    }
  };

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
          } else {
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

/// Tray stuff ///
void toggle_cb(struct tray_menu *item);
void quit_cb(struct tray_menu *item);

struct tray tray = {
    .icon = "resources/keycap.ico",
    .menu = (struct tray_menu[]){{"Toggle me", 0, 0, toggle_cb, NULL},
                                 {"-", 0, 0, NULL, NULL},
                                 {"Quit", 0, 0, quit_cb, NULL},
                                 {NULL, 0, 0, NULL, NULL}},
};

void toggle_cb(struct tray_menu *item) {
	item->checked = !item->checked;
	tray_update(&tray);
}

void quit_cb(UNUSED struct tray_menu *item) {
  PostQuitMessage(0);
  tray_exit();
  PostThreadMessage(mainThreadID, WM_QUIT, 0, 0);
}

void tray_thread(UNUSED void *arg) {
  tray_init(&tray);
  while (tray_loop(1) == 0);
  quit_cb(NULL);
}

/// Main ///
int main() {
  UNUSED HANDLE mutex = CreateMutex(NULL, TRUE, "key-remap.single");
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    MessageBox(NULL,"Program is already running!","Alert",MB_OK);
    return -1;
  }

  CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tray_thread, NULL, 0, NULL);

  kHook = SetWindowsHookEx(WH_KEYBOARD_LL, hookKeyboard, NULL, 0);
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0) > 0) {
    if(msg.message == WM_QUIT) break;

    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}