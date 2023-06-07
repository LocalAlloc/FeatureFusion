#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <winternl.h>
#include <tchar.h>
#include <Uxtheme.h>
#define IDC_PROCESS_NAME 1001
#define IDC_TERMINATE_PROCESS 1002
#define IDOK 1
#define IDCANCEL 2


#pragma comment(lib, "ntdll.lib")
#define ProcessBreakawayMode                  (PROCESS_INFORMATION_CLASS)29
#define PROCESS_BREAKAWAY_MASK                (DWORD)0x07000000L 
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)  

typedef NTSTATUS(WINAPI* pNtSetInformationProcess)(HANDLE, PROCESS_INFORMATION_CLASS, PVOID, ULONG);
LRESULT CALLBACK InputDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

bool IsCriticalProcess(HANDLE hProcess) {
    NTSTATUS status;
    ULONG BreakOnTermination;
    status = NtQueryInformationProcess(hProcess, ProcessBreakOnTermination, &BreakOnTermination, sizeof(BreakOnTermination), NULL);
    if (NT_SUCCESS(status)) {
        if (BreakOnTermination == 1) {
            return true;
        }
    }
    return false;
}

bool SetPrivilege(HANDLE hToken, LPCTSTR Privilege, BOOL bEnablePrivilege) {
    TOKEN_PRIVILEGES tp = { 0 };
    LUID luid;

    if (!LookupPrivilegeValue(NULL, Privilege, &luid)) {
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = (bEnablePrivilege) ? SE_PRIVILEGE_ENABLED : 0;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
        return false;
    }

    return (GetLastError() == ERROR_SUCCESS);
}

void enableRegistryEditor()
{
    HKEY hKey;
    DWORD dwValue = 0;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\"), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey, _T("DisableRegistryTools"), 0, REG_DWORD, (BYTE*)&dwValue, sizeof(DWORD));
        RegCloseKey(hKey);
        MessageBoxA(NULL, "Success!", NULL, MB_OK | MB_ICONINFORMATION);
    }
    else {
        MessageBoxA(NULL, "Failed!", NULL, MB_OK | MB_ICONSTOP);
    }
}

void enableTaskManager()
{
    HKEY hKey;
    DWORD dwValue = 0;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\"), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey, _T("DisableTaskmgr"), 0, REG_DWORD, (BYTE*)&dwValue, sizeof(DWORD));
        RegCloseKey(hKey);
        MessageBoxA(NULL, "Success!", NULL, MB_OK | MB_ICONINFORMATION);
    }
    else {
        MessageBoxA(NULL, "Failed!", NULL, MB_OK | MB_ICONSTOP);
    }
}

void enableCMD()
{
    HKEY hKey;
    const WCHAR* cmdDisabledValue = L"DisableCMD";

    // Open the registry key for modifying
    if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Policies\\Microsoft\\Windows\\System", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS)
    {
        // Set the value to disable CMD
        //reinterpret_cast<const BYTE*>(&disableValue)
        DWORD disableValue = 0;
        if (RegSetValueEx(hKey, cmdDisabledValue, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&disableValue), sizeof(DWORD)) == ERROR_SUCCESS)
        {
            MessageBoxA(NULL, "Command Prompt (CMD) has been enabled.", "Success", MB_ICONINFORMATION);
        }
        else
        {
            MessageBoxA(NULL, "Failed to set registry value.", "Error", MB_ICONERROR);
        }

        // Close the registry key
        RegCloseKey(hKey);
    }
    else
    {
        MessageBoxA(NULL, "Failed to open registry key.", "Error", MB_ICONERROR);
    }
}

void enableUAC()
{
    DWORD dwVal = 1;

    HKEY hKey;
    RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System\\", 0, KEY_ALL_ACCESS, &hKey);
    RegSetValueEx(hKey, L"EnableLUA", 0, REG_DWORD, (LPBYTE)&dwVal, sizeof(DWORD));
    RegCloseKey(hKey);
}

void enableThemes()
{
    HKEY hKey;
    DWORD dwValue = 0;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System"), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey, _T("NoThemesTab"), 0, REG_DWORD, (BYTE*)&dwValue, sizeof(DWORD));
        RegCloseKey(hKey);
        MessageBoxA(NULL, "Success!", NULL, MB_OK | MB_ICONINFORMATION);
    }
    else {
        MessageBoxA(NULL, "Failed!", NULL, MB_OK | MB_ICONSTOP);
    }
}

