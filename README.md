# Windows logger application

This application records all mouse movements (only, no keyboard logging).
More precisely:
- x and y motions of the mouse
- x and y coordinates of the cursor (screen)
- mouse wheel state (if there is a wheel)

## How to use it ?

- Download the executable file **MouseLogger.exe** in the **Release** folder
- Put it in a folder
- Double click on **MouseLogger.exe** to launch it
- The application is now running and a new file has been created to store data

## Code

The application is a simple Windows App that uses the Win32 API to have a direct access to hardware.
All the code is contained in on C++ file **Main.cpp**.
Developed with Microsoft Visual Studio 2019.

### How a Windows application works ? (roughly speaking)

A Windows application needs two essential components: a window (yay) and procedure.

The window is used to receive all messages (events, for example when the user clicks on a button) from Windows.
This window is represented by the `WinMain` function in the code.

When the window receives a message, it transmits the message to the procedure.
The procedure contains the process to follow for each message (event) we want to handle.
The procedure is represented by the `WndProc` function, which contains a list (switch-case) of all the events we are interested in.

### How this application works ?

#### Window
This application runs as a service (it is invisible).
So the window should be invisible. To do that we can use a "Message-only window" whose name is pretty explicit: it is used only to receive messages and can not be used to interact with it (it is invisible actually).

To setup this kind of window we just have to specify `HWND_MESSAGE` as a parameter of the `CreateWindow` function (in `WinMain`).

#### Procedure
The procedure handles three messages:
- `WM_CREATE`
- `WM_DESTROY`
- `WM_INPUT`

`WM_CREATE` is received when the window is created. We create file for writing data in it and we specify which data we want to get from the Raw Input interface. To specify that we use the two following lines:
```
rid.usUsagePage = 1;
rid.usUsage = 2;
```
(`rid` for "Raw Input Device")

`rid.usUsagePage = 1` means that we want to register generic desktop controls
In this category "generic desktop controls" we can choose more precisely what we want with `usUsage`.
`rid.usUsage = 2` means that we want mouse data (6 is used to get keyboard data for example).

`WM_DESTROY` is received when the window is destroyed.

`WM_INPUT` is the core message of the application. It is received every time we have a new device input (mouse movement for example).
Here is how we get data:
```
raw->data.mouse.usButtonFlags,
raw->data.mouse.usButtonData,
raw->data.mouse.lLastX,
raw->data.mouse.lLastY,
```

`usButtonFlags` is set to "0400" (hexadecimal) when the mouse wheel is used, otherwise it is "0000".
`usButtonData` contains the mouse wheel "value" when the mouse wheel is used.
`lLastX` and `lLastY` represent x and y motions of the mouse
(cf. RAWINPUT links for more details)

To get the cursor position on the screen we also use the `GetCursorPos` function.

Another thing we do when we receive a `WM_INPUT` message is to check the time.
Indeed, we change the storage file every hour of inactivity, which means that if the mouse is not used for one hour, we will create a new file to store data. It avoids us having all the data in a single file and allows us to divide them into kind of "user sessions".


### Links that helped me to build this application
Win32 app documentation: https://docs.microsoft.com/en-us/windows/win32/  
Window app with Visual Studio: https://docs.microsoft.com/en-us/cpp/windows/walkthrough-creating-windows-desktop-applications-cpp?view=vs-2019  

Detailed explanation for a keylogger app: https://www.codeproject.com/Articles/297312/Minimal-Key-Logger-using-RAWINPUT  

RAWINPUT: https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawinput?redirectedfrom=MSDN  
RAWINPUT (mouse): https://docs.microsoft.com/en-us/windows/win32/api/winuser/ns-winuser-rawmouse  
GetCursorPos: https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getcursorpos?redirectedfrom=MSDN  
