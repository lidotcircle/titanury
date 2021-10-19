#include "transcript.h"
#include <string>
#include <vector>
#include <Windows.h>
#include <tlhelp32.h>
using namespace std;


int GetPIDByProcessName(const std::string& processName)
{
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32))
    {
        wprintf(L"Error getting first process\n");
        CloseHandle(hProcessSnap);
        return 0;
    }

    DWORD pid = 0;

    do
    {
        if (_stricmp(pe32.szExeFile, processName.c_str()))
        {
            pid = pe32.th32ProcessID;
            break;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return pid;
}

std::string GetLastErrorAsString()
{
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0)
        return std::string();
    
    LPSTR messageBuffer = nullptr;

    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    
    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
            
    return message;
}

void* GetModuleBaseAddress(int dwProcessID, const string& moduleName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessID);
    BYTE* dwModuleBaseAddress = NULL;
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        MODULEENTRY32 ModuleEntry32 = { 0 };
        ModuleEntry32.dwSize = sizeof(MODULEENTRY32);
        if (Module32First(hSnapshot, &ModuleEntry32))
        {
            do
            {
                if (_stricmp(ModuleEntry32.szModule, moduleName.c_str()) == 0)
                {
                    dwModuleBaseAddress = ModuleEntry32.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnapshot, &ModuleEntry32));
        }
        CloseHandle(hSnapshot);
    }
    return dwModuleBaseAddress;
}

vector<string> string_split(const string &str, const string& delim) {
    size_t off = 0;
    size_t pos = 0;
    vector<string> ans;
    while ((pos = str.find(delim, off)) != std::string::npos)
    {
        ans.push_back(str.substr(off, pos - off));
        off = pos + delim.size();
    }

    if (off < str.size())
        ans.push_back(str.substr(off));

    return ans;
}
