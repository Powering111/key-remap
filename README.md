# Key-Remap

Key-Remap is a utility program that allows you to remap your keyboard keys for enhanced productivity and convenience in Windows, inspired by VIM. It rearranges the behavior of various keys on your keyboard, making it easier to perform certain actions and streamline your workflow.

Please note that Key-Remap is not fully customizable and only supports a limited number of fixed keyboard remaps, which are listed below.

## Features

- Remap the **Caps lock** key to the **Escape key.**
- Remap combinations of **Caps lock** with other keys to **Ctrl+other keys** for easy shortcuts.
- Use **Alt+hjkl** to control mouse cursor movement.
- Utilize **Alt+Caps lock+hjkl** for fast mouse cursor movement.
- Assign **Alt+u/i** to left/right mouse click.
- Restore the original functionality of the **Caps lock** key when used in combination with **Shift**.
- Run as a background process without a visible window, ensuring seamless integration into your system.
- Display a tray icon for quick access to the Key-Remap settings.

## Getting Started

To get started with Key-Remap, follow these steps:

1. Clone the repository.
2. Run `make release` (or just `make`) in the folder you've just cloned.
3. run `keymap.exe` to activate the features.
4. If you want the program to run every time you start your PC, add the path to **keymap.exe** to the following path.

```bash
%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup
```

1. If you no longer want to use the remapping functionalities, find the program icon in the tray, click on it, and select the Quit option from the menu.

## Contributing

We welcome contributions from the open-source community to improve Key-Remap. If you have any ideas, bug reports, or feature requests, please submit them via the **issue tracker**. You can also contribute by submitting pull requests with code improvements or new features.

## License

Key-Remap is released under the [MIT License](https://opensource.org/licenses/MIT). Please review the [LICENSE](https://github.com/key-remap/key-remap/blob/main/LICENSE) file for more details.

## Acknowledgements

We would like to thank all the contributors who have helped make Key-Remap a better tool through their valuable contributions.