void enableColorSettings()
{
    HKEY hKey;
    DWORD dwValue = 0;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System"), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey, _T("NoDispAppearancePage"), 0, REG_DWORD, (BYTE*)&dwValue, sizeof(DWORD));
        RegCloseKey(hKey);
        MessageBoxA(NULL, "Success!", NULL, MB_OK | MB_ICONINFORMATION);
    }
    else {
        MessageBoxA(NULL, "Failed!", NULL, MB_OK | MB_ICONSTOP);
    }
}

void enableRunMenu()
{
    HKEY hKey;
    DWORD dwValue = 0;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer"), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey, _T("NoRun"), 0, REG_DWORD, (BYTE*)&dwValue, sizeof(DWORD));
        RegCloseKey(hKey);
        MessageBoxA(NULL, "Success!", NULL, MB_OK | MB_ICONINFORMATION);
    }
    else {
        MessageBoxA(NULL, "Failed!", NULL, MB_OK | MB_ICONSTOP);
    }
}
void enableWindowsKey()
{
    HKEY hKey;
    DWORD dwValue = 0;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer"), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey, _T("NoWinKeys"), 0, REG_DWORD, (BYTE*)&dwValue, sizeof(DWORD));
        RegCloseKey(hKey);
        MessageBoxA(NULL, "Success!", NULL, MB_OK | MB_ICONINFORMATION);
    }
    else {
        MessageBoxA(NULL, "Failed!", NULL, MB_OK | MB_ICONSTOP);
    }
}

void resetShellKey()
{
    HKEY hKey;
    LPCTSTR defaultValue = _T("explorer.exe");

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey, _T("Shell"), 0, REG_SZ, (BYTE*)defaultValue, (_tcslen(defaultValue) + 1) * sizeof(TCHAR));
        RegCloseKey(hKey);
        MessageBoxA(NULL, "Success!", NULL, MB_OK | MB_ICONINFORMATION);
    }
    else {
        MessageBoxA(NULL, "Failed!", NULL, MB_OK | MB_ICONSTOP);
    }
}

void resetUserinitKey()
{
    HKEY hKey;
    LPCTSTR defaultValue = _T("C:\\Windows\\system32\\userinit.exe,");

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey, _T("Userinit"), 0, REG_SZ, (BYTE*)defaultValue, (_tcslen(defaultValue) + 1) * sizeof(TCHAR));
        RegCloseKey(hKey);
        MessageBoxA(NULL, "Success!", NULL, MB_OK | MB_ICONINFORMATION);
    }
    else {
        MessageBoxA(NULL, "Failed!", NULL, MB_OK | MB_ICONSTOP);
    }
}

void removeKeyboardRestrictions()
{
    HKEY hKey;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Control\\Keyboard Layout\\"), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        RegDeleteValue(hKey, _T("Scancode Map"));
        RegCloseKey(hKey);
        MessageBoxA(NULL, "Success!", NULL, MB_OK | MB_ICONINFORMATION);
    }
    else {
        MessageBoxA(NULL, "Failed!", NULL, MB_OK | MB_ICONSTOP);
    }
}

void resetPrimaryMouseButton()
{
    HKEY hKey;
    DWORD dwValue = 0;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Control Panel\\Mouse"), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey, _T("SwapMouseButtons"), 0, REG_DWORD, (BYTE*)&dwValue, sizeof(DWORD));
        RegCloseKey(hKey);
        MessageBoxA(NULL, "Success!", NULL, MB_OK | MB_ICONINFORMATION);
    }
    else {
        MessageBoxA(NULL, "Failed!", NULL, MB_OK | MB_ICONSTOP);
    }
}
void resetExeIcons()
{
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("exefile\\DefaultIcon"), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey, _T(""), 0, REG_SZ, (BYTE*)_T("%1"), (_tcslen(_T("%1")) + 1) * sizeof(TCHAR));
        RegCloseKey(hKey);
        MessageBoxA(NULL, "Success!", NULL, MB_OK | MB_ICONINFORMATION);
    }
    else {
        MessageBoxA(NULL, "Failed!", NULL, MB_OK | MB_ICONSTOP);
    }
}

void resetTxtIcons()
{
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, _T("txtfile\\DefaultIcon"), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey, _T(""), 0, REG_SZ, (BYTE*)_T("%systemroot%\\system32\\imageres.dll,-102"), (_tcslen(_T("%systemroot%\\system32\\imageres.dll,-102")) + 1) * sizeof(TCHAR));
        RegCloseKey(hKey);
        MessageBoxA(NULL, "Success!", NULL, MB_OK | MB_ICONINFORMATION);
    }
    else {
        MessageBoxA(NULL, "Failed!", NULL, MB_OK | MB_ICONSTOP);
    }
}

