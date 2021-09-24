#include <cxxopts.hpp>
#include <string>
#include <iostream>
#include <Windows.h>
#include "transcript.h"
using namespace std;


int main (int argc, char** argv) {
    string dllname;

    cxxopts::Options options(argv[0], "load a single dynamic-link library");
    options.add_options()
        ("n,dll",  "dynamic-link library name eg. kernel32.dll", cxxopts::value<string>(dllname), "<dll>")
        ("h,help", "print this help");
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        cout << options.help();
        return 0;
    }

    if (dllname.empty()) {
        cout << options.help();
        return 1;
    }

    auto dllH = LoadLibraryA(dllname.c_str());
    if (!dllH) {
        cout << "load '" << dllname << "' failed: " << GetLastErrorAsString() << endl;
        return 1;
    }

    auto pid = GetCurrentProcessId();
    cout << "pid = " << pid << ", press ENTER key to exit ..." << endl;
    for (char c=0;c!='\n';c=cin.get());

    FreeLibrary(dllH);
    return 0;
}

