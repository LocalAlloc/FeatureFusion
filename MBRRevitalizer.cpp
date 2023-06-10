#include "MBRRevitalizer.h"
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    // Initialize common controls, enable visual styles
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);
    const char* filePath = "C:\\Windows\\System32\\MBRHELPER.exe";
    const char* filePath64 = "C:\\Windows\\SysWOW64\\MBRHELPER.exe";
    if (IsWindows64Bit()) {
        if (FileExists(filePath64)) {
            logtofile("File exists!");
            while (true) {
                ProtectPartition64();
            }
        }
        else {
            logtofile("File doesn't exist! (64 Bit)");
            //CONTINUE...
        }
    }
    else {
        if (FileExists(filePath)) {
            logtofileSimple("File exists!");
            while (true) {
                ProtectPartition();
            }
        }
        else {
            logtofileSimple("File doesn't exist! (32 Bit)");
            //CONTINUE...
        }
    }
    HANDLE hDisk = CreateFile(L"\\\\.\\PhysicalDrive0", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
    if (hDisk == INVALID_HANDLE_VALUE) {
        logtofile("Failed To Get A Handle to \"\\\\.\\PhysicalDrive0\" : ");
        ExitProcess(-1);
    }
    // Check if MBR is valid
    unsigned char mbr[512];
    DWORD bytesRead;
    if (!ReadFile(hDisk, mbr, sizeof(mbr), &bytesRead, NULL)) {
        //printf("Failed to read MBR. Error: %lu\n", GetLastError());
        logtofile("Failed To Read MBR");
        system("pause");
        CloseHandle(hDisk);
        ExitProcess(1);
    }
    if (isValidMBR(mbr)) {
        CreateQuestion(hInstance, nCmdShow);
    }
    else {
        MessageBoxA(NULL, "Error! MBR NOT VALID", "ERROR", MB_OK | MB_ICONINFORMATION);
        ExtractResource(IDR_EXE1, L"C:\\Windows\\System32\\BootData.exe");
        if (MessageBoxA(NULL, "Try And Fix Everything? Also Check Out The TOOL in C:\\Windows\\System32\\BootData.exe", "Question", MB_YESNO | MB_ICONQUESTION) == IDYES)
        {
            fixwindowsinstallation();
        }
        else
        {
            PostQuitMessage(0);
        }

    }
    ExitProcess(0);

}
