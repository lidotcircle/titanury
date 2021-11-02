#include <WinSock2.h>
#include <Windows.h>
#include <cxxopts.hpp>
#include <string>
#include <vector>
#include "transcript.h"
#pragma comment(lib, "ws2_32.lib")
using namespace std;


int main(int argc, char* argv[]) {
    WSAData wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    char buf[0x10000];
    bool reply = false;
    bool noprint = false;
    uint16_t udpport = 0;
    string bindaddr;

    cxxopts::Options options(argv[0], "recv udp message and print it");
    options.add_options()
        ("p,port",  "listening port", cxxopts::value<uint16_t>(udpport), "<port>")
        ("a,addr",  "bind address of listening udp socket", cxxopts::value<string>(bindaddr)->default_value("0.0.0.0"), "<addr>")
        ("noprint", "don't print recieved message", cxxopts::value<bool>(noprint))
        ("r,reply", "reply remote end with same message", cxxopts::value<bool>(reply))
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

    auto uport = htons(udpport);
    auto uaddr = inet_addr(bindaddr.c_str());

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_addr = *((LPIN_ADDR)&uaddr);
    saddr.sin_port = uport;
    if (bind(sock, (sockaddr*)(&saddr), sizeof(sockaddr_in)) == SOCKET_ERROR) {
        cerr << "bind error: " << GetLastErrorAsString() << endl;
        return 1;
    }
    int len = sizeof(sockaddr_in);
    getsockname(sock, (sockaddr*)(&saddr), &len);
    udpport = ntohs(saddr.sin_port);

    cout << "listening on " << bindaddr << ":" << udpport << endl;
    while (true) {
        sockaddr_in sfrom;
        int fromlen = sizeof(sfrom);
        int len = recvfrom(sock, buf, sizeof(buf), 0, (sockaddr*)&sfrom, &fromlen);
        if (len == SOCKET_ERROR) {
            cerr << "recvfrom error: " << GetLastErrorAsString() << endl;
            return 1;
        }

        if (!noprint) {
            string remoteaddr(inet_ntoa(saddr.sin_addr));
            uint16_t remoteport = ntohs(saddr.sin_port);
            auto l = strnlen(buf, len);
            cout << remoteaddr << ":" << remoteport << " => " << string(buf, l) << endl;
        }

        if (reply) {
            if (sendto(sock, buf, len, 0, (sockaddr*)&sfrom, fromlen) == SOCKET_ERROR) {
                cerr << "sendto error: " << GetLastErrorAsString() << endl;
                return 1;
            }
        }
    }

    return 0;
}