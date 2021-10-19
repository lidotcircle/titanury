#include <cxxopts.hpp>
#include <string>
#include <iostream>
#include <sstream>
#include <thread>
#include <Windows.h>
#include <TlHelp32.h>
#include "transcript.h"
using namespace std;

BYTE* dwGetModuleBaseAddress(DWORD dwProcessID, const string& moduleName)
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

bool GetMemoryExpr(DWORD pid, HANDLE pHandle, const std::string& expr, DWORD* out) {
    auto kvkv = string_split(expr, "+");
    void* value = (void*)dwGetModuleBaseAddress(pid, kvkv[0]);
    if (value == nullptr) {
        std::cerr << "can't get module base address '" << kvkv[0] << "'" << endl;
        return false;
    }
    kvkv.erase(kvkv.begin());

    for(auto& v: kvkv) {
        int intv = 0;
        stringstream ss;
        ss << std::hex << v;
        ss >> intv;

        if (!ReadProcessMemory(pHandle, (void*)((int)value + intv), &value, 4, nullptr)) {
            cerr << "read expr '" << expr << "' failed, at 0x" << value << endl;
            return false;
        }
    }

    *out = (DWORD)value;
    return true;
}

int main (int argc, char** argv) {
    string procname;
    int pid = -1;
    vector<string> exprs;

    cxxopts::Options options(argv[0], "read process memory");
    options.add_options()
        ("p,pid",  "target process id", cxxopts::value<int>(pid), "<pid>")
        ("n,procname", "target process name", cxxopts::value<string>(procname), "<procname>")
        ("e,exprs", "memory expressions", cxxopts::value<vector<string>>(exprs), "<exprs>")
        ("h,help", "print this help");
    options.parse_positional("exprs");
    auto result = options.parse(argc, argv);

    if (!result.unmatched().empty()) {
        cerr << options.help();
        return 1;
    }

    if (result.count("help") > 0) {
        std::cout << options.help();
        return 0;
    }

    if (pid <= 0) {
        if (procname.empty()) {
            cerr << "specify pid or process name" << endl;
            cerr << options.help();
            return 1;
        }

        pid = GetPIDByProcessName(procname);
    }

    if (pid <= 0) {
        cerr << "can't find process " << procname << endl;
        return 1;
    }

    if (exprs.empty()) {
        cerr << "specify at least one memory expression" << endl;
        return 1;
    }

    auto ph = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, false, pid);
    if (ph == INVALID_HANDLE_VALUE) {
        cerr << "can't open target process" << endl;
        return 1;
    }

    vector<pair<string, DWORD>> valpairs;
    size_t ml = 0;
    for (auto& expr: exprs) {
        DWORD val = 0;
        if (!GetMemoryExpr(pid, ph, expr, &val)) {
            std::cerr << "!!! Get memory expression '" << expr << "' failed." << endl;
            continue;
        }

        if (expr.size() > ml)
            ml = expr.size();
        valpairs.push_back(make_pair(expr, val));
    }

    for (auto& kv: valpairs) {
        std::cout << "MemoryExpression['" << kv.first << "'] " << string(ml - kv.first.size(), ' ')
            << "= 0x" << std::hex << kv.second << endl;
    }

    return 0;
}
