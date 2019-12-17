#define _CRT_SECURE_NO_DEPRECATE
#include <windows.h>
#include <windowsx.h>
#include <time.h>
#include <stdio.h>
#include <chrono>
#include <string>

using namespace std;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HWND hWnd, hActiveWindow, hPrevWindow;
UINT dwSize;
DWORD fWritten;
WCHAR keyChar;
HANDLE hFile;

UINT64 nextFileTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
wchar_t tempBuffer[50];
wchar_t * currentTimestamp = _i64tow(nextFileTimestamp, tempBuffer, 10);

std::wstring name(currentTimestamp);
std::wstring concattedStdstr = L"data/" + name + L".log";
LPCWSTR fName = concattedStdstr.c_str();

UINT64 newFilePeriod = 3600000;  // in milliseconds

INT len;
CHAR pWindowTitle[256] = "";
CHAR activeWindowTitle[256] = "";
CHAR* tmpBuf;
CHAR tmpBufLen = 0;
RAWINPUTDEVICE rid;

POINT cursorPoint;  

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	nextFileTimestamp = nextFileTimestamp + newFilePeriod;

	MSG msg = { 0 };
	WNDCLASS wc = { 0 };

	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"Message-Only Window";

	RegisterClass(&wc);
	hWnd = CreateWindow(wc.lpszClassName, NULL, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);

	while (GetMessage(&msg, hWnd, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	//return msg.wParam;
	return 0;
}

// WndProc is called when a window message is sent to the handle of the window
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {

	case WM_CREATE: {
		// open log file for writing
		hFile = CreateFile(fName, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFile == INVALID_HANDLE_VALUE) {
			PostQuitMessage(0);
			break;
		}

		// register interest in raw data
		rid.dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK;	
		rid.usUsagePage = 1;
		rid.usUsage = 2;
		rid.hwndTarget = hWnd;
		RegisterRawInputDevices(&rid, 1, sizeof(rid));
		break;
	}// end case WM_CREATE

	case WM_DESTROY: {
		FlushFileBuffers(hFile);
		CloseHandle(hFile);
		PostQuitMessage(0);
		break;
	}// end case WM_DESTROY

	case WM_INPUT: {
		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER)) == -1) {
			PostQuitMessage(0);
			break;
		}

		LPBYTE lpb = new BYTE[dwSize];
		if (lpb == NULL) {
			PostQuitMessage(0);
			break;
		}

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
			delete[] lpb;
			PostQuitMessage(0);
			break;
		}
		
		// Check the time to see if we have to create a new file or not
		UINT64 currentTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

		if (currentTimestamp > nextFileTimestamp) {
			wchar_t tempBuffer[50];
			wchar_t* currentTimestampStr = _i64tow(nextFileTimestamp, tempBuffer, 10);

			std::wstring name(currentTimestampStr);
			std::wstring concattedStdstr = L"data/" + name + L".log";
			fName = concattedStdstr.c_str();

			CloseHandle(hFile);
			hFile = CreateFile(fName, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			if (hFile == INVALID_HANDLE_VALUE) {
				PostQuitMessage(0);
				break;
			}
		}
		nextFileTimestamp = currentTimestamp + newFilePeriod;

		PRAWINPUT raw = (PRAWINPUT)lpb;
		CHAR wt[300] = "";
		GetCursorPos(&cursorPoint);

		sprintf(wt, "%lld,%04x,%04x,%ld,%ld,%ld,%ld\r\n",
			currentTimestamp,
			raw->data.mouse.usButtonFlags,
			raw->data.mouse.usButtonData,
			raw->data.mouse.lLastX,
			raw->data.mouse.lLastY,
			cursorPoint.x,
			cursorPoint.y);

		WriteFile(hFile, wt, strlen(wt), &fWritten, 0);
		delete[] lpb;  // free this now
		return 0;

	}// end case WM_INPUT

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);

	}// end switch

	return 0;
}// end WndProc