#include <Windows.h>

LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	const wchar_t lpwClassName[] = L"JAKUBKUCZKOWIAK_DLLINJECTOR";
	const wchar_t lpwWindowName[] = L"Jakub Kuczkowiak's DLL Injector";

	WNDCLASS wc = { };
	wc.lpszClassName = lpwClassName;
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WindowProc;

	RegisterClass(&wc);
	
	int nWidth = 445, nHeight = 300;
	int nScreenX = GetSystemMetrics(SM_CXSCREEN), nScreenY = GetSystemMetrics(SM_CYSCREEN);

	HWND hWnd = CreateWindowEx(0, lpwClassName, lpwWindowName,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, // Window style.
		(nScreenX - nWidth) / 2, (nScreenY - nHeight) / 2, nWidth, nHeight,
		NULL, NULL, hInstance, NULL);

	if (hWnd == NULL)
	{
		return 0;
	}

	ShowWindow(hWnd, nCmdShow);

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

enum Handler
{
	btnApplication,
	btnDynamicLinkLibrary,

	btnInject,
	btnAddItem,
	btnUpdateItem,
	btnDeleteItem,

	btnReorderUp,
	btnReorderDown,

	txtProcessName,
	txtDLLName,
	txtItemName,

	cmbItems
};

HWND hTXT_ExecutableLocation;
HWND hTXT_DllLocation;

HWND hTXT_ItemName;
HWND hCMB_Items;

HWND hBTN_ReorderUp;
HWND hBTN_ReorderDown;