void resetExeCommandDefault()
{
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Classes\\exefile\\shell\\open\\command"), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey, _T(""), 0, REG_SZ, (BYTE*)_T("\"%1\" %*"), (_tcslen(_T("\"%1\" %*")) + 1) * sizeof(TCHAR));
        RegCloseKey(hKey);
        MessageBoxA(NULL, "Success!", NULL, MB_OK | MB_ICONINFORMATION);
    }
    else {
        MessageBoxA(NULL, "Failed!", NULL, MB_OK | MB_ICONSTOP);
    }
}

void resetRunAsCommandDefault()
{
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Classes\\exefile\\shell\\runas\\command"), 0, KEY_WRITE, &hKey) == ERROR_SUCCESS)
    {
        RegSetValueEx(hKey, _T(""), 0, REG_SZ, (BYTE*)_T("\"%1\" %*"), (_tcslen(_T("\"%1\" %*")) + 1) * sizeof(TCHAR));
        RegCloseKey(hKey);
        MessageBoxA(NULL, "Success!", NULL, MB_OK | MB_ICONINFORMATION);
    }
    else {
        MessageBoxA(NULL, "Failed!", NULL, MB_OK | MB_ICONSTOP);
    }
}
int automatic()
{
    MessageBoxA(NULL, "Trying everything..", "", MB_OK | MB_ICONINFORMATION);
    // Re-enable registry editor if disabled
    enableRegistryEditor();

    // Re-enable Task Manager if disabled
    enableTaskManager();

    // Re-enable cmd if disabled
    enableCMD();

    // Re-enable UAC if disabled
    enableUAC();

    // Re-enable themes settings if not allowed
    enableThemes();

    // Re-enable color settings if disabled
    enableColorSettings();

    // Re-enable run menu if disabled
    enableRunMenu();

    // Re-enable Windows key if disabled
    enableWindowsKey();

    // Reset the shell registry key to default in Windows
    resetShellKey();

    // Reset the userinit registry key in Windows to default
    resetUserinitKey();

    // Remove keyboard restrictions
    removeKeyboardRestrictions();

    // Toggle the primary mouse button if switched
    resetPrimaryMouseButton();

    //reset exe icons if not default
    resetExeIcons();

    //reset exefile shell/open/command if switched
    resetExeCommandDefault();

    //reset txt file icons if switched
    resetTxtIcons();

    //same as before but for runas parameter in function call
    resetRunAsCommandDefault();

    return 0;
}
// Function to enable visual styles
void EnableVisualStyles()
{
    HMODULE hModule = LoadLibrary(TEXT("uxtheme.dll"));
    if (hModule)
    {
        typedef HRESULT(WINAPI* PFNISAPPTHEMED)();
        typedef HTHEME(WINAPI* PFNOPENTHEMEDATA)(HWND, LPCWSTR);

        PFNISAPPTHEMED pfnIsAppThemed = (PFNISAPPTHEMED)GetProcAddress(hModule, "IsAppThemed");
        PFNOPENTHEMEDATA pfnOpenThemeData = (PFNOPENTHEMEDATA)GetProcAddress(hModule, "OpenThemeData");

        if (pfnIsAppThemed && pfnOpenThemeData && pfnIsAppThemed())
        {
            HWND hWnd = NULL; // Replace with your window handle
            HTHEME hTheme = pfnOpenThemeData(hWnd, L"BUTTON");
            if (hTheme)
            {
                // Apply visual styles to the window
                SetWindowTheme(hWnd, L"Explorer", NULL);
                CloseThemeData(hTheme);
            }
        }

        FreeLibrary(hModule);
    }
}
// Global variables
HWND hMainWindow;
HWND hInputDialog;
HWND hProcessNameTextBox;
// Function to create and display the input dialog
void ShowInputDialog()
{
    // Create the input dialog window
    hInputDialog = CreateWindowW(L"STATIC", L"Enter Process ID: ",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,
        hMainWindow, NULL, NULL, NULL);
    SetWindowLongPtr(hInputDialog, GWLP_WNDPROC, (LONG_PTR)InputDialogProc);

    // Create the process name text box
    hProcessNameTextBox = CreateWindowW(L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        20, 50, 260, 24, hInputDialog, NULL, NULL, NULL);

    // Create the "Terminate Process" checkbox
    HWND hTerminateCheckBox = CreateWindowW(L"BUTTON", L"Terminate Process",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        20, 80, 260, 24, hInputDialog, NULL, NULL, NULL);

    HWND hOkButton = CreateWindowW(L"BUTTON", L"OK",
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        140, 110, 100, 30, hInputDialog, (HMENU)IDOK, NULL, NULL);

    HWND hCancelButton = CreateWindowW(L"BUTTON", L"Cancel",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 110, 100, 30, hInputDialog, (HMENU)IDCANCEL, NULL, NULL);

    // Apply modern styles and alignment
    SetWindowLongPtr(hInputDialog, GWL_EXSTYLE, GetWindowLongPtr(hInputDialog, GWL_EXSTYLE) | WS_EX_CONTROLPARENT);
    SetWindowLongPtr(hInputDialog, GWL_STYLE, GetWindowLongPtr(hInputDialog, GWL_STYLE) & ~WS_CAPTION);
    SendMessageW(hInputDialog, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

    SendMessageW(hProcessNameTextBox, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    SendMessageW(hTerminateCheckBox, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    SendMessageW(hOkButton, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    SendMessageW(hCancelButton, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);

    //Center the input dialog on the screen
    RECT dialogRect, desktopRect;
    GetWindowRect(hInputDialog, &dialogRect);
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &desktopRect, 0);
    int centerX = (desktopRect.right + desktopRect.left - (dialogRect.right - dialogRect.left)) / 2;
    int centerY = (desktopRect.bottom + desktopRect.top - (dialogRect.bottom - dialogRect.top)) / 2;
    SetWindowPos(hInputDialog, NULL, centerX, centerY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

// Function to handle messages for the input dialog window
LRESULT CALLBACK InputDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) // Corrected the function name
{
    switch (uMsg)
    {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) // OK button clicked
        {
            TCHAR processIdStr[256];
            GetWindowText(hProcessNameTextBox, processIdStr, sizeof(processIdStr) / sizeof(processIdStr[0]));
            DWORD processId = _wtoi(processIdStr);
            bool terminateProcess = SendMessageW(GetDlgItem(hwnd, 2), BM_GETCHECK, 0, 0) == BST_CHECKED;
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_SET_INFORMATION, FALSE, processId);
            if (hProcess == NULL) {
                MessageBox(hwnd, L"Failed to open process", L"Error", MB_ICONERROR);
                return 1;
            }

            bool isCritical = IsCriticalProcess(hProcess);

            if (isCritical) {
                HANDLE hToken;
                if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
                    if (SetPrivilege(hToken, SE_DEBUG_NAME, TRUE)) {
                        pNtSetInformationProcess NtSetInformationProcess = (pNtSetInformationProcess)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtSetInformationProcess");
                        if (NtSetInformationProcess != NULL) {
                            ULONG BreakawayFlags = 0;
                            if (NtSetInformationProcess(hProcess, ProcessBreakawayMode, &BreakawayFlags, sizeof(BreakawayFlags)) == STATUS_SUCCESS) {
                                MessageBox(hwnd, L"Removed critical system process property", L"Success", MB_ICONINFORMATION);
                            }
                            else {
                                MessageBox(hwnd, L"Failed to remove critical system process property", L"Error", MB_ICONERROR);
                            }
                        }
                        else {
                            MessageBox(hwnd, L"Failed to get address of NtSetInformationProcess function", L"Error", MB_ICONERROR);
                        }
                    }
                    else {
                        MessageBox(hwnd, L"Failed to enable SE_DEBUG_NAME privilege", L"Error", MB_ICONERROR);
                    }
                }
                else {
                    MessageBox(hwnd, L"Failed to open process token", L"Error", MB_ICONERROR);
                }
            }
            else {
                MessageBox(hwnd, L"Process is not a critical system process", L"Error", MB_ICONERROR);
            }
            // Convert the process ID to a string
            std::string command = "taskkill /F /PID " + std::to_string(processId);

            // Execute the command
            std::system(command.c_str());
            if (terminateProcess) {
                if (TerminateProcess(hProcess, 0)) {
                    MessageBox(hwnd, L"Terminated process", L"Success", MB_ICONINFORMATION);
                }
                else {
                    MessageBox(hwnd, L"Failed to terminate process", L"Error", MB_ICONERROR);
                }
            }
            hInputDialog = NULL;
            // Close the input dialog
            EndDialog(hwnd, IDCANCEL);
            DestroyWindow(hwnd);

            return 0;
        }
        if (LOWORD(wParam) == IDCANCEL) // Cancel button clicked (IDCANCEL)
        {
            hInputDialog = NULL;
            // Close the input dialog
            EndDialog(hwnd, IDCANCEL);
            DestroyWindow(hwnd);
            return 0;
        }
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}