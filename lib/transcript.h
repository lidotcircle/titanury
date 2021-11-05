#ifndef _TRANSCRIPT_H_
#define _TRANSCRIPT_H_

#include <string>
#include <vector>
#include "kernel_object.h"


int GetPIDByProcessName(const std::string& processName);
void* GetModuleBaseAddress(int dwProcessID, const std::string& moduleName);

std::string GetLastErrorAsString();

std::vector<std::string> string_split(const std::string &str, const std::string& delim);

#endif // _TRANSCRIPT_H_