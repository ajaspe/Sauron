#include <windows.h>
#include <Psapi.h>
#include <stdio.h>
#include <regex>

#include "resource.h"

#include "activity_sampler.h"

#define ID_TRAY_APP_ICON    1001
#define ID_TRAY_EXIT        1002
#define WM_SYSICON          (WM_USER + 1)

const LPSTR g_szClassName = "myWindowClass";
const LPSTR tip = "Este es el tip";

HWND hMain, hEdit;
HMENU hMenuRightClick;
NOTIFYICONDATA notifyIconData;
ActivitySampler activitySampler;

void appendText(HWND hEditWnd, LPCTSTR Text)
{
	int idx = GetWindowTextLength(hEditWnd);
	SendMessage(hEditWnd, EM_SETSEL, (WPARAM)idx, (LPARAM)idx);
	SendMessage(hEditWnd, EM_REPLACESEL, 0, (LPARAM)Text);
}

void log() {
	// Get actve window
	HWND hActiveWindow;
	hActiveWindow = GetForegroundWindow();
	
	// Get active window title
	CHAR wnd_title[1024];
	GetWindowText(hActiveWindow, wnd_title, sizeof(wnd_title));

	std::string result;
	std::cmatch match;
	std::regex re("^(.+) - Microsoft Visual Studio");
	if (std::regex_search(wnd_title, match, re) && match.size() > 1) {
		result = match.str(1);
	}
	else {
		result = std::string("NO ENCONTRADO");
	}

	// Get active window process
	DWORD pid;
	GetWindowThreadProcessId(hActiveWindow, &pid);
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	CHAR processName[256], processFileName[256];
	GetModuleFileNameEx(hProcess, NULL, processName, 256);
	_splitpath(processName, NULL, NULL, processFileName, NULL);

	// Get last input for IDLE computing
	LASTINPUTINFO lii;
	lii.cbSize = sizeof(LASTINPUTINFO);
	GetLastInputInfo(&lii);
	DWORD te = GetTickCount();
	float idleSeconds = float(te - lii.dwTime) / 1000.0f;

	// Get Local Time for timestamp
	SYSTEMTIME st;
	GetLocalTime(&st);

	
	CHAR mess[256];
	if(idleSeconds < 10.0)
		sprintf(mess, "%i/%i/%i %i:%i:%i.%i\t%s\t%s\t%.2f\n",
			st.wDay, st.wMonth, st.wYear, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
			processFileName, result.c_str(), idleSeconds);
	else 
		sprintf(mess, "--- IDLE --- (for %.1f seconds)\n", idleSeconds);
	
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
	{
		HFONT hfDefault;

		hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "",
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
			0, 0, 200, 100, hwnd, NULL, GetModuleHandle(NULL), NULL);
		
		if (hEdit == NULL) MessageBox(hwnd, "Could not create edit box.", "Error", MB_OK | MB_ICONERROR);

		hfDefault = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
		SendMessage(hEdit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));

		hMenuRightClick = CreatePopupMenu();
		AppendMenu(hMenuRightClick, MF_STRING, ID_TRAY_EXIT, TEXT("Exit"));


		UINT_PTR timer_id = SetTimer(hwnd, 1, 1000, NULL);

	}
	break;
	case WM_SIZE:
	{
		RECT rcClient;
		GetClientRect(hwnd, &rcClient);
		SetWindowPos(hEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
	}
	break;
	case WM_ACTIVATE:
		Shell_NotifyIcon(NIM_ADD, &notifyIconData);
		break;
	
	//Our user defined WM_SYSICON message.
	case WM_SYSICON:
	{
		switch (wParam)
		{
		case ID_TRAY_APP_ICON:
			SetForegroundWindow(hMain);
			break;
		}


		if (lParam == WM_LBUTTONUP)
		{
			ShowWindow(hMain, SW_SHOW);
		}
		else if (lParam == WM_RBUTTONDOWN)
		{
			// Get current mouse position.
			POINT curPoint;
			GetCursorPos(&curPoint);
			SetForegroundWindow(hMain);

			// TrackPopupMenu blocks the app until TrackPopupMenu returns

			UINT clicked = TrackPopupMenu(hMenuRightClick, TPM_RETURNCMD | TPM_NONOTIFY, curPoint.x, curPoint.y, 0, hwnd, NULL);



			SendMessage(hwnd, WM_NULL, 0, 0); // send benign message to window to make sure the menu goes away.
			if (clicked == ID_TRAY_EXIT)
			{
				// quit the application.
				Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
				PostQuitMessage(0);
			}
		}
	}
	break;

	// intercept the hittest message..
	case WM_NCHITTEST:
	{
		UINT uHitTest = (UINT)DefWindowProc(hwnd, WM_NCHITTEST, wParam, lParam);
		if (uHitTest == HTCLIENT)
			return HTCAPTION;
		else
			return uHitTest;
	}

	case WM_TIMER:
		activitySampler.takeSample();
		appendText(hEdit, activitySampler.getLastSampleString().c_str());
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	
	case WM_SYSCOMMAND:
		/*In WM_SYSCOMMAND messages, the four low-order bits of the wParam parameter
		are used internally by the system. To obtain the correct result when testing the value of wParam,
		an application must combine the value 0xFFF0 with the wParam value by using the bitwise AND operator.*/

		switch (wParam & 0xFFF0)
		{
		case SC_MINIMIZE:
		//case SC_CLOSE:
			ShowWindow(hMain, SW_HIDE);
			return 0;
			break;
		}

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	HICON hIcon = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_MYICON), IMAGE_ICON, 128, 128, LR_DEFAULTCOLOR);
	HICON hIconSm = (HICON) LoadImage(hInstance, MAKEINTRESOURCE(IDI_MYICON), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);

	WNDCLASSEX wc;
	MSG Msg;

	//Registering the Window Class
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm = hIconSm;

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, "Window Registration Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// Step 2: Creating the Window
	hMain = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		g_szClassName,
		"Sauron EYE (monitoring)",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 400, 200,
		NULL, NULL, hInstance, NULL);

	if (hMain == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!",
			MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	memset(&notifyIconData, 0, sizeof(NOTIFYICONDATA));
	notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
	notifyIconData.hWnd = hMain;
	notifyIconData.uID = ID_TRAY_APP_ICON;
	notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	notifyIconData.uCallbackMessage = WM_SYSICON; //Set up our invented Windows Message
	notifyIconData.hIcon = hIconSm;// (HICON)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ICO1));
	strcpy(notifyIconData.szTip, tip);

	ShowWindow(hMain, nCmdShow);
	UpdateWindow(hMain);

	// Step 3: The Message Loop
	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return (int)Msg.wParam;
}
