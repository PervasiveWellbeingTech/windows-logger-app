#define _CRT_SECURE_NO_DEPRECATE
#include <windows.h>
#include <windowsx.h>
#include <time.h>
#include <stdio.h>
#include <chrono>
#include <string>
#include <wintoastlib.h>

#define INFO_BUFFER_SIZE 32767

using namespace std;
using namespace WinToastLib;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HWND hWnd, hActiveWindow, hPrevWindow;
UINT dwSize;
DWORD fWritten;
WCHAR keyChar;
LPCWSTR computerName;
UINT64 newFilePeriod = 3600000;  // in milliseconds

UINT64 nextFileTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
wchar_t tempBuffer[50];
wchar_t * currentTimestamp = _i64tow(nextFileTimestamp, tempBuffer, 10);

std::wstring computerFolderName = L"data/computer/";
std::wstring folderName = L"data/raw_input/";
std::wstring name(currentTimestamp);
std::wstring concattedStdstr = folderName + name + L".log";
LPCWSTR fName = concattedStdstr.c_str();

INT len;
CHAR pWindowTitle[256] = "";
CHAR activeWindowTitle[256] = "";
CHAR* tmpBuf;
CHAR tmpBufLen = 0;
RAWINPUTDEVICE rid;

POINT cursorPoint;

class FileManager {
public:
	std::wstring computerFolderName = L"data/computer/";
	std::wstring rawInputFolderName = L"data/raw_input/";
	LPCWSTR fileName = L"0000000000000";
	HANDLE hFile;

	LPCWSTR fileNameA = L"data/computer/test_a.log";
	LPCWSTR fileNameB = L"data/computer/test_b.log";
	HANDLE aFile;
	HANDLE bFile;

	LPCWSTR computerFileName;
	LPCWSTR rawInputFileName;
	HANDLE computerFile;
	HANDLE rawInputFile;
};

FileManager fileManager;

