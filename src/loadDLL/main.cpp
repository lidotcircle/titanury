#include <cxxopts.hpp>
#include <string>
#include <iostream>
#include <thread>
#include <Windows.h>
#include "transcript.h"
using namespace std;


int main (int argc, char** argv) {
    string dllname;
    std::vector<string> entrypoint;
    unsigned int wait_ms = 0;

    cxxopts::Options options(argv[0], "load a single dynamic-link library");
    options.add_options()
        ("n,dll",  "dynamic-link library name eg. kernel32.dll", cxxopts::value<string>(dllname), "<dll>")
        ("w,wait", "wait n ms to load library", cxxopts::value<unsigned int>(wait_ms)->default_value(0), "<milliseconds>")
        ("e,entrypoint", "call entrypoint with <argument> if this option presents", cxxopts::value<vector<string>>(entrypoint), "<entrypoint> [ <argument> ]")
        ("h,help", "print this help");
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        cout << options.help();
        return 0;
    }

    if (dllname.empty()) {
        cerr << options.help();
        return 1;
    }

    if (entrypoint.size() > 2) {
        cerr << "ERROR: entrypoint function only accept a string argument (can be null)" << endl;
        cerr << options.help();
        return 1;
    }

    if (wait_ms > 0) {
        cout << "waiting " << wait_ms << "ms" << endl;
        std::this_thread::sleep_for(wait_ms * 1ms);
    }

    auto dllH = LoadLibraryA(dllname.c_str());
    if (!dllH) {
        cerr << "load '" << dllname << "' failed: " << GetLastErrorAsString() << endl;
        return 1;
    }

    auto pid = GetCurrentProcessId();
    cout << "pid = " << pid << ", handle = 0x" << dllH << endl;

    typedef void (*entrypoint_t)(const char*);
    if (!entrypoint.empty()) {
        entrypoint_t e = (entrypoint_t)GetProcAddress(dllH, entrypoint[0].c_str());
        if (e == nullptr) {
            cerr << "can't get entrypoint function '" << entrypoint[0] << "'" << endl;
            return 1;
        }

        const char* arg = nullptr;
        if (entrypoint.size() > 1) arg = entrypoint[1].c_str();

        cout << "calling function " << entrypoint[0] << "()" << endl;
        e(arg);
    }

    cout << "press ENTER key to exit ..." << endl;
    for (char c=0;c!='\n';c=cin.get());

    FreeLibrary(dllH);
    return 0;
}

