<div align="center">

# TinyShell

### A Small Windows shell, built from the API up.

A native shell (~4x less RAM usage when compared to Windows Powershell).

![Language: C](https://img.shields.io/badge/language-C-00599C?style=flat-square)
![Platform: Windows](https://img.shields.io/badge/platform-Windows-0078D4?style=flat-square)
![API: Win32](https://img.shields.io/badge/API-Win32-5C2D91?style=flat-square)
![Status: Experimental](https://img.shields.io/badge/status-experimental-E9A23B?style=flat-square)

[Getting started](#getting-started) · [Try it](#try-it) · [How it works](#how-it-works) · [Explore the code](#explore-the-code)

</div>

---

TinyShell is a lightweight, native Windows GUI written in C. It has all the features of a normal graphical shell: window messages, keyboard input, process creation, anonymous pipes, text rendering, and manually managed command history.

The interface uses Win32 controls and GDI directly. Submitting an executable path starts a worker thread, launches the program, and collects its output for display in the window.

TinyShell currently supports command parsing and normal shell built-ins.

## Getting started

### Requirements

- Windows
- MinGW-w64 GCC with Windows headers and libraries
- GNU Make if using the included `Makefile`

The Makefile defaults to `x86_64-w64-mingw32-gcc` and links against `gdi32`, `comctl32`, and `shlwapi`.

### Build

Clone the repository and enter its directory:

```sh
git clone https://github.com/deanliang06/TinyShell.git
cd TinyShell
```

Build with GNU Make from a shell where the compiler is on `PATH`:

```sh
make
```

If your MinGW compiler is named `gcc`, override the default:

```sh
make CC=gcc
```

You can also build directly without Make:

```sh
gcc -municode window.c memory.c display.c handleText.c -o terminal.exe -lgdi32 -lcomctl32 -lshlwapi
```

### Launch

From PowerShell in the project directory:

```powershell
.\terminal.exe
```

## Try it

1. Launch `terminal.exe`.
2. Enter the path to a Windows executable in the input field.
3. Press **Enter** to run it.
4. After execution finishes, inspect the captured output and enter another path.

For example, on a typical Windows installation, enter:

```text
C:\Windows\System32\hostname.exe
```

This program prints the computer’s host name. Relative executable paths are resolved from TinyShell’s working directory.

`window.c` connects user input to execution. `handleText.c` validates and launches the executable, then passes captured bytes to the output buffer in `memory.c`. The display layer paints the resulting history using GDI.

## Explore the code

| File | Responsibility |
| --- | --- |
| [`window.c`](window.c) | Application entry point, window messages, input control, and worker thread |
| [`handleText.c`](handleText.c) | Executable validation, process creation, pipe reads, and error messages |
| [`memory.c`](memory.c) | History allocation, output buffering, and working-directory helpers |
| [`display.c`](display.c) | History rendering and input-field placement |
| [`types.h`](types.h) | Shared message, history, and window configuration structures |
| [`Makefile`](Makefile) | Object compilation and Windows library linking |
| [`test/main.c`](test/main.c) | Manual child-process fixture that prints output and deliberately dereferences a null pointer |