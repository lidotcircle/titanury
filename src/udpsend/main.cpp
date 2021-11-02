#include <WinSock2.h>
#include <Windows.h>
#include <cxxopts.hpp>
#include <string>
#include <vector>
#include <thread>
#include "transcript.h"
#pragma comment(lib, "ws2_32.lib")
using namespace std;

int recv_and_print(SOCKET sock) {
    while (true) {
        char buf[0x10000];
        if (recv(sock, buf, sizeof(buf), 0) == SOCKET_ERROR) {
            cerr << "recv error: " << GetLastErrorAsString() << endl;
            exit(-1);
        }

        size_t len = strnlen(buf, sizeof(buf));
        string message(buf, len);
        cout << endl << "> " << message << endl << "< ";
    }

    return 0;
}

int main(int argc, char* argv[]) {
    WSAData wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    bool reply = false;
    bool print = false;
    uint16_t remoteport = 0;
    string remoteaddr;

    cxxopts::Options options(argv[0], "send lines from stdin to udp address");
    options.allow_unrecognised_options();
    options.add_options()
        ("p,port", "listening port", cxxopts::value<uint16_t>(remoteport)->default_value("7710"), "<port>")
        ("a,addr", "bind address of listening udp socket", cxxopts::value<string>(remoteaddr)->default_value("127.0.0.1"), "<addr>")
        ("print",  "print recieved message", cxxopts::value<bool>(print))
        ("h,help", "print help");
    auto result = options.parse(argc, argv);

    if (!result.unmatched().empty()) {
        cerr << options.help();
        return 1;
    }

    if (result.count("help") > 0) {
        std::cout << options.help();
        return 0;
    }

    sockaddr_in saddr;
    auto uport = htons(remoteport);
    auto uaddr = inet_addr(remoteaddr.c_str());
    saddr.sin_family = AF_INET;
    saddr.sin_addr = *((LPIN_ADDR)&uaddr);
    saddr.sin_port = uport;

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    {
        sockaddr_in saddr;
        auto uaddr = inet_addr("0.0.0.0");
        saddr.sin_family = AF_INET;
        saddr.sin_addr = *((LPIN_ADDR)&uaddr);
        saddr.sin_port = 0;
        if (bind(sock, (sockaddr*)&saddr, sizeof(saddr)) == SOCKET_ERROR) {
            cerr << "bind error: " << GetLastErrorAsString() << endl;
            return 1;
        }

        int len = sizeof(saddr);
        if (getsockname(sock, (sockaddr*)&saddr, &len) == SOCKET_ERROR) {
            cerr << "getsockname error: " << GetLastErrorAsString() << endl;
            return 1;
        }

        cout << "bind " << inet_ntoa(saddr.sin_addr) << ":" << ntohs(saddr.sin_port) << endl;
    }

    cout << "sendto " << remoteaddr << ":" << remoteport << endl << "< ";

    std::thread recv_thread;
    if (print)
        recv_thread = std::thread(recv_and_print, sock);

    string line;
    while (cin >> line) {
        size_t len;
        if ((len = sendto(sock, line.c_str(), line.size(), 0, (sockaddr*)&saddr, sizeof(saddr))) != line.size()) {
            cerr << "sendto failed: " << GetLastErrorAsString() << endl;
            return 1;
        }

        cout << "< ";
    }

    return 0;
}