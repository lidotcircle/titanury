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
static bool attachFinish = false;
static bool attachSuccess = false;
void attach_hello() {
    cout << "attached" << endl;
    attachFinish = true;
    attachSuccess = true;
    at_cv.notify_one();
}

int main (int argc, char** argv) {
    int pid = 0;
    string processName;
    int attach_ms = 0;

    cxxopts::Options options("setHWBP", "attach process and set a hardware breakpoint");
    options.add_options()
        ("p,pid", "target process pid", cxxopts::value<int>(pid), "<pid>")
        ("n,pname", "process name", cxxopts::value<string>(processName), "<process name>")
        ("ms", "delay from attach to detach, 0 means waiting user hit <enter>", cxxopts::value<int>(attach_ms)->default_value("0"), "<millisecond>")
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
            attachFinish = true;
            at_cv.notify_all();
            return 1;
        }

        return 0;
    }, pid);

    std::unique_lock<std::mutex> lock(at_m);
    at_cv.wait(lock, []() {return attachFinish;});

    if (!attachSuccess)
        return 1;

    if (attach_ms == 0) {
        std::cout << "press ENTER key to detach...";
        for (char c = 0; c != '\n'; c = std::cin.get());
    } else {
        std::this_thread::sleep_for(attach_ms * 1ms);
    }

    TE::UE::DetachDebugger(pid);
    debuggerLoop.join();

    return 0;
}
