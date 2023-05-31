# key-remap
Project for using vim in Windows.

## features
For less stress to the finger,

- Remap [Caps Lock] to [Escape]
- Remap [Caps Lock + Other key] to [Ctrl + Other key]

## how to use
Use this command to compile.

    g++ main.cpp -o keymap.exe -mwindows

After compiling, run keymap.exe once.
Add link of keymap.exe to the start menu folder as written below.

    %APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup

Then the program will run automatically every startup.