bool DoesFileExist(LPCWSTR lpwFileLocation)
{
	HANDLE handle = CreateFile(lpwFileLocation, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	CloseHandle(handle);

	return ((int)handle != -1);
}

const LPCWSTR MAIN_REGISTRY_NAME = L"Software\\Jakub Kuczkowiak's DLL Injector";

void SwapItemsInRegistry(unsigned int index1, unsigned int index2)
{
	HKEY hKey, hKeyItem1, hKeyItem2, hKeyTemporary;
	RegCreateKey(HKEY_CURRENT_USER, MAIN_REGISTRY_NAME, &hKey);

	wchar_t lpwItemName[16] = { };
	wsprintf(lpwItemName, L"Item %d", index1);

	RegCreateKey(hKey, lpwItemName, &hKeyItem1); // first
	RegCreateKey(hKey, L"Temporary Item", &hKeyTemporary); // temporary
	wsprintf(lpwItemName, L"Item %d", index2);
	RegCreateKey(hKey, lpwItemName, &hKeyItem2); // second

	RegCopyTree(hKeyItem1, NULL, hKeyTemporary);
	RegCopyTree(hKeyItem2, NULL, hKeyItem1);
	RegCopyTree(hKeyTemporary, NULL, hKeyItem2);

	RegDeleteTree(hKey, L"Temporary Item");

	RegCloseKey(hKeyItem1);
	RegCloseKey(hKeyItem2);
	RegCloseKey(hKeyTemporary);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_CREATE:
		{
			HFONT hFont = CreateFont(18, 0, 0, 0, 0, false, false, false, 0, 0, 0, 0, 0, L"Segoe UI");
			HKEY hKey;
			DWORD size;
			RegCreateKey(HKEY_CURRENT_USER, MAIN_REGISTRY_NAME, &hKey);

			HWND hButton = CreateWindow(L"BUTTON", L"App", WS_VISIBLE | WS_CHILD, 10, 10, 50, 25, hWnd, (HMENU)Handler::btnApplication, (HINSTANCE)(GetWindowLongPtr(hWnd, GWLP_HINSTANCE)), NULL);
			SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);
			
			hButton = CreateWindow(L"BUTTON", L"DLL", WS_VISIBLE | WS_CHILD, 10, 40, 50, 25, hWnd, (HMENU)Handler::btnDynamicLinkLibrary, (HINSTANCE)(GetWindowLongPtr(hWnd, GWLP_HINSTANCE)), NULL);
			SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);

			hTXT_ExecutableLocation = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"Application path here.",
				ES_MULTILINE | ES_AUTOHSCROLL | WS_CHILD | WS_VISIBLE | WS_BORDER, 70, 10, 350, 25, hWnd, (HMENU)Handler::txtProcessName, (HINSTANCE)(GetWindowLongPtr(hWnd, GWLP_HINSTANCE)), NULL);
			SendMessage(hTXT_ExecutableLocation, WM_SETFONT, (WPARAM)hFont, TRUE);

			hTXT_DllLocation = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"DLL path here.",
				ES_MULTILINE | ES_AUTOHSCROLL | WS_CHILD | WS_VISIBLE | WS_BORDER, 70, 40, 350, 25, hWnd, (HMENU)Handler::txtDLLName, (HINSTANCE)(GetWindowLongPtr(hWnd, GWLP_HINSTANCE)), NULL);
			SendMessage(hTXT_DllLocation, WM_SETFONT, (WPARAM)hFont, TRUE);

			hButton = CreateWindow(L"BUTTON", L"Inject", WS_VISIBLE | WS_CHILD, 10, 70, 410, 25, hWnd, (HMENU)Handler::btnInject, (HINSTANCE)(GetWindowLongPtr(hWnd, GWLP_HINSTANCE)), NULL);
			SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);

			hTXT_ItemName = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"Item name.",
				ES_MULTILINE | ES_AUTOHSCROLL | WS_CHILD | WS_VISIBLE | WS_BORDER, 10, 130, 290, 25, hWnd, (HMENU)Handler::txtItemName, (HINSTANCE)(GetWindowLongPtr(hWnd, GWLP_HINSTANCE)), NULL);
			SendMessage(hTXT_ItemName, WM_SETFONT, (WPARAM)hFont, TRUE);

			hButton = CreateWindowEx(0, L"BUTTON", L"Add Item", WS_VISIBLE | WS_CHILD, 330, 130, 90, 35, hWnd,
				(HMENU)Handler::btnAddItem, (HINSTANCE)(GetWindowLongPtr(hWnd, GWLP_HINSTANCE)), NULL);
			SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);

			hButton = CreateWindowEx(0, L"BUTTON", L"Update Item", WS_VISIBLE | WS_CHILD, 330, 170, 90, 35, hWnd,
				(HMENU)Handler::btnUpdateItem, (HINSTANCE)(GetWindowLongPtr(hWnd, GWLP_HINSTANCE)), NULL);
			SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);

			hButton = CreateWindowEx(0, L"BUTTON", L"Delete Item", WS_VISIBLE | WS_CHILD, 330, 210, 90, 35, hWnd,
				(HMENU)Handler::btnDeleteItem, (HINSTANCE)(GetWindowLongPtr(hWnd, GWLP_HINSTANCE)), NULL);
			SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);

			hCMB_Items = CreateWindow(L"COMBOBOX", L"Combo box.",
				CBS_HASSTRINGS | WS_VSCROLL | WS_VISIBLE | WS_CHILD, 10, 160, 290, 100, hWnd, (HMENU)Handler::cmbItems, (HINSTANCE)(GetWindowLongPtr(hWnd, GWLP_HINSTANCE)), NULL);
			SendMessage(hCMB_Items, WM_SETFONT, (WPARAM)hFont, TRUE);

			hFont = CreateFont(22, 0, 0, 0, 700, false, false, false, 0, 0, 0, 0, 0, L"Calibri");
			hBTN_ReorderUp = CreateWindowEx(0, L"BUTTON", L"↑", WS_DISABLED | WS_VISIBLE | WS_CHILD, 305, 185, 20, 20, hWnd,
				(HMENU)Handler::btnReorderUp, (HINSTANCE)(GetWindowLongPtr(hWnd, GWLP_HINSTANCE)), NULL);
			SendMessage(hBTN_ReorderUp, WM_SETFONT, (WPARAM)hFont, TRUE);

			hBTN_ReorderDown = CreateWindowEx(0, L"BUTTON", L"↓", WS_DISABLED | WS_VISIBLE | WS_CHILD, 305, 235, 20, 20, hWnd,
				(HMENU)Handler::btnReorderDown, (HINSTANCE)(GetWindowLongPtr(hWnd, GWLP_HINSTANCE)), NULL);
			SendMessage(hBTN_ReorderDown, WM_SETFONT, (WPARAM)hFont, TRUE);

			// TODO: You also must select last selected item. Allow user to choose, whenever he wants or not that option.
			int i;
			for (i = 0;; ++i)
			{
				wchar_t* data;

				wchar_t lpwItem[16] = { };
				wsprintf(lpwItem, L"Item %d", i);

				HKEY currentKey;
				if (RegOpenKey(hKey, lpwItem, &currentKey))
					break;
				
				RegQueryValueEx(currentKey, L"Item name", 0, NULL, NULL, &size);
				if (size)
				{
					data = new wchar_t[size];
					RegQueryValueEx(currentKey, L"Item name", 0, NULL, (unsigned char*)data, &size);

					SendMessage(hCMB_Items, CB_ADDSTRING, NULL, (LPARAM)data);
					delete[] data;
				}
			}
		}
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case Handler::btnApplication:
				{
					wchar_t* lpwFile = new wchar_t[MAX_PATH];
					ZeroMemory(lpwFile, MAX_PATH);

					OPENFILENAME ofn = { };
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hWnd;
					ofn.lpstrTitle = L"Choose an application, to which DLL has to be injected.";
					ofn.lpstrFilter = L"Applications\0*.exe;*.ex\0";
					ofn.lpstrFile = lpwFile;
					ofn.lpstrDefExt = L".exe";
					ofn.nMaxFile = MAX_PATH;
					ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

					if (GetOpenFileName(&ofn))
						SetWindowText(hTXT_ExecutableLocation, lpwFile);

					delete[] lpwFile;
				}
				break;

			case Handler::btnDynamicLinkLibrary:
				{
					wchar_t* lpwFile = new wchar_t[MAX_PATH];
					ZeroMemory(lpwFile, MAX_PATH);

					OPENFILENAME ofn = { };
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hWnd;
					ofn.lpstrTitle = L"Choose a dynamic link library file, which has to be injected into an application.";
					ofn.lpstrFilter = L"Dynamic Link Library\0*.dll\0";
					ofn.lpstrFile = lpwFile;
					ofn.lpstrDefExt = L".dll";
					ofn.nMaxFile = MAX_PATH;
					ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

					if (GetOpenFileName(&ofn))
						SetWindowText(hTXT_DllLocation, lpwFile);

					delete[] lpwFile;
				}
				break;

			case Handler::btnInject:
				{					
					int nAppTextLength = GetWindowTextLength(hTXT_ExecutableLocation) + 1;

					wchar_t* lpwAppLocation = new wchar_t[nAppTextLength];
					GetWindowText(hTXT_ExecutableLocation, lpwAppLocation, nAppTextLength);

					// Removes all chars '"' in the app location.
					unsigned int counter = 0;
					for (unsigned int i = 0; lpwAppLocation[i] != NULL; ++i)
						if (lpwAppLocation[i] == '"')
							++counter;

					if (counter)
					{
						wchar_t* lpwFixLocation = new wchar_t[nAppTextLength - counter];

						for (int i = 0, j = 0; i < nAppTextLength; ++i)
							if (lpwAppLocation[i] != '"')
								lpwFixLocation[j++] = lpwAppLocation[i]; 

						nAppTextLength -= counter;
						delete[] lpwAppLocation;
						lpwAppLocation = lpwFixLocation;
					}
					
					if (!DoesFileExist(lpwAppLocation))
					{
						MessageBox(hWnd, L"The application path does not exist.", L"Error.", MB_ICONERROR);
						delete[] lpwAppLocation;

						break;
					}

					int nDllTextLength = GetWindowTextLength(hTXT_DllLocation) + 1;

					wchar_t* lpwDllLocation = new wchar_t[nDllTextLength];
					GetWindowText(hTXT_DllLocation, lpwDllLocation, nDllTextLength);

					// Removes all chars '"' in the dll file location.
					counter = 0;
					for (unsigned int i = 0; lpwDllLocation[i] != NULL; ++i)
						if (lpwDllLocation[i] == '"')
							++counter;

					if (counter)
					{
						wchar_t* lpwFixLocation = new wchar_t[nDllTextLength - counter];

						for (int i = 0, j = 0; i < nDllTextLength; ++i)
							if (lpwDllLocation[i] != '"')
								lpwFixLocation[j++] = lpwDllLocation[i]; 

						nDllTextLength -= counter;
						delete[] lpwDllLocation;
						lpwDllLocation = lpwFixLocation;
					}

					if (!DoesFileExist(lpwDllLocation))
					{
						MessageBox(hWnd, L"The dynamic link library path does not exist.", L"Error.", MB_ICONERROR);
						delete[] lpwAppLocation;
						delete[] lpwDllLocation;

						break;
					}

					STARTUPINFO si = { };
					si.cb = sizeof(si);

					PROCESS_INFORMATION pi = { };
					if (CreateProcess(NULL, lpwAppLocation, NULL, NULL, false, CREATE_SUSPENDED, NULL, NULL, &si, &pi))
					{
						/*
						int address = (int)GetProcAddress(GetModuleHandle(L"Kernel32"), "LoadLibraryW");
						SuspendThread(pi.hThread);OpenThread(
						*/
						
						//CONTEXT context;
						//context.ContextFlags = CONTEXT_CONTROL;
						//GetThreadContext(pi.hThread, &context);

						////context.
						//void* lpAddress = VirtualAllocEx(pi.hProcess, NULL, nDllTextLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
						//ResumeThread(pi.hThread);


						void* lpAddress = VirtualAllocEx(pi.hProcess, NULL, nDllTextLength, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
						if (WriteProcessMemory(pi.hProcess, lpAddress, lpwDllLocation, sizeof(wchar_t) * nDllTextLength, NULL))
						{
							CreateRemoteThread(pi.hProcess, NULL, 0,
								(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"Kernel32"), "LoadLibraryW"),
								lpAddress, 0, NULL);
						}

						ResumeThread(pi.hThread);
					}

					delete[] lpwDllLocation;
					delete[] lpwAppLocation;
				}
				break;

			case Handler::btnAddItem:
				{
					// Get a title.
					int length = GetWindowTextLength(hTXT_ItemName) + 1;

					wchar_t* lpwItemName = new wchar_t[length];

					GetWindowText(hTXT_ItemName, lpwItemName, length);

					// Add an item to combo box.
					SendMessage(hCMB_Items, CB_ADDSTRING, NULL, (LPARAM)lpwItemName);

					// TODO: Make it better...
					// Get items number.
					HKEY hKey;
					RegCreateKey(HKEY_CURRENT_USER, MAIN_REGISTRY_NAME, &hKey);

					wchar_t lpwItemWithIndex[16] = { };
					unsigned int i;
					for (i = 0;; ++i)
					{
						wsprintf(lpwItemWithIndex, L"Item %d", i);

						HKEY key;
						if (RegOpenKeyEx(hKey, lpwItemWithIndex, 0, KEY_ALL_ACCESS, &key)) // if failed...
							break;
					}

					RegCreateKey(hKey, lpwItemWithIndex, &hKey);

					RegSetValueEx(hKey, L"Item name", 0, REG_SZ, (unsigned char*)lpwItemName, length * sizeof(wchar_t));
					delete[] lpwItemName;

					length = GetWindowTextLength(hTXT_ExecutableLocation) + 1;
					wchar_t* lpwExectuableLocation = new wchar_t[length];

					GetWindowText(hTXT_ExecutableLocation, lpwExectuableLocation, length);
					RegSetValueEx(hKey, L"Executable location", 0, REG_SZ, (unsigned char*)lpwExectuableLocation, length * sizeof(wchar_t));

					delete[] lpwExectuableLocation;

					length = GetWindowTextLength(hTXT_DllLocation) + 1;
					wchar_t* lpwDllLocation = new wchar_t[length];

					GetWindowText(hTXT_DllLocation, lpwDllLocation, length);
					RegSetValueEx(hKey, L"Dynamic Link Library location", 0, REG_SZ, (unsigned char*)lpwDllLocation, length * sizeof(wchar_t));

					delete[] lpwDllLocation;

					RegCloseKey(hKey);
				}
				break;

			case Handler::btnUpdateItem:
				{
					unsigned int itemsNumber = SendMessage(hCMB_Items, CB_GETCOUNT, NULL, NULL);
					if (itemsNumber)
					{
						unsigned int selectedItemIndex = SendMessage(hCMB_Items, CB_GETCURSEL, NULL, NULL);
						wchar_t lpwItemWithIndex[16] = { };

						wsprintf(lpwItemWithIndex, L"Item %d", selectedItemIndex);

						HKEY hKey;
						RegCreateKey(HKEY_CURRENT_USER, MAIN_REGISTRY_NAME, &hKey);
						RegCreateKey(hKey, lpwItemWithIndex, &hKey);

						int length;

						length = GetWindowTextLength(hTXT_ItemName) + 1;

						wchar_t* lpwItemName = new wchar_t[length];
						GetWindowText(hTXT_ItemName, lpwItemName, length);

						RegSetValueEx(hKey, L"Item name", 0, REG_SZ, (unsigned char*)lpwItemName, length * sizeof(wchar_t));

						SendMessage(hCMB_Items, CB_DELETESTRING, (WPARAM)selectedItemIndex, NULL);
						SendMessage(hCMB_Items, CB_INSERTSTRING, (WPARAM)selectedItemIndex, (LPARAM)lpwItemName);
						SendMessage(hCMB_Items, CB_SETCURSEL, (WPARAM)selectedItemIndex, NULL);

						delete[] lpwItemName;

						length = GetWindowTextLength(hTXT_ExecutableLocation) + 1;
						wchar_t* lpwExectuableLocation = new wchar_t[length];

						GetWindowText(hTXT_ExecutableLocation, lpwExectuableLocation, length);
						RegSetValueEx(hKey, L"Executable location", 0, REG_SZ, (unsigned char*)lpwExectuableLocation, length * sizeof(wchar_t));

						delete[] lpwExectuableLocation;

						length = GetWindowTextLength(hTXT_DllLocation) + 1;
						wchar_t* lpwDllLocation = new wchar_t[length];

						GetWindowText(hTXT_DllLocation, lpwDllLocation, length);
						RegSetValueEx(hKey, L"Dynamic Link Library location", 0, REG_SZ, (unsigned char*)lpwDllLocation, length * sizeof(wchar_t));

						delete[] lpwDllLocation;
						RegCloseKey(hKey);
					}
				}
				break;

			case Handler::btnDeleteItem:
				{
					int itemsNumber = SendMessage(hCMB_Items, CB_GETCOUNT, NULL, NULL);
					if (itemsNumber)
					{
						int selectedItemIndex = SendMessage(hCMB_Items, CB_GETCURSEL, NULL, NULL);
						if (selectedItemIndex == -1)
							break;

						// Delete current tree.
						HKEY hKey;
						RegCreateKey(HKEY_CURRENT_USER, MAIN_REGISTRY_NAME, &hKey);

						wchar_t lpwItemName[16] = { };
						wsprintf(lpwItemName, L"Item %d", selectedItemIndex);

						RegDeleteTree(hKey, lpwItemName);
						SendMessage(hCMB_Items, CB_DELETESTRING, (WPARAM)selectedItemIndex, NULL);

						for (int i = selectedItemIndex + 1; i < itemsNumber; ++i)
						{
							wchar_t lpwPreviousItemName[16] = { };
							wsprintf(lpwPreviousItemName, L"Item %d", i - 1);
							wsprintf(lpwItemName, L"Item %d", i);

							HKEY hKeyDestination;
							RegCreateKey(HKEY_CURRENT_USER, MAIN_REGISTRY_NAME, &hKeyDestination);
							RegCreateKey(hKeyDestination, lpwPreviousItemName, &hKeyDestination);

							RegCopyTree(hKey, lpwItemName, hKeyDestination);
							RegDeleteTree(hKey, lpwItemName);
						}

						RegCloseKey(hKey);

						EnableWindow(hBTN_ReorderUp, false);
						EnableWindow(hBTN_ReorderDown, false);
					}
				}
				break;

			case Handler::btnReorderUp:
				{
					unsigned int selectedItemIndex = SendMessage(hCMB_Items, CB_GETCURSEL, NULL, NULL);
					if (selectedItemIndex == 0)
						break;

					SwapItemsInRegistry(selectedItemIndex, selectedItemIndex - 1);

					int length = SendMessage(hCMB_Items, CB_GETLBTEXTLEN, (WPARAM)selectedItemIndex, NULL);
					wchar_t* lpwItem1 = new wchar_t[length + 1];
					SendMessage(hCMB_Items, CB_GETLBTEXT, (WPARAM)selectedItemIndex, (LPARAM)lpwItem1);

					length = SendMessage(hCMB_Items, CB_GETLBTEXTLEN, (WPARAM)selectedItemIndex - 1, NULL);
					wchar_t* lpwItem2 = new wchar_t[length + 1];
					SendMessage(hCMB_Items, CB_GETLBTEXT, (WPARAM)selectedItemIndex - 1, (LPARAM)lpwItem2);

					SendMessage(hCMB_Items, CB_DELETESTRING, (WPARAM)selectedItemIndex, NULL);
					SendMessage(hCMB_Items, CB_INSERTSTRING, (WPARAM)selectedItemIndex, (LPARAM)lpwItem2);

					SendMessage(hCMB_Items, CB_DELETESTRING, (WPARAM)selectedItemIndex - 1, NULL);
					SendMessage(hCMB_Items, CB_INSERTSTRING, (WPARAM)selectedItemIndex - 1, (LPARAM)lpwItem1);

					SendMessage(hCMB_Items, CB_SETCURSEL, (WPARAM)selectedItemIndex - 1, NULL);

					if (selectedItemIndex - 1 == 0)
						EnableWindow(hBTN_ReorderUp, false);
					else
						EnableWindow(hBTN_ReorderUp, true);

					EnableWindow(hBTN_ReorderDown, true);

					delete[] lpwItem1;
					delete[] lpwItem2;
				}
				break;

			case Handler::btnReorderDown:
				{
					unsigned int selectedItemIndex = SendMessage(hCMB_Items, CB_GETCURSEL, NULL, NULL);
					unsigned int itemsNumber = SendMessage(hCMB_Items, CB_GETCOUNT, NULL, NULL);
					if (selectedItemIndex >= itemsNumber - 1)
						break;

					SwapItemsInRegistry(selectedItemIndex, selectedItemIndex + 1);

					int length = SendMessage(hCMB_Items, CB_GETLBTEXTLEN, (WPARAM)selectedItemIndex, NULL);
					wchar_t* lpwItem1 = new wchar_t[length + 1];
					SendMessage(hCMB_Items, CB_GETLBTEXT, (WPARAM)selectedItemIndex, (LPARAM)lpwItem1);

					length = SendMessage(hCMB_Items, CB_GETLBTEXTLEN, (WPARAM)selectedItemIndex + 1, NULL);
					wchar_t* lpwItem2 = new wchar_t[length + 1];
					SendMessage(hCMB_Items, CB_GETLBTEXT, (WPARAM)selectedItemIndex + 1, (LPARAM)lpwItem2);

					SendMessage(hCMB_Items, CB_DELETESTRING, (WPARAM)selectedItemIndex, NULL);
					SendMessage(hCMB_Items, CB_INSERTSTRING, (WPARAM)selectedItemIndex, (LPARAM)lpwItem2);

					SendMessage(hCMB_Items, CB_DELETESTRING, (WPARAM)selectedItemIndex + 1, NULL);
					SendMessage(hCMB_Items, CB_INSERTSTRING, (WPARAM)selectedItemIndex + 1, (LPARAM)lpwItem1);

					SendMessage(hCMB_Items, CB_SETCURSEL, (WPARAM)selectedItemIndex + 1, NULL);

					EnableWindow(hBTN_ReorderUp, true);

					if (selectedItemIndex + 1 == itemsNumber - 1)
						EnableWindow(hBTN_ReorderDown, false);
					else
						EnableWindow(hBTN_ReorderDown, true);

					delete[] lpwItem1;
					delete[] lpwItem2;
				}
				break;

			case Handler::cmbItems:
				{
					switch (HIWORD(wParam))
					{
					case CBN_SELCHANGE:
						{
							unsigned int selectedItemIndex = SendMessage(hCMB_Items, CB_GETCURSEL, NULL, NULL);
							unsigned int itemsNumber = SendMessage(hCMB_Items, CB_GETCOUNT, NULL, NULL);

							if (selectedItemIndex == 0)
								EnableWindow(hBTN_ReorderUp, false);
							else
								EnableWindow(hBTN_ReorderUp, true);

							if (selectedItemIndex == itemsNumber - 1)
								EnableWindow(hBTN_ReorderDown, false);
							else
								EnableWindow(hBTN_ReorderDown, true);

							HKEY hKey;
							RegCreateKey(HKEY_CURRENT_USER, MAIN_REGISTRY_NAME, &hKey);

							wchar_t lpwItem[16] = { };
							wsprintf(lpwItem, L"Item %d", selectedItemIndex);

							RegOpenKey(hKey, lpwItem, &hKey);
							
							DWORD size;
							wchar_t* data;
							RegQueryValueEx(hKey, L"Item name", NULL, NULL, NULL, &size);
							if (size)
							{
								data = new wchar_t[size];

								RegQueryValueEx(hKey, L"Item name", NULL, NULL, (unsigned char*)data, &size);
								SendMessage(hTXT_ItemName, WM_SETTEXT, NULL, (LPARAM)data);

								delete[] data;
							}

							RegQueryValueEx(hKey, L"Executable location", NULL, NULL, NULL, &size);
							if (size)
							{
								data = new wchar_t[size];

								RegQueryValueEx(hKey, L"Executable location", NULL, NULL, (unsigned char*)data, &size);
								SendMessage(hTXT_ExecutableLocation, WM_SETTEXT, NULL, (LPARAM)data);

								delete[] data;
							}

							RegQueryValueEx(hKey, L"Dynamic Link Library location", NULL, NULL, NULL, &size);
							if (size)
							{
								data = new wchar_t[size];

								RegQueryValueEx(hKey, L"Dynamic Link Library location", NULL, NULL, (unsigned char*)data, &size);
								SendMessage(hTXT_DllLocation, WM_SETTEXT, NULL, (LPARAM)data);

								delete[] data;
							}

							RegCloseKey(hKey);
						}
						break;
					}
				}
				break;
			}
		}
		break;

	case WM_DESTROY:
		{
			PostQuitMessage(wParam);
			return 0;
		}
		break;

	case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC hDC = BeginPaint(hWnd, &paint);

			FillRect(hDC, &paint.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

			EndPaint(hWnd, &paint);
		}
		break;

	default:
		break;
	}

	return DefWindowProc(hWnd, Msg, wParam, lParam);
}