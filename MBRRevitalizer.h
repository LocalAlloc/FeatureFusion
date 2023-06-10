#include <windows.h>
#include <Shlwapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <taskschd.h>
#include <comutil.h>
#include <PowrProf.h>
#include <CommDlg.h>
#include <iostream>
#include <vector>
#include <string>
#include <iostream>
#include <Shlobj.h>
#include <fstream>
#include "FixReg.h"
#include <Vssym32.h>
#include "resource.h"
#include <uxtheme.h>
#include <dwmapi.h>
#include <mmsystem.h>
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "dwmapi.lib")
#define MENU_REMOVE_CRITICAL_PROCESS 1
#define BUTTON_ID_OFFSET 100
#pragma comment(lib, "Taskschd.lib")
#pragma comment(lib, "Comsuppw.lib")
#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "OleAut32.lib")
#pragma comment(lib, "PowrProf.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "winmm.lib")
using namespace std;
void ProtectPartition();
//void runtask();
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MalwareProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK QuestionProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateFixWindows(_In_ HINSTANCE hInstance, _In_ int nCmdShow);
// Global variables
HINSTANCE g_hInstance;
int g_nCmdShow;
#define BOOT_SECTOR_SIZE 512
char backupPath[MAX_PATH] = "C:\\Windows\\System32\\MBRBackup.bin";
void logtofileSimple(const char* message) {
    FILE* file = fopen("C:\\Windows\\log.txt", "a");
    if (file == NULL) {
        printf("Failed to open log file.\n");
        return;
    }

    fprintf(file, "%s\n", message);
    fclose(file);
}
void logtofile(const char* message) {
    FILE* file = fopen("C:\\Windows\\log.txt", "a");
    if (file == NULL) {
        printf("Failed to open log file.\n");
        return;
    }

    // Retrieve error information
    DWORD errorCode = GetLastError();
    LPSTR errorMessage = NULL;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&errorMessage,
        0,
        NULL
    );

    // Get current timestamp
    time_t now = time(NULL);
    struct tm* localTime = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localTime);

    // Append timestamp, error information, and the message to the log file
    fprintf(file, "[%s] Error %d: %s\n%s\n", timestamp, errorCode, errorMessage, message);

    // Free the error message buffer
    LocalFree(errorMessage);

    fclose(file);
}

BOOL IsWindows64Bit()
{
#if defined(_WIN64)
    return TRUE;  // 64-bit program running on 64-bit Windows
#elif defined(_WIN32)
    // 32-bit program running on 64-bit Windows?
    BOOL isWow64 = FALSE;
    typedef BOOL(WINAPI* LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
    LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
        GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
    if (fnIsWow64Process != NULL)
    {
        if (!fnIsWow64Process(GetCurrentProcess(), &isWow64))
        {
            return FALSE;  // function call failed
        }
        else if (isWow64)
        {
            return TRUE;  // 32-bit program running on 64-bit Windows
        }
    }
    return FALSE;  // 32-bit program running on 32-bit Windows
#else
    return FALSE;  // not on Windows
#endif
}
bool isValidMBR(unsigned char* mbr) {
    return (mbr[0x1FE] == 0x55 && mbr[0x1FF] == 0xAA);
}



bool backupMBR(char* path) {
    HANDLE hDisk = CreateFile(L"\\\\.\\PhysicalDrive0", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
    if (hDisk == INVALID_HANDLE_VALUE) {
        printf("Failed to open disk. Error: %lu\n", GetLastError());
        ExitProcess(-1);
    }

    unsigned char mbr[512];
    DWORD bytesRead;
    if (!ReadFile(hDisk, mbr, sizeof(mbr), &bytesRead, NULL)) {
        DWORD errorMessageId = GetLastError();
        LPVOID errorMessageBuffer = NULL;
        DWORD bufferSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            errorMessageId,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&errorMessageBuffer,
            0,
            NULL);
        MessageBox(NULL, (LPCWSTR)errorMessageBuffer, L"Error reading MBR", MB_ICONERROR | MB_OK);
        LocalFree(errorMessageBuffer);
        CloseHandle(hDisk);
        ExitProcess(-1);
    }

    if (isValidMBR(mbr)) {
        FILE* fp;
        fp = fopen(path, "wb");
        if (fp == NULL) {
            DWORD errorMessageId = GetLastError();
            LPVOID errorMessageBuffer = NULL;
            DWORD bufferSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                errorMessageId,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&errorMessageBuffer,
                0,
                NULL);
            MessageBox(NULL, (LPCWSTR)errorMessageBuffer, L"Failed to create backup file.", MB_ICONERROR | MB_OK);
            LocalFree(errorMessageBuffer);
            CloseHandle(hDisk);
            ExitProcess(-1);
        }

        fwrite(mbr, 1, sizeof(mbr), fp);
        fclose(fp);
        MessageBoxA(NULL, "MBR BACKED UP SUCCESSFULLY! ", "INFORMATION", MB_ICONINFORMATION | MB_OK);
    }
    else {
        MessageBoxA(NULL, "INVALID MBR DUDE!", "INFORMATION", MB_ICONINFORMATION | MB_OK);
    }

    CloseHandle(hDisk);
    return 0;
}

