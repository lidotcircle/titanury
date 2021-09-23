#include <cxxopts.hpp>
#include <string>
#include <iostream>
#include <TitanEngine.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "transcript.h"
using namespace std;


std::mutex at_m;
std::condition_variable at_cv;
static bool attached = false;
void attach_hello() {
    cout << "attached" << endl;
    attached = true;
    at_cv.notify_one();
}

static int n = 0;
void hwbp_hello(void* ptr) {
    cout << "0x" << ptr << " hardware breakpoint hit " << n << endl;
    n++;
}

int main (int argc, char** argv) {
    int pid = 0;
    string processName;
    string module;
    string funcname;

    cxxopts::Options options("setHWBP", "attach process and set a hardware breakpoint");
    options.add_options()
        ("p,pid", "target process pid", cxxopts::value<int>(pid), "<pid>")
        ("n,pname", "process name", cxxopts::value<string>(processName), "<process name>")
        ("m,module", "module name eg. kernel32.dll", cxxopts::value<string>(module)->default_value("kernel32.dll"), "<module>")
        ("f,function", "function name of the module", cxxopts::value<string>(funcname)->default_value("ReadFile"), "<func>")
        ("h,help", "print this help");
    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        cout << options.help();
        return 0;
    }

    if (!processName.empty()) {
        pid = GetPIDByProcessName(processName);
        if (!pid) {
            cout << "can't find process with 'name = " << processName << "' failed" << endl;
        }
    }

    if (pid == 0) {
        cout << options.help();
        return 1;
    }

    auto debuggerLoop = std::thread([](int pid) {
        if (!TE::UE::AttachDebugger(pid, true, nullptr, attach_hello)) {
            cout << "attach process with 'pid = " << pid << "' failed" << endl;
            at_cv.notify_all();
            return 1;
        }

        return 0;
    }, pid);

    std::unique_lock<std::mutex> lock(at_m);
    at_cv.wait(lock, []() {return attached;});
    auto funcaddr = TE::UE::ImporterGetRemoteAPIAddressEx(const_cast<char*>(module.c_str()), const_cast<char*>(funcname.c_str()));
    if (!funcaddr) {
        cout << "Get Debuggee address of " << module << ":" << funcname << " failed" << endl;
        return 1;
    }

    if (!TE::UE::SetHardwareBreakPoint(funcaddr, TE::UE::UE_DR0, TE::UE::UE_HARDWARE_EXECUTE, TE::UE::UE_HARDWARE_SIZE_1, hwbp_hello)) {
        cout << "Set HWBP at " << module << ":" << funcname << " failed" << endl;
        return 1;
    }

    cout << "set HWBP at " << module << ":" << funcname << endl;
    debuggerLoop.join();

    return 0;
}
