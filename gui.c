#include "mcbeinject.h"

INT_PTR CALLBACK MainDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	// The first parameter is NULL, hence the Dialog Box shall use the current executable, which is desired.
	INT_PTR res = DialogBoxW(NULL, MAKEINTRESOURCEW(IDD_MAINDIALOG), NULL, MainDlgProc);
	if (res == -1)
	{
		MessageBoxW(NULL, L"Failed to create Dialog Box.", L"MCBEInject", MB_OK | MB_ICONERROR);
	}

	return 0;
}

INT_PTR CALLBACK MainDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		return TRUE;
	}
	case WM_CLOSE:
	{
		EndDialog(hWnd, 0);
		return TRUE;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTONFINDDLL: 
		{
			WCHAR fPath[MAX_PATH];
			ZeroMemory(&fPath, sizeof(fPath));

			OPENFILENAMEW ofn;
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFile = fPath;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFilter = L"DLL Files\0*.dll\0";
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			if (GetOpenFileNameW(&ofn))
			{
				SetDlgItemTextW(hWnd, IDC_PATHEDIT, fPath);
			}

			return TRUE;
		}
		case IDC_INJECTBUTTON:
		{
			WCHAR fPath[MAX_PATH];

			DWORD attributes;

			if (GetDlgItemTextW(hWnd, IDC_PATHEDIT, fPath, MAX_PATH) == 0 ||
				(attributes = GetFileAttributesW(fPath)) == INVALID_FILE_ATTRIBUTES ||
				(attributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				MessageBoxW(NULL, L"Please enter a valid DLL path.", L"MCBEInject", MB_OK | MB_ICONERROR);
				return TRUE;
			}

			if (DllInjectW(CreateMCBEProc(), fPath) == FALSE)
			{
				// There is no need to alert the user here; should DllInjectW have failed, it will have alerted the user
				// itself anyway.
				return TRUE;
			}
			return TRUE;
		}
		case IDCLOSEBUTTON:
		{
			EndDialog(hWnd, 0);
			return TRUE;
		}
		}
	}

	return FALSE;
}