BOOL FileExists(const char* filePath) {
    DWORD fileAttributes = GetFileAttributesA(filePath);
    return (fileAttributes != INVALID_FILE_ATTRIBUTES && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

void test()
{
    OPENFILENAME ofn;
    wchar_t szFileName[MAX_PATH] = {};

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = L"Binary Files (*.bin)\0*.bin\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = L"bin";

    if (GetOpenFileName(&ofn)) {

        // Open the binary file
        std::ifstream file(szFileName, std::ios::binary);
        if (!file) {
            MessageBoxA(NULL, "Error unable to open binary file!", "ERROR", MB_ICONHAND || MB_OK);
            ExitProcess(-1);
        }

        // Get the size of the binary file
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        // Read the binary file into a buffer
        std::vector<char> buffer(size);
        if (!file.read(buffer.data(), size)) {
            MessageBoxA(NULL, "Error Unable to read the binary file", "ERROR", MB_ICONHAND || MB_OK);
            ExitProcess(-1);
        }

        // Open the physical drive for writing
        HANDLE hDrive = CreateFile(L"\\\\.\\PhysicalDrive0", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
        if (hDrive == INVALID_HANDLE_VALUE) {
            MessageBoxA(NULL, "Error opening handle to \\\\.\\PhysicalDrive0", "ERROR", MB_ICONHAND || MB_OK);
            ExitProcess(-1);
        }

        // Write the buffer to the boot sector of the physical drive
        DWORD bytesWritten;
        if (!WriteFile(hDrive, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesWritten, NULL)) {
            MessageBoxA(NULL, "Error not able to write to \\\\.\\PhysicalDrive0", "ERROR", MB_ICONHAND || MB_OK);
            ExitProcess(-1);
        }

        // Close the physical drive
        CloseHandle(hDrive);

        MessageBoxA(NULL, "Bootloader backup has been recovered!", "INFORMATION", MB_OK | MB_ICONINFORMATION);
    }

    PostQuitMessage(0);
}
bool ExtractResource(int iId, LPCWSTR pDest) {
    HRSRC aResourceH = FindResource(NULL, MAKEINTRESOURCE(iId), L"EXE");
    if (!aResourceH) {
        MessageBoxA(NULL, "Unable to find resource.", "", MB_OK | MB_ICONHAND);
        return false;
    }

    HGLOBAL aResourceHGlobal = LoadResource(NULL, aResourceH);
    if (!aResourceHGlobal) {
        MessageBoxA(NULL, "Unable to load resource.", "", MB_OK | MB_ICONHAND);
        return false;
    }

    unsigned char* aFilePtr = (unsigned char*)LockResource(aResourceHGlobal);
    if (!aFilePtr) {
        MessageBoxA(NULL, "Unable to lock resource.", "", MB_OK | MB_ICONHAND);
        return false;
    }

    unsigned long aFileSize = SizeofResource(NULL, aResourceH);

    HANDLE file_handle = CreateFile(pDest, FILE_ALL_ACCESS, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (INVALID_HANDLE_VALUE == file_handle) {
        int err = GetLastError();
        if ((ERROR_ALREADY_EXISTS == err) || (32 == err)) {
            return true;
        }
        return false;
    }

    unsigned long numWritten;
    WriteFile(file_handle, aFilePtr, aFileSize, &numWritten, NULL);
    CloseHandle(file_handle);

    return true;
}
void fixwindowsinstallation() {
    if (MessageBoxA(NULL, "Do You Have A Boot Loader Backup?", "Question", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        test();
    }
    else {
        int result;

        system("bootrec /fixmbr");
        Sleep(3000);
        system("bootrec /fixboot");
        system("bootrec /rebuildbcd");
        // Fix the MBR
        result = system("dism /image:C:\\ /cleanup-image /revertpendingactions");
        if (result != 0) {
            MessageBoxA(NULL, "Failed to Fix MBR", "", MB_OK | MB_ICONERROR);
            ExitProcess(-1);
        }

        // Fix the boot sector
        result = system("bootsect /nt60 all /force");
        if (result != 0) {
            MessageBoxA(NULL, "Failed to Fix boot sector", "", MB_OK | MB_ICONERROR);
            ExitProcess(-1);
        }
        result = system("REG ADD \"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\CrashControl\" /v \"AutoReboot\" /t REG_DWORD /d 0 /f");
        if (result != 0) {
            MessageBoxA(NULL, "Failed to add registry key to disable AutoReboot", "", MB_OK | MB_ICONERROR);
            ExitProcess(-1);
        }


        result = system("del /F /Q C:\\boot\\bcd");
        if (result != 0) {
            MessageBoxA(NULL, "Failed to delete OLD bcd", "", MB_OK | MB_ICONERROR);
            ExitProcess(-1);
        }

        result = system("bcdedit /createstore C:\\boot\\bcd");
        if (result != 0) {
            MessageBoxA(NULL, "Failed to Create NEW bcd store", "", MB_OK | MB_ICONERROR);
            ExitProcess(-1);
        }

        result = system("bcdedit /store C:\\boot\\bcd /import C:\\Windows\\Boot\\BCD");
        if (result != 0) {
            MessageBoxA(NULL, "Failed to Import Boot Configuration data", "", MB_OK | MB_ICONERROR);
            ExitProcess(-1);
        }
        MessageBoxA(NULL, "Applied Potential Fixes Possible, If It Doesn't Work \n Please Run MBR RECOVERY TOOL PROVIDED IN WINDOWS! ", "INFORMATION", MB_ICONINFORMATION || MB_OK);
    }
    PostQuitMessage(0);
}
/*Alternate Function*/
bool restoreMBR(char* path) {
    HANDLE hDisk = CreateFile(L"\\\\.\\PhysicalDrive0", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
    if (hDisk == INVALID_HANDLE_VALUE) {
        char errorMessage[256];
        DWORD error = GetLastError();
        sprintf_s(errorMessage, "Failed to open disk. Error: %lu", error);
        MessageBoxA(NULL, errorMessage, "Error", MB_ICONERROR | MB_OK);
        return false;
    }

    FILE* fp;
    fp = fopen(path, "rb");
    if (fp == NULL) {
        MessageBoxA(NULL, "Failed to open backup file.", "Error", MB_ICONERROR | MB_OK);
        CloseHandle(hDisk);
        return false;
    }

    unsigned char mbr[512];
    size_t bytesRead = 0;
    bytesRead = fread(mbr, 1, sizeof(mbr), fp);
    if (bytesRead != sizeof(mbr)) {
        MessageBoxA(NULL, "Failed to read backup file.", "Error", MB_ICONERROR | MB_OK);
        fclose(fp);
        CloseHandle(hDisk);
        return false;
    }

    DWORD bytesWritten;
    if (!WriteFile(hDisk, mbr, sizeof(mbr), &bytesWritten, NULL)) {
        char errorMessage[256];
        DWORD error = GetLastError();
        sprintf_s(errorMessage, "Failed to write MBR. Error: %lu", error);
        MessageBoxA(NULL, errorMessage, "Error", MB_ICONERROR | MB_OK);
        fclose(fp);
        CloseHandle(hDisk);
        return false;
    }

    fclose(fp);
    MessageBoxA(NULL, "MBR restored successfully.", "Success", MB_ICONINFORMATION | MB_OK);

    CloseHandle(hDisk);
    return true;
}
// Function to restore the MBR from the backup file
/*Helper Function For ProtectPartition And ProtectPartition64*/
bool RestoreMBRFromBackup()
{
    HANDLE hDrive = CreateFileA("\\\\.\\PhysicalDrive0", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
    if (hDrive == INVALID_HANDLE_VALUE)
    {
        MessageBoxA(NULL, "Error Invalid DRIVE HANDLE!", "", MB_OK || MB_ICONHAND);
        return false;
    }

    BYTE buffer[BOOT_SECTOR_SIZE];
    DWORD bytesRead;

    // Read the MBR from the backup file
    ifstream file("C:\\Windows\\System32\\MBRBackup.bin", ios::in | ios::binary);
    if (!file.read(reinterpret_cast<char*>(buffer), BOOT_SECTOR_SIZE))
    {
        CloseHandle(hDrive);
        file.close();
        return false;
    }

    // Write the MBR to the drive
    if (!WriteFile(hDrive, buffer, BOOT_SECTOR_SIZE, &bytesRead, NULL))
    {
        MessageBoxA(NULL, "Error Can't Write backup file to MBR", "", MB_OK | MB_ICONHAND);
        CloseHandle(hDrive);
        file.close();
        ExitProcess(1);
    }
    MessageBoxA(NULL, "SUCCESS", "", MB_OK | MB_ICONINFORMATION);
    CloseHandle(hDrive);
    file.close();
    return 0;
}

// Function to check if the MBR has been modified
/*Function Which Monitors Modifications in The MBR Continously(Bin File)
And If It Finds Modifications Fixes Them(32 Bit)*/
void ProtectPartition()
{
    // Read the MBR into a buffer
    HANDLE hPhysicalDisk = CreateFileA("\\\\.\\PhysicalDrive0", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
    if (hPhysicalDisk == INVALID_HANDLE_VALUE)
    {
        return;
    }

    BYTE buffer[BOOT_SECTOR_SIZE];
    DWORD bytesRead;
    if (!ReadFile(hPhysicalDisk, buffer, BOOT_SECTOR_SIZE, &bytesRead, NULL))
    {
        CloseHandle(hPhysicalDisk);
        return;
    }

    // Close the physical disk handle
    CloseHandle(hPhysicalDisk);

    // Read the backup file into a buffer
    std::ifstream backupFile("C:\\Windows\\System32\\MBRBackup.bin", std::ios::binary);
    BYTE backupBuffer[BOOT_SECTOR_SIZE];
    backupFile.read(reinterpret_cast<char*>(backupBuffer), BOOT_SECTOR_SIZE);

    // Compare the current MBR with the backup buffer
    if (memcmp(buffer, backupBuffer, BOOT_SECTOR_SIZE) != 0)
    {
        // Restore the backup
        MessageBoxA(NULL, "CORRUPT MBR FOUND RECOVERING!", "", MB_OK | MB_ICONHAND);
        RestoreMBRFromBackup();
    }
}
/*Function Which Monitors Changes in The MBR Continously(Bin File)
And If It Finds Changes Fixes Them(64 Bit)*/
void ProtectPartition64()
{
    // Read the MBR into a buffer
    HANDLE hPhysicalDisk = CreateFileA("\\\\.\\PhysicalDrive0", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
    if (hPhysicalDisk == INVALID_HANDLE_VALUE)
    {
        return;
    }

    BYTE buffer[BOOT_SECTOR_SIZE];
    DWORD bytesRead;
    if (!ReadFile(hPhysicalDisk, buffer, BOOT_SECTOR_SIZE, &bytesRead, NULL))
    {
        CloseHandle(hPhysicalDisk);
        return;
    }

    // Close the physical disk handle
    CloseHandle(hPhysicalDisk);

    // Read the backup file into a buffer
    std::ifstream backupFile("C:\\Windows\\SysWOW64\\MBRBackup.bin", std::ios::binary);
    BYTE backupBuffer[BOOT_SECTOR_SIZE];
    backupFile.read(reinterpret_cast<char*>(backupBuffer), BOOT_SECTOR_SIZE);

    // Compare the current MBR with the backup buffer
    if (memcmp(buffer, backupBuffer, BOOT_SECTOR_SIZE) != 0)
    {
        // Restore the backup
        MessageBoxA(NULL, "CORRUPT MBR FOUND RECOVERING!", "", MB_OK | MB_ICONHAND);
        RestoreMBRFromBackup();
    }
}
/*End Function To Check If The MBR Has Been Modified*/

/*If The Build Is 32 Bit Do it the normal way*/
int setTask() {
    HRESULT hr1 = CoInitialize(NULL);
    if (FAILED(hr1)) {
        ExitProcess(-1);
    }

    ITaskService* pService = NULL;
    HRESULT hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
    if (FAILED(hr)) {
        ExitProcess(1);
    }

    const VARIANT variant{ {{VT_NULL, 0}} };
    hr = pService->Connect(variant, variant, variant, variant);
    if (FAILED(hr)) {
        pService->Release();
        ExitProcess(1);
    }

    ITaskFolder* pRootFolder = NULL;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr)) {
        pService->Release();
        ExitProcess(1);
    }

    ITaskDefinition* pTask = NULL;
    hr = pService->NewTask(0, &pTask);
    if (FAILED(hr)) {
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IRegistrationInfo* pRegInfo = NULL;
    hr = pTask->get_RegistrationInfo(&pRegInfo);
    if (FAILED(hr)) {
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    hr = pRegInfo->put_Author(_bstr_t(L"LocalAlloc"));
    if (FAILED(hr)) {
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IActionCollection* pActionCollection = NULL;
    hr = pTask->get_Actions(&pActionCollection);
    if (FAILED(hr)) {
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IAction* pAction = NULL;
    hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
    if (FAILED(hr)) {
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IExecAction* pExecAction = NULL;
    hr = pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
    if (FAILED(hr)) {
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    hr = pExecAction->put_Path(_bstr_t(L"C:\\Windows\\System32\\MBRHELPER.exe"));

    // Set the task to run on user logon
    ITriggerCollection* pTriggerCollection = NULL;
    hr = pTask->get_Triggers(&pTriggerCollection);
    if (FAILED(hr)) {
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    ITrigger* pTrigger = NULL;
    hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
    if (FAILED(hr)) {
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    ILogonTrigger* pLogonTrigger = NULL;
    hr = pTrigger->QueryInterface(IID_ILogonTrigger, (void**)&pLogonTrigger);
    if (FAILED(hr)) {
        pTrigger->Release();
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    hr = pLogonTrigger->put_Id(_bstr_t(L"Trigger1"));
    if (FAILED(hr)) {
        pLogonTrigger->Release();
        pTrigger->Release();
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    //hr = pTriggerCollection->Create(pTrigger);
    hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
    if (FAILED(hr)) {
        pLogonTrigger->Release();
        pTrigger->Release();
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    pTriggerCollection->Release();
    pLogonTrigger->Release();
    pTrigger->Release();

    // Set the task to run with highest privileges
    IPrincipal* pPrincipal = NULL;
    hr = pTask->get_Principal(&pPrincipal);
    if (SUCCEEDED(hr) && pPrincipal != NULL) {
        hr = pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
        pPrincipal->Release();
    }
    else {
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    // Register the task
    IRegisteredTask* pRegisteredTask = NULL;
    hr = pRootFolder->RegisterTaskDefinition(
        _bstr_t(L"MBR Utility"),  // Task name
        pTask,  // Task definition
        TASK_CREATE_OR_UPDATE,  // Create or update the task
        _variant_t(),  // No user account information
        _variant_t(),  // No password information
        TASK_LOGON_INTERACTIVE_TOKEN,  // Run the task with the interactive user token
        _variant_t(L""),  // No sddl security descriptor information
        &pRegisteredTask  // Task registration
    );
    if (FAILED(hr)) {
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    pRegisteredTask->Release();
    pTask->Release();
    pRootFolder->Release();
    pService->Release();

    CoUninitialize();
}
/*else do it using a alternating function*/
int setTask64() {
    HRESULT hr1 = CoInitialize(NULL);
    if (FAILED(hr1)) {
        ExitProcess(-1);
    }

    ITaskService* pService = NULL;
    HRESULT hr = CoCreateInstance(CLSID_TaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskService, (void**)&pService);
    if (FAILED(hr)) {
        ExitProcess(1);
    }

    const VARIANT variant{ {{VT_NULL, 0}} };
    hr = pService->Connect(variant, variant, variant, variant);
    if (FAILED(hr)) {
        pService->Release();
        ExitProcess(1);
    }

    ITaskFolder* pRootFolder = NULL;
    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr)) {
        pService->Release();
        ExitProcess(1);
    }

    ITaskDefinition* pTask = NULL;
    hr = pService->NewTask(0, &pTask);
    if (FAILED(hr)) {
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IRegistrationInfo* pRegInfo = NULL;
    hr = pTask->get_RegistrationInfo(&pRegInfo);
    if (FAILED(hr)) {
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    hr = pRegInfo->put_Author(_bstr_t(L"LocalAlloc"));
    if (FAILED(hr)) {
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IActionCollection* pActionCollection = NULL;
    hr = pTask->get_Actions(&pActionCollection);
    if (FAILED(hr)) {
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IAction* pAction = NULL;
    hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
    if (FAILED(hr)) {
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    IExecAction* pExecAction = NULL;
    hr = pAction->QueryInterface(IID_IExecAction, (void**)&pExecAction);
    if (FAILED(hr)) {
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    hr = pExecAction->put_Path(_bstr_t(L"C:\\Windows\\SysWOW64\\MBRHELPER.exe"));

    // Set the task to run on user logon
    ITriggerCollection* pTriggerCollection = NULL;
    hr = pTask->get_Triggers(&pTriggerCollection);
    if (FAILED(hr)) {
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    ITrigger* pTrigger = NULL;
    hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
    if (FAILED(hr)) {
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    ILogonTrigger* pLogonTrigger = NULL;
    hr = pTrigger->QueryInterface(IID_ILogonTrigger, (void**)&pLogonTrigger);
    if (FAILED(hr)) {
        pTrigger->Release();
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    hr = pLogonTrigger->put_Id(_bstr_t(L"Trigger1"));
    if (FAILED(hr)) {
        pLogonTrigger->Release();
        pTrigger->Release();
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    //hr = pTriggerCollection->Create(pTrigger);
    hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
    if (FAILED(hr)) {
        pLogonTrigger->Release();
        pTrigger->Release();
        pTriggerCollection->Release();
        pExecAction->Release();
        pAction->Release();
        pActionCollection->Release();
        pRegInfo->Release();
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    pTriggerCollection->Release();
    pLogonTrigger->Release();
    pTrigger->Release();

    // Set the task to run with highest privileges
    IPrincipal* pPrincipal = NULL;
    hr = pTask->get_Principal(&pPrincipal);
    if (SUCCEEDED(hr) && pPrincipal != NULL) {
        hr = pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
        pPrincipal->Release();
    }
    else {
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    // Register the task
    IRegisteredTask* pRegisteredTask = NULL;
    hr = pRootFolder->RegisterTaskDefinition(
        _bstr_t(L"MBR Utility"),  // Task name
        pTask,  // Task definition
        TASK_CREATE_OR_UPDATE,  // Create or update the task
        _variant_t(),  // No user account information
        _variant_t(),  // No password information
        TASK_LOGON_INTERACTIVE_TOKEN,  // Run the task with the interactive user token
        _variant_t(L""),  // No sddl security descriptor information
        &pRegisteredTask  // Task registration
    );
    if (FAILED(hr)) {
        pTask->Release();
        pRootFolder->Release();
        pService->Release();
        ExitProcess(1);
    }

    pRegisteredTask->Release();
    pTask->Release();
    pRootFolder->Release();
    pService->Release();

    CoUninitialize();
}
/* Plays audio stored as a resource. */
bool playAudio(HINSTANCE hInstance, WCHAR* lpName, UINT bFlags) {
    HANDLE hResInfo = FindResourceW(hInstance, lpName, L"WAVE");

    if (hResInfo == nullptr)
        return false;

    HANDLE hRes = LoadResource(hInstance, (HRSRC)hResInfo);

    if (hRes == nullptr)
        return false;

    WCHAR* lpRes = (WCHAR*)LockResource(hRes);
    FreeResource(hRes);

    return sndPlaySoundW(lpRes, SND_MEMORY | bFlags);
}
/*stops any audio playing from the program*/
void stopAudio() {
    PlaySoundW(nullptr, nullptr, 0);
}
/*Helper Function To Extract And Execute An Executable File(TaskMgrLite)*/
void TaskMgrLite() {
    ExtractResource(IDR_EXE2, L"C:\\Windows\\System32\\TaskMgrLite.exe");
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    if (!CreateProcess(L"C:\\Windows\\System32\\TaskMgrLite.exe", nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
    {
        DeleteFileA("C:\\Windows\\System32\\TaskMgrLite.exe");
        MessageBoxA(NULL, "Failed!", NULL, MB_OK | MB_ICONSTOP);
    }

    // Monitor TaskMgrLite process
    HANDLE hProcess = pi.hProcess;
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    DeleteFileA("C:\\Windows\\System32\\TaskMgrLite.exe");
}
/*Helper Function To Extract And Execute An Executable File(FileCleaner)*/
void FileCleaner() {
    MessageBoxA(NULL, "This is a Dangerous Tool Please Use It With Causion!", "Danger", MB_OK | MB_ICONINFORMATION);
    ExtractResource(IDR_EXE3, L"C:\\Windows\\System32\\FileCleaner.exe");
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    if (!CreateProcess(L"C:\\Windows\\System32\\FileCleaner.exe", nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
    {
        DeleteFileA("C:\\Windows\\System32\\FileCleaner.exe");
        MessageBoxA(NULL, "Failed!", NULL, MB_OK | MB_ICONSTOP);
    }

    // Monitor TaskMgrLite process
    HANDLE hProcess = pi.hProcess;
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    DeleteFileA("C:\\Windows\\System32\\FileCleaner.exe");
}
/*Helper Function To Extract And Execute A Executable File(FileProtector)*/
void FileProtector() {
    MessageBoxA(NULL, "This is a Dangerous Tool Please Use It With Causion!", "Danger", MB_OK | MB_ICONINFORMATION);
    ExtractResource(IDR_EXE4, L"C:\\Windows\\System32\\FileProtectorUI.exe");
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    if (!CreateProcess(L"C:\\Windows\\System32\\FileProtectorUI.exe", nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
    {
        DeleteFileA("C:\\Windows\\System32\\FileProtectorUI.exe");
        MessageBoxA(NULL, "Failed!", NULL, MB_OK | MB_ICONSTOP);
    }

    // Monitor TaskMgrLite process
    HANDLE hProcess = pi.hProcess;
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    DeleteFileA("C:\\Windows\\System32\\FileProtectorUI.exe");
}
/*Create A Question Window Which Handles Modes(Windows)*/
void CreateQuestion(_In_ HINSTANCE hInstance, _In_ int nCmdShow) {
    // Register the window class
    const wchar_t CLASS_NAME[] = L"QuestionClass";
    g_hInstance = hInstance;
    g_nCmdShow = nCmdShow;

    WNDCLASS wc = {};
    wc.lpfnWndProc = QuestionProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

    if (!RegisterClass(&wc)) {
        MessageBoxA(NULL, "Failed To Register Class!", NULL, MB_OK | MB_ICONSTOP);
        ExitProcess(0);
    }

    // Create the window
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Question",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 300,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL)
    {
        MessageBoxA(NULL, "Unable To Create Window!", NULL, MB_OK | MB_ICONSTOP);
        ExitProcess(0);
    }

    // Create three buttons
    HWND button1 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed
        L"MalRemedy",      // Button text
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles
        50,         // x position
        150,         // y position
        100,        // Button width
        30,        // Button height
        hwnd,     // Parent window
        (HMENU)1,       // Button ID
        hInstance,
        NULL);      // Pointer not needed

    HWND button2 = CreateWindow(
        L"BUTTON", L"MBRMon", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        200, 150, 100, 30, hwnd, (HMENU)2, hInstance, NULL);

    HWND button3 = CreateWindow(
        L"BUTTON", L"TaskmgrLite", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        350, 150, 100, 30, hwnd, (HMENU)3, hInstance, NULL);
    HWND button4 = CreateWindow(
        L"BUTTON", L"FileCleaner", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        200, 200, 100, 30, hwnd, (HMENU)4, hInstance, NULL);
    HWND button5 = CreateWindow(
        L"BUTTON", L"FileProtector", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        50, 200, 100, 30, hwnd, (HMENU)5, hInstance, NULL);
    // Create a static control for displaying text
    HWND staticText = CreateWindow(
        L"STATIC", L"Which Mode Would You Like To Use?\nClick The Buttons Below\nTo Confirm Your Choice!",
        WS_VISIBLE | WS_CHILD | SS_CENTER,
        0, 0, 500, 100,
        hwnd, NULL, hInstance, NULL);

    // Set the font for the static control
    HFONT hFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");
    SendMessage(staticText, WM_SETFONT, (WPARAM)hFont, TRUE);

    // Show the window
    ShowWindow(hwnd, nCmdShow);

    // Run the message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
/*Creates A Window For Removing Common Malware Known To Me*/
void CreateFixWindows(_In_ HINSTANCE hInstance, _In_ int nCmdShow) {
    const TCHAR szClassName[] = TEXT("FixWindows");

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = MalwareProc;
    wc.hInstance = hInstance;
    //wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hbrBackground = CreateSolidBrush(RGB(30, 30, 30));
    wc.lpszClassName = szClassName;

    if (!RegisterClass(&wc))
    {
        MessageBoxA(NULL, "Failed To Register Class!", NULL, MB_OK | MB_ICONSTOP);
        ExitProcess(1);
    }

    HWND hwnd = CreateWindowEx(0, szClassName, TEXT("FixWindows"), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 650, NULL, NULL, hInstance, NULL);

    if (!hwnd)
    {
        MessageBoxA(NULL, "Window not found! Returning 2", NULL, MB_OK | MB_ICONSTOP);
        ExitProcess(2);
    }

    // Create the menu
    HMENU hMenu = CreateMenu();
    HMENU hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, MENU_REMOVE_CRITICAL_PROCESS, L"Remove Critical Process Property For A Process");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, L"Menu");


    // Set the menu for the main window
    SetMenu(hwnd, hMenu);
    // Enable visual styles
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    // Create the buttons
    const int numButtons = 11;
    const wchar_t* buttonText[] = {
        L"Launch",
        L"Activate(#1)",
        L"Activate(#2)",
        L"Activate(#3)",
        L"Activate(#4)",
        L"Activate(#5)",
        L"Activate(#6)",
        L"Activate(#7)",
        L"Activate(#8)",
        L"Activate(#9)",
        L"Activate(#10)",
    };

    for (int i = 0; i < numButtons; ++i)
    {
        // Create the button
        HWND hButton = CreateWindowW(TEXT("BUTTON"), buttonText[i], WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            600, 20 + (50 * i), 120, 40, hwnd, (HMENU)(BUTTON_ID_OFFSET + i), NULL, NULL);

        // Set the font for the button
        HFONT hButtonFont = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"));
        HFONT hFont = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"));
        SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (hInputDialog == NULL || !IsDialogMessage(hInputDialog, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}
/*Same As Before But Creates Window Which Guides The User To Install The Tool*/
void CreateMBRWindow(_In_ HINSTANCE hInstance, _In_ int nCmdShow) {
    HBRUSH hBrush = CreateSolidBrush(RGB(173, 216, 230)); // Light blue color
    // Register window class
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(wc);
    wc.hInstance = hInstance;
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = L"FixMbrClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wc.hbrBackground = hBrush;
    if (!RegisterClassEx(&wc))
        ExitProcess(1);

    // Create window
    HWND hwnd1 = CreateWindowExA(0, "FixMbrClass", "MBR Utility", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);
    if (!hwnd1)
        ExitProcess(1);

    // Show window
    ShowWindow(hwnd1, nCmdShow);
    SetClassLongPtr(hwnd1, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);

    // Message loop
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
/*WNDProc Area*/
/*For CreateMBRWindow Function*/
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE: {
        HFONT hFont = CreateFont(40, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");
        // set the font as the window's font
        SendMessage(hwnd, WM_SETFONT, WPARAM(hFont), TRUE);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Set the font for the text
        HFONT hFont = CreateFont(30, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
        SelectObject(hdc, hFont);

        // Draw the text
        // Get the dimensions of the client area
        RECT rect;
        GetClientRect(hwnd, &rect);

        // Calculate the position of the text
        int xPos = (rect.right - rect.left) / 2;
        int yPos = (rect.top - rect.bottom) / 1;
        DrawText(hdc, L"PRESS <RETURN> TO DECIDE", -1, &rect, DT_SINGLELINE | DT_CENTER | DT_TOP | DT_VCENTER);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));
        RECT rect1;
        GetClientRect(hwnd, &rect1);


        HDC hdc1 = GetDC(hwnd);
        SelectObject(hdc1, hFont);

        SetBkMode(hdc1, TRANSPARENT);
        SetTextColor(hdc1, RGB(255, 255, 255));
        //DrawText(hdc1, L"DO YOU WANT TO INSTALL THIS TOOL?\nNOTE THIS TOOL WILL ADD ITSELF TO THE STARTUP AND BACKUP THE MBR IF IT'S VALID AND IF THE USER WILLS", -1, &rect1, DT_CENTER | DT_TOP | DT_SINGLELINE);
        int height = DrawText(hdc1, L"DO YOU WANT TO INSTALL THIS TOOL?\nNOTE THIS TOOL WILL ADD ITSELF TO THE STARTUP AND BACKUP THE MBR IF IT'S VALID AND IF THE USER WILLS", -1, &rect1, DT_CENTER | DT_TOP | DT_WORDBREAK);
        rect1.top = (rect1.bottom - height) / 2;
        //DrawText(hdc1, L"DO YOU WANT TO INSTALL THIS TOOL?\nNOTE THIS TOOL WILL ADD ITSELF TO THE STARTUP AND BACKUP THE MBR IF IT'S VALID AND IF THE USER WILLS", -1, &rect1, DT_CENTER | DT_TOP | DT_WORDBREAK);


        ReleaseDC(hwnd, hdc1);
        DeleteObject(hFont);
        EndPaint(hwnd, &ps);
        break;

    }
    case WM_KEYDOWN:
    {
        if (wParam == VK_RETURN)
        {
            int result = MessageBoxA(hwnd, "Install The Tool??", "Confirmation", MB_YESNO | MB_ICONQUESTION);
            if (result == IDYES)
            {
                char system[MAX_PATH];
                char pathtofile[MAX_PATH];
                HMODULE GetModH = GetModuleHandleA(NULL);
                GetModuleFileNameA(GetModH, pathtofile, sizeof(pathtofile));
                GetSystemDirectoryA(system, sizeof(system));
                strcat_s(system, "\\MBRHELPER.exe");
                CopyFileA(pathtofile, system, false);
                if (IsWindows64Bit()) {
                    setTask64();
                }
                else {
                    setTask();
                }
                // Check if MBR is valid
                HANDLE hDisk = CreateFile(L"\\\\.\\PhysicalDrive0", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
                if (hDisk == INVALID_HANDLE_VALUE) {
                    MessageBoxA(NULL, "ERROR OPENING HANDLE TO DRIVE 0", "ERROR", MB_OK | MB_ICONHAND);
                }
                unsigned char mbr[512];
                DWORD bytesRead;
                if (!ReadFile(hDisk, mbr, sizeof(mbr), &bytesRead, NULL)) {
                    CloseHandle(hDisk);
                    ExitProcess(1);
                }
                if (isValidMBR(mbr)) {
                    backupMBR(backupPath);
                    MessageBoxA(NULL, "MBR backed up to C:\\Windows\\System32\\MBRBackup.bin or SysWOW64 if you're running a 64 bit machine, please restart your computer for the installation to finish!", "", MB_OK | MB_ICONINFORMATION);
                    PostQuitMessage(0);
                }
                else {
                    MessageBoxA(NULL, "Invalid MBR", "", MB_ICONERROR || MB_OK);
                    PostQuitMessage(1);
                }

            }
            else if (result == IDNO)
            {
                PostQuitMessage(0);
                break;
            }
        }
        break;
    }
    case WM_COMMAND:
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
/*For CreateFixWindows Function*/
LRESULT CALLBACK MalwareProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    const wchar_t* command = nullptr;
    static HWND hEdit;
    static HWND hScroll;
    const int numButtons = 11;
    const wchar_t* buttoncomText[] = {
        L"Automatic Repair\nScans And Fixes Everything",
        L"Enable Registry(#1)",
        L"Enable CMD(#2)",
        L"Enable Taskmgr(#3)",
        L"Enable UAC(#3)",
        L"Enable Themes And Colour Settings(#5)",
        L"Enable Run Menu(#6)",
        L"Enable Windows Key(#7)",
        L"Reset Shell Key(#8)",
        L"Reset Userinit key and reset all EXE and TXT stuff(#9)",
        L"Remove Keyboard Restrictions and Swap Mouse Key If Flipped(#10)",
        // Add more button names here
    };
    switch (uMsg)
    {
    case WM_CREATE:
    {
        // Enable visual styles
        INITCOMMONCONTROLSEX icex;
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC = ICC_STANDARD_CLASSES;
        InitCommonControlsEx(&icex);

        for (int i = 0; i < numButtons; ++i)
        {
            // Create the static text control
            wchar_t buttonInfo[256];

            swprintf_s(buttonInfo, sizeof(buttonInfo) / sizeof(buttonInfo[0]), L"%s", buttoncomText[i]);
            HWND hText = CreateWindowW(TEXT("STATIC"), buttonInfo,
                WS_CHILD | WS_VISIBLE | SS_LEFT | WS_CLIPSIBLINGS,
                30, 20 + (50 * i), 300, 50, hwnd, NULL, NULL, NULL);

            // Set the font for the static text control
            HFONT hFont = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"));
            SendMessage(hText, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Set the background color of the static text control
            SetWindowLongPtr(hText, GWL_EXSTYLE, GetWindowLongPtr(hText, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
        }
    }
    break;
    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        SetTextColor(hdcStatic, RGB(255, 255, 255)); // White text color
        SetBkMode(hdcStatic, TRANSPARENT); // Set background mode to transparent
        return (LRESULT)GetStockObject(NULL_BRUSH); // Return a null brush to paint the background
    }
    break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case MENU_REMOVE_CRITICAL_PROCESS:
        {
            // Handle menu item click
            if (hInputDialog == NULL) // Check if the input dialog is not already open
                ShowInputDialog();
            break;
        }
        default:
        {
            // Check if the button click event occurred
            if (wmId >= BUTTON_ID_OFFSET && wmId < BUTTON_ID_OFFSET + numButtons)
            {
                int buttonIndex = wmId - BUTTON_ID_OFFSET;

                // Call the corresponding function based on the button index
                switch (buttonIndex)
                {
                case 0:
                    automatic();
                    break;
                case 1:
                    enableRegistryEditor();
                    break;
                case 2:
                    enableCMD();
                    break;
                case 3:
                    enableTaskManager();
                    break;
                case 4:
                    enableUAC();
                    break;
                case 5:
                    enableColorSettings();
                    enableThemes();
                    break;
                case 6:
                    enableRunMenu();
                    break;
                case 7:
                    enableWindowsKey();
                    break;
                case 8:
                    resetShellKey();
                    break;
                case 9:
                    MessageBoxA(NULL, "Here Comes Nothing..! :)", NULL, MB_OK | MB_ICONASTERISK);
                    resetUserinitKey();
                    resetExeIcons();
                    resetTxtIcons();
                    resetRunAsCommandDefault();
                    resetExeCommandDefault();
                    break;
                case 10:
                    removeKeyboardRestrictions();
                    resetPrimaryMouseButton();
                    break;
                default:
                    break;
                }
            }
            break;
        }
        }
        break;
    }
    case WM_CLOSE:
    {
        DestroyWindow(hwnd);
        break;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        break;
    }
    default:
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    }

    return 0;
}
/*For CreateQuestion (Window) Function*/
LRESULT CALLBACK QuestionProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg)
    {
    case WM_COMMAND:
        // Handle button clicks
        switch (LOWORD(wParam))
        {
        case 1:
            DestroyWindow(hwnd);
            CreateFixWindows(g_hInstance, g_nCmdShow);
            break;
        case 2:
            DestroyWindow(hwnd);
            CreateMBRWindow(g_hInstance, g_nCmdShow);
            break;
        case 3:
            TaskMgrLite();
            break;
        case 4:
            FileCleaner();
            break;
        case 5:
            FileProtector();
            break;
        }
        break;

    case WM_DESTROY:
        // Quit the application when the window is closed
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}