#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <cxxopts.hpp>
using namespace std;


int main(int argc, char *argv[])
{
    string input1;
    string input2;
    size_t beg1, beg2;
    size_t len;
    long offset;

    cxxopts::Options options(argv[0], "translate decimal (preceding and following with boundary character, eg. []()<>\\t\\n) into hexadecimal");
    options.add_options()
        ("in1", "input file 1", cxxopts::value<string>(input1), "<file>")
        ("in2", "input file 2", cxxopts::value<string>(input2), "<file>")
        ("beg1", "start offset of file 1", cxxopts::value<size_t>(beg1))
        ("beg2", "start offset of file 2", cxxopts::value<size_t>(beg2))
        ("len", "comparison length", cxxopts::value<size_t>(len))
        ("off", "base offset", cxxopts::value<long>(offset))
        ("h,help", "print help");

    cxxopts::ParseResult result;
    try {
        result = options.parse(argc, argv);
    } catch (std::exception e) {
        cerr << "bad arguments" << endl;
        cerr << options.help() << endl;
        return 1;
    }

    if (result.count("help")) {
        cout << options.help() << endl;
        return 0;
    }

    if (result.count("in1") == 0) {
        cerr << "input file 1 is not specified" << endl;
        cerr << options.help() << endl;
        return 1;
    }

    if (result.count("in2") == 0) {
        cerr << "input file 2 is not specified" << endl;
        cerr << options.help() << endl;
        return 1;
    }

    if (result.count("beg1") == 0) {
        cerr << "start offset of file 1 is not specified" << endl;
        cerr << options.help() << endl;
        return 1;
    }

    if (result.count("beg2") == 0) {
        cerr << "start offset of file 2 is not specified" << endl;
        cerr << options.help() << endl;
        return 1;
    }

    if (result.count("len") == 0) {
        cerr << "comparison length is not specified" << endl;
        cerr << options.help() << endl;
        return 1;
    }

    if (result.count("off") == 0) {
        cerr << "base offset is not specified" << endl;
        cerr << options.help() << endl;
        return 1;
    }

    ifstream ifs1(input1, ios::binary);
    if (!ifs1) {
        cerr << "failed to open file " << input1 << endl;
        return 1;
    }

    ifstream ifs2(input2, ios::binary);
    if (!ifs2) {
        cerr << "failed to open file " << input2 << endl;
        return 1;
    }

    ifs1.seekg(beg1, ios::beg);
    ifs2.seekg(beg2, ios::beg);

    if (ifs1.tellg() != beg1) {
        cerr << "failed to seek to " << beg1 << " in file " << input1 << endl;
        return 1;
    }

    if (ifs2.tellg() != beg2) {
        cerr << "failed to seek to " << beg2 << " in file " << input2 << endl;
        return 1;
    }

    if (len == 0) {
        cerr << "comparison length is 0" << endl;
        return 1;
    }

    vector<char> buf1(len), buf2(len);
    try {
        ifs1.read(buf1.data(), len);
        if (ifs1.tellg() != beg1 + len)
            throw runtime_error("failed to read " + to_string(len) + " bytes from " + input1);
    } catch (std::exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    try {
        ifs2.read(buf2.data(), len);
        if (ifs2.tellg() != beg2 + len)
            throw runtime_error("failed to read " + to_string(len) + " bytes from " + input2);
    } catch (std::exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    for (size_t i = 0; i < len; i++) {
        unsigned char c1 = buf1[i];
        unsigned char c2 = buf2[i];

        if (c1 == c2)
            continue;
        
        uint32_t* p1 = (uint32_t*)&buf1[i];
        uint32_t* p2 = (uint32_t*)&buf2[i];

        if ((*p1 - *p2) == offset) {
            i += 3;
            continue;
        }
        
        cout << "file offset 0x" << setw(8) << setfill('0') << hex << beg1 + i;
        cout << ": " << setw(2) << setfill('0') << hex << (uint32_t)c1 << " | " << setw(2) << setfill('0') << hex << (uint32_t)c2 << endl;
    }

    return 0;
}
