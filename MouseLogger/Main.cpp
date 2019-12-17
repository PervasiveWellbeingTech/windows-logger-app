#define _CRT_SECURE_NO_DEPRECATE
#include <windows.h>
#include <windowsx.h>
#include <time.h>
#include <stdio.h>
#include <chrono>
#include <string>

using namespace std;

// prototype
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HWND hWnd, hActiveWindow, hPrevWindow;
UINT dwSize;
DWORD fWritten;
WCHAR keyChar;
HANDLE hFile;
//LPCWSTR fName = L"C:/Users/Hugo/source/repos/MouseLogger/Debug/mouse.log"; //GetTempPath 
//LPCWSTR fName = L"data/mouse.log"; //GetTempPath

/*
std::wstring name(L"put_time_here");
std::wstring concatted_stdstr = L"data/" + name + L".log";
LPCWSTR fName = concatted_stdstr.c_str();*/

UINT64 next_file_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
wchar_t tmp_buffer[50];
wchar_t * current_timestamp = _i64tow(next_file_timestamp, tmp_buffer, 10);

std::wstring name(current_timestamp);
std::wstring concatted_stdstr = L"data/" + name + L".log";
LPCWSTR fName = concatted_stdstr.c_str();

UINT64 new_file_period = 3600000;  // in milliseconds

INT len;
CHAR p_window_title[256] = "";
CHAR active_window_title[256] = "";
CHAR* tmp_buf;
CHAR tmp_buf_len = 0;
RAWINPUTDEVICE rid;

POINT pt;                  // cursor location  

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	next_file_timestamp = next_file_timestamp + new_file_period;

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
		UINT64 current_timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

		if (current_timestamp > next_file_timestamp) {
			wchar_t tmp_buffer[50];
			wchar_t* current_timestamp_str = _i64tow(next_file_timestamp, tmp_buffer, 10);

			std::wstring name(current_timestamp_str);
			std::wstring concatted_stdstr = L"data/" + name + L".log";
			fName = concatted_stdstr.c_str();

			CloseHandle(hFile);
			hFile = CreateFile(fName, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			if (hFile == INVALID_HANDLE_VALUE) {
				PostQuitMessage(0);
				break;
			}
		}
		next_file_timestamp = current_timestamp + new_file_period;

		PRAWINPUT raw = (PRAWINPUT)lpb;
		//UINT Event;
		CHAR wt[300] = "";

		/*
		sprintf(wt, "Mouse: usFlags=%04x ulButtons=%04x usButtonFlags=%04x usButtonData=%04x ulRawButtons=%04x lLastX=%04x lLastY=%04x ulExtraInformation=%04x\r\n",
			raw->data.mouse.usFlags,
			raw->data.mouse.ulButtons,
			raw->data.mouse.usButtonFlags,
			raw->data.mouse.usButtonData,
			raw->data.mouse.ulRawButtons,
			raw->data.mouse.lLastX,
			raw->data.mouse.lLastY,
			raw->data.mouse.ulExtraInformation);
		*/

		//UINT64 now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

		/*
		sprintf(wt, "%lld,%04x,%04x,%04x,%ld,%ld,%ld,%ld,%04x\r\n",
			now,
			raw->data.mouse.usFlags,
			raw->data.mouse.ulButtons,
			raw->data.mouse.usButtonFlags,
			raw->data.mouse.usButtonData,
			raw->data.mouse.ulRawButtons,
			raw->data.mouse.lLastX,
			raw->data.mouse.lLastY,
			raw->data.mouse.ulExtraInformation);
		*/

		GetCursorPos(&pt);

		sprintf(wt, "%lld,%04x,%04x,%ld,%ld,%ld,%ld\r\n",
			current_timestamp,
			raw->data.mouse.usButtonFlags,
			raw->data.mouse.usButtonData,
			raw->data.mouse.lLastX,
			raw->data.mouse.lLastY,
			pt.x,
			pt.y);

		WriteFile(hFile, wt, strlen(wt), &fWritten, 0);
		delete[] lpb;	// free this now
		return 0;

	}// end case WM_INPUT

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);

	}// end switch

	return 0;
}// end WndProc