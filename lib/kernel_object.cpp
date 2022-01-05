#include "kernel_object.h"
#include "ntdll.h"
#include <Windows.h>
#include <string>
#include <vector>
#include <locale>
#include <codecvt>
#include <memory>
#include <exception>
#include <stdexcept>
using namespace std;

typedef struct _OBJECT_DIRECTORY_INFORMATION {
    UNICODE_STRING Name;
    UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, *POBJECT_DIRECTORY_INFORMATION;


vector<KernelObjectInfo> get_kernel_object_list(const std::string& path) {
    wstring wpath = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(path);
    vector<KernelObjectInfo> result;

    HANDLE handle;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING str;
    RtlInitUnicodeString(&str, const_cast<PWSTR>(wpath.c_str()));

    InitializeObjectAttributes(&objectAttributes, &str, OBJ_CASE_INSENSITIVE, NULL, NULL);
    if (!NT_SUCCESS(NtOpenDirectoryObject(&handle, DIRECTORY_QUERY, &objectAttributes)))
        throw std::runtime_error("NtOpenDirectoryObject '" + path + "' failed");

    ULONG queryContext = 0, rLength;
    do {
        std::shared_ptr<OBJECT_DIRECTORY_INFORMATION> pObjDirInfo;
        auto status = NtQueryDirectoryObject(handle, nullptr, 0, TRUE, FALSE, &queryContext, &rLength);

        if (status != STATUS_BUFFER_TOO_SMALL)
            break;

        pObjDirInfo = std::shared_ptr<OBJECT_DIRECTORY_INFORMATION>(
            (POBJECT_DIRECTORY_INFORMATION)new char[rLength],
            [](POBJECT_DIRECTORY_INFORMATION p) { delete[] (char*)p; }
        );
        status = NtQueryDirectoryObject(handle, pObjDirInfo.get(), rLength, TRUE, FALSE, &queryContext, &rLength);

        std::wstring wname(pObjDirInfo->Name.Buffer, pObjDirInfo->Name.Length / sizeof(WCHAR));
        std::wstring wtype(pObjDirInfo->TypeName.Buffer, pObjDirInfo->TypeName.Length / sizeof(WCHAR));
        auto name = wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(wname);
        auto type = wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(wtype);

        if (NT_SUCCESS(status)) {
            result.push_back({ name, type });
        } else {
            break;
        }
    } while (true);

    NtClose(handle);
    return result;
}


std::string get_kernel_object_type(const std::string& path) {
    if (path == "\\") {
        return "Directory";
    }

    auto last_slash = path.find_last_of("\\");
    if (last_slash == string::npos) {
        throw std::runtime_error("Invalid path");
    }

    auto parent_path = path.substr(0, last_slash);
    if (parent_path.empty())
        parent_path = "\\";
    auto siblings = get_kernel_object_list(parent_path);

    for (auto& sibling : siblings) {
        if (sibling.name == path.substr(last_slash + 1)) {
            return sibling.type;
        }
    }

    throw std::runtime_error("Invalid path");
}
