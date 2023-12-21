#include <iostream>
#include <string.h>
#include <chrono>

#include <windows.h>
#include <winerror.h>
#include <winuser.h>

#define TRAY_WINAPI 1
#include "tray.h"

#define UNUSED [[maybe_unused]]


long long getTime() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
long long lastTime = 0;

DWORD mainThreadID = GetCurrentThreadId();
DWORD trayThreadID = 0;

HHOOK kHook;
bool capsPressed = false;
bool altPressed = false;
bool hPressed = false, jPressed = false, kPressed = false, lPressed = false;
bool dualKey = false;

/// Keyboard stuff ///
// Send keyboard input
void inputKey(WORD vkCode, WORD scanCode, bool keyup) {
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

// Send mouse input
void inputMouse(bool isLeft, bool keyup) {
  INPUT input = {
    .type = INPUT_MOUSE,
    .mi   = {
      .dx           = 0,
      .dy           = 0,
      .mouseData    = 0,
      .dwFlags      = (DWORD)(
                        isLeft ? 
                          (keyup ? MOUSEEVENTF_LEFTUP : MOUSEEVENTF_LEFTDOWN)
                        :
                          (keyup ? MOUSEEVENTF_RIGHTUP : MOUSEEVENTF_RIGHTDOWN)
                      ),
      .time        = 0,
      .dwExtraInfo = (ULONG_PTR)0x12345678
    }
  };

  SendInput(1, &input, sizeof(input));
}

// Timer callback function for cursor movement
VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    // Process mouse cursor movement logic here based on the keyboard input states
    POINT p;
    if (GetCursorPos(&p)) {
      if(!altPressed) return;
      long long time = getTime();
      long long delta = (lastTime == 0) ? 0 : time - lastTime;

      int _h = (int)lPressed - (int)hPressed;
      int _v = (int)jPressed - (int)kPressed;

      double _spd = (double)delta*0.5;
      if(capsPressed) _spd *= 2.0;
      if(_h != 0 && _v != 0) _spd *= 0.70710678118;

      SetCursorPos(p.x + (int)(_h*_spd), p.y + (int)(_v*_spd));

      lastTime = time;
    }
}

// Keyboard hook callback function
LRESULT CALLBACK hookKeyboard(int nCode, WPARAM wParam, LPARAM lParam) {
  bool keydown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
  KBDLLHOOKSTRUCT *key = (KBDLLHOOKSTRUCT *)lParam;

  if (nCode != HC_ACTION) goto end;
  if (key->dwExtraInfo == 0x12345678) goto end;

  if (key->vkCode == VK_CAPITAL) {
    capsPressed = keydown;

    // Process CapsLock release logic
    if (!capsPressed) {
      if (dualKey) {
        // Release Ctrl key
        inputKey(VK_CONTROL, 0, 1);
        dualKey = false;
      } else if (!altPressed){
        // Send Esc key
        inputKey(VK_ESCAPE, 0, 0);
        inputKey(VK_ESCAPE, 0, 1);
      }
    }
    return 1;
  } else if (key->vkCode == VK_LMENU) {
    altPressed = keydown;
  } else {
    if (capsPressed) {
      if (keydown) {
        // Send Ctrl+Key
        dualKey = true;
        inputKey(VK_CONTROL, 0, 0);
      }
    }
    switch (key->vkCode) {
      case 0x48: // h
        hPressed = keydown;
        if(altPressed) return 1;
        break;
      case 0x4A: // j
        jPressed = keydown;
        if(altPressed) return 1;
        break;
      case 0x4B: // k
        kPressed = keydown;
        if(altPressed) return 1;
        break;
      case 0x4C: // l
        lPressed = keydown;
        if(altPressed) return 1;
        break; 
      case 0x55: // u
        if(altPressed) {
          inputMouse(true, !keydown);
          return 1;
        }
        break;
      case 0x49: // i
        if(altPressed) {
          inputMouse(false, !keydown);
          return 1;
        }
        break;
    }
  }

end:
  return CallNextHookEx(kHook, nCode, wParam, lParam);
}

/// Tray stuff ///
void toggle_cb(struct tray_menu *item);
void quit_cb(struct tray_menu *item);

struct tray tray = {
    .icon = "MAINICON",
    .menu = (struct tray_menu[]) {
      {"Quit", 0, 0, quit_cb, NULL},
      {NULL, 0, 0, NULL, NULL}
    },
};

void toggle_cb(struct tray_menu *item) {
	item->checked = !item->checked;
	tray_update(&tray);
}

void quit_cb(UNUSED struct tray_menu *item) {
  PostThreadMessage(mainThreadID, WM_QUIT, 0, 0);
  tray_exit();
}

void tray_thread(UNUSED void *arg) {
  tray_init(&tray);
  while (tray_loop(1) == 0);
  tray_exit();
}

// When the user closes the console window
BOOL WINAPI consoleHandler(DWORD signal) {
  if (signal == CTRL_CLOSE_EVENT) {
    // Send WM_QUIT to tray thread
    PostThreadMessage(trayThreadID, WM_QUIT, 0, 0);
    return TRUE;
  }
  return FALSE;
}

/// Main ///
int main() {
  UNUSED HANDLE mutex = CreateMutex(NULL, TRUE, "key-remap.single");
  if (GetLastError() == ERROR_ALREADY_EXISTS) {
    MessageBox(NULL,"Program is already running!","Alert",MB_OK);
    return -1;
  }

  UNUSED HANDLE trayThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)tray_thread, NULL, 0, &trayThreadID);
  if(trayThread == NULL) {
    MessageBox(NULL,"Failed to create tray thread!","Alert",MB_OK);
    return -1;
  }

  SetConsoleCtrlHandler(consoleHandler, TRUE);
  SetTimer(NULL, 0, 8, TimerProc);

  kHook = SetWindowsHookEx(WH_KEYBOARD_LL, hookKeyboard, NULL, 0);
  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0) > 0) {
    if(msg.message == WM_QUIT) break;

    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}