int openFile(LPCWSTR fileName, LPCWSTR folderName) {
	// Open log file for writing
	if (CreateDirectory(folderName, NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
		fileManager.hFile = CreateFile(fileName, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	}

	if (fileManager.hFile == INVALID_HANDLE_VALUE) {
		PostQuitMessage(0);
		return -1;
	}

	return 0;
}

UINT64 getCurrentTimestamp() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::wstring getComputerName() {
	TCHAR infoBuf[INFO_BUFFER_SIZE];
	DWORD bufCharCount = INFO_BUFFER_SIZE;

	GetComputerName(infoBuf, &bufCharCount);  // Get the name of the computer and store it in infoBuf
	return infoBuf;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	nextFileTimestamp = nextFileTimestamp + newFilePeriod;

	fileManager.aFile = CreateFile(fileManager.fileNameA, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	fileManager.bFile = CreateFile(fileManager.fileNameB, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	CHAR wta[300] = "";
	sprintf(wta, "%lld,bla bla test a\r\n", getCurrentTimestamp());
	WriteFile(fileManager.aFile, wta, strlen(wta), &fWritten, 0);

	CHAR wtb[300] = "";
	sprintf(wtb, "%lld,bla bla test b\r\n", getCurrentTimestamp());
	WriteFile(fileManager.bFile, wtb, strlen(wtb), &fWritten, 0);

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

class CustomHandler : public IWinToastHandler {
public:

	void toastActivated() const {
		OutputDebugString(L"The user clicked in this toast\n");

		std::wstring computerFileName = computerFolderName + getComputerName() + L".log";
		//openFile(fileName.c_str(), L"data/computer/");

		if (CreateDirectory(computerFolderName.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
			fileManager.hFile = CreateFile(computerFileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			SetFilePointer(fileManager.hFile, 0, NULL, FILE_END);  // Set the cursor at the end of the file

			CHAR wt[300] = "";
			sprintf(wt, "%lld,toast activated\r\n", getCurrentTimestamp());
			WriteFile(fileManager.hFile, wt, strlen(wt), &fWritten, 0);
			CloseHandle(fileManager.hFile);
		}

		if (CreateDirectory(fileManager.rawInputFolderName.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
			OutputDebugString(fileManager.fileName);
			OutputDebugString(L"\n");
			fileManager.hFile = CreateFile(fileManager.fileName, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			SetFilePointer(fileManager.hFile, 0, NULL, FILE_END);  // Set the cursor at the end of the file

			CHAR wt[300] = "";
			sprintf(wt, "%lld,toast activated AGAAAIN\r\n", getCurrentTimestamp());
			WriteFile(fileManager.hFile, wt, strlen(wt), &fWritten, 0);
			OutputDebugString(fileManager.fileName);
			OutputDebugString(L"\n");
		}
		else {
			OutputDebugString(L"NUL...\n");
		}

		CHAR wtComputer[300] = "";
		sprintf(wtComputer, "%lld,toast activated\r\n", getCurrentTimestamp());
		WriteFile(fileManager.computerFile, wtComputer, strlen(wtComputer), &fWritten, 0);
		ShellExecute(NULL, L"open", L"https://www.qualtrics.com/lp/survey-platform", nullptr, nullptr, SW_SHOWNORMAL);
	}

	void toastActivated(int actionIndex) const {
		std::wcout << L"The user clicked on button #" << actionIndex << L" in this toast" << std::endl;
	}

	void toastFailed() const {
		std::wcout << L"Error showing current toast" << std::endl;
	}
	void toastDismissed(WinToastDismissalReason state) const {
		switch (state) {
		case UserCanceled:
			std::wcout << L"The user dismissed this toast" << std::endl;
			break;
		case ApplicationHidden:
			std::wcout << L"The application hid the toast using ToastNotifier.hide()" << std::endl;
			break;
		case TimedOut:
			std::wcout << L"The toast has timed out" << std::endl;
			break;
		default:
			std::wcout << L"Toast not activated" << std::endl;
			break;
		}
	}
};

// WndProc is called when a window message is sent to the handle of the window
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {

	case WM_CREATE: {

		// Setup the "computer file" which contains general information about computer, users, sessions
		TCHAR infoBuf[INFO_BUFFER_SIZE];
		DWORD bufCharCount = INFO_BUFFER_SIZE;

		GetComputerName(infoBuf, &bufCharCount);  // Get the name of the computer and store it in infoBuf
		std::wstring computerName(infoBuf);
		std::wstring concattedStdstr = computerFolderName + computerName + L".log";
		fileManager.computerFileName = concattedStdstr.c_str();

		// The opened file is the object fileManager.hFile
		/*
		if (openFile(computerfName, computerFolderName.c_str()) == -1) {
			break;
		}
		*/
		if (CreateDirectory(fileManager.computerFolderName.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
			fileManager.computerFile = CreateFile(fileManager.computerFileName, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		}

		if (fileManager.computerFile == INVALID_HANDLE_VALUE) {
			PostQuitMessage(0);
			return -1;
		}
		SetFilePointer(fileManager.computerFile, 0, NULL, FILE_END);  // Set the cursor at the end of the file
		
		UINT64 startTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

		CHAR wt[300] = "";
		sprintf(wt, "Computer name: %ls\r\n%lld,connection\r\n",
			computerName.c_str(),
			startTimestamp);
		WriteFile(fileManager.computerFile, wt, strlen(wt), &fWritten, 0);

		// Setup the toast
		if (!WinToast::isCompatible()) {
			std::wcout << L"Error, your system in not supported!" << std::endl;
		}

		WinToast::instance()->setAppName(L"Stanford PWT Lab");
		
		// companyName, productName, subProduct, versionInformation
		const auto aumi = WinToast::configureAUMI(L"StanfordPWTLab", L"windows_logger", L"windows_logger", L"20200130");
		WinToast::instance()->setAppUserModelId(aumi);

		if (!WinToast::instance()->initialize()) {
			std::wcout << L"Error, could not initialize the lib!" << std::endl;
		}

		CustomHandler* handler = new CustomHandler;
		WinToastTemplate templ = WinToastTemplate(WinToastTemplate::Text02);
		templ.setTextField(L"Stanford PWT Lab", WinToastTemplate::FirstLine);
		templ.setTextField(L"If you have the time to answer a survey, click on this notification", WinToastTemplate::SecondLine);

		if (!WinToast::instance()->showToast(templ, handler)) {
			std::wcout << L"Error: Could not launch your toast notification!" << std::endl;
		}
		else {
			CHAR wt[300] = "";
			sprintf(wt, "%lld,first survey toasted\r\n", getCurrentTimestamp());
			WriteFile(fileManager.computerFile, wt, strlen(wt), &fWritten, 0);
			//CloseHandle(fileManager.computerFile);  // Close the file
		}

		nextFileTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		wchar_t tempBuffer[50];
		wchar_t* currentTimestamp = _i64tow(nextFileTimestamp, tempBuffer, 10);

		//folderName = L"data/raw_input/";
		/*
		name = currentTimestamp;
		concattedStdstr = folderName + name + L".log";
		fName = concattedStdstr.c_str();
		*/

		// open log file for writing
		/*
		if (CreateDirectory(folderName.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
			fileManager.hFile = CreateFile(fName, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		}
		
		if (fileManager.hFile == INVALID_HANDLE_VALUE) {
			PostQuitMessage(0);
			break;
		}
		SetFilePointer(fileManager.hFile, 0, NULL, FILE_END);  // Set the cursor at the end of the file

		fileManager.fileName = fName;
		*/

		name = currentTimestamp;
		concattedStdstr = fileManager.rawInputFolderName + name + L".log";
		fileManager.rawInputFileName = concattedStdstr.c_str();

		if (CreateDirectory(fileManager.computerFolderName.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
			fileManager.rawInputFile = CreateFile(fileManager.rawInputFileName, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		}

		if (fileManager.rawInputFile == INVALID_HANDLE_VALUE) {
			PostQuitMessage(0);
			return -1;
		}
		SetFilePointer(fileManager.rawInputFile, 0, NULL, FILE_END);  // Set the cursor at the end of the file

		// register interest in raw data
		rid.dwFlags = RIDEV_NOLEGACY | RIDEV_INPUTSINK;	
		rid.usUsagePage = 1;
		rid.usUsage = 2;
		rid.hwndTarget = hWnd;
		RegisterRawInputDevices(&rid, 1, sizeof(rid));
		break;
	}// end case WM_CREATE

	case WM_DESTROY: {
		FlushFileBuffers(fileManager.hFile);
		CloseHandle(fileManager.hFile);
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
		
		// Check the time to see if we have to create a new file or not,
		// because every newFilePeriod-milliseconds we create a new file (to avoid big files)
		// TODO: update with new fileManager
		UINT64 currentTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

		if (currentTimestamp > nextFileTimestamp) {
			wchar_t tempBuffer[50];
			wchar_t* currentTimestampStr = _i64tow(nextFileTimestamp, tempBuffer, 10);

			std::wstring name(currentTimestampStr);
			std::wstring concattedStdstr = L"data/raw_input/" + name + L".log";
			fName = concattedStdstr.c_str();

			CloseHandle(fileManager.hFile);
			if (CreateDirectory(folderName.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
				fileManager.hFile = CreateFile(fileManager.fileName, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			}

			if (fileManager.hFile == INVALID_HANDLE_VALUE) {
				PostQuitMessage(0);
				break;
			}

			//fileManager.fileName = L"new_test_file_test";

			OutputDebugString(L"MANAAAAAAAGER\n");
			OutputDebugString(fileManager.fileName);
			OutputDebugString(L"\n");

			OutputDebugString(L"AM I HEREEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE ?\n");
			OutputDebugString(fName);
			OutputDebugString(L"\n");
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

		wchar_t* displayTimestamp = _i64tow(currentTimestamp, tempBuffer, 10);

		OutputDebugString(displayTimestamp);
		OutputDebugString(L"\n");
		OutputDebugString(fileManager.fileName);
		OutputDebugString(L"\n");
		
		WriteFile(fileManager.rawInputFile, wt, strlen(wt), &fWritten, 0);

		delete[] lpb;

		return 0;
	}// end case WM_INPUT

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}// end switch

	return 0;
}// end WndProc