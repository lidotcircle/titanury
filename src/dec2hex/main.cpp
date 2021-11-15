#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <memory>
#include <set>
#include <ctype.h>
#include <cxxopts.hpp>
using namespace std;


const static set<char> boundary_chars = {
    ' ', '\t', '\n',
    '\x01', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07', '\x08', '\x09', '\x0a', '\x0b', '\x0c', '\x0d', '\x0e', '\x0f',
    '\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17', '\x18', '\x19', '\x1a', '\x1b', '\x1c', '\x1d', '\x1e', '\x1f',
    '\x7f', '(', ')', '[', ']', '<', '>',
};

int main(int argc, char *argv[])
{
    string input_file;
    string output_file;

    cxxopts::Options options(argv[0], "translate decimal (preceding and following with boundary character, eg. []()<>\\t\\n) into hexadecimal");
    options.add_options()
        ("i,in", "input file, if not specified read from stdin", cxxopts::value<string>(input_file), "<file>")
        ("o,out", "output file, if not specified write to stdout", cxxopts::value<string>(output_file), "<file>")
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

    auto cin_delete  = [](std::istream* p) {};
    auto cout_delete = [](std::ostream* p) {};

    std::shared_ptr<std::istream> in   = (input_file.empty())  ? 
        std::shared_ptr<std::istream>(&std::cin, cin_delete)   : std::make_shared<std::ifstream>(input_file, std::ios::binary);

    std::shared_ptr<std::ostream> out  = (output_file.empty()) ? 
        std::shared_ptr<std::ostream>(&std::cout, cout_delete) : std::make_shared<std::ofstream>(output_file, std::ios::binary);

    /*
     * 0: nope
     * 1: boundary
     * 2: number
     */
    int state = 1;
    std::string hdstr;
    while (!in->eof()) {
        char c = in->get();

        switch (state) {
            case 0:
            {
                if (boundary_chars.find(c) != boundary_chars.end())
                    state = 1;
                out->put(c);
            } break;
            case 1:
            {
                if (isdigit(c)) {
                    hdstr.push_back(c);
                    state = 2;
                } else {
                    out->put(c);
                    state = 0;
                }
            } break;
            case 2:
            {
                if (isdigit(c)) {
                    hdstr.push_back(c);
                    state = 2;
                } else if (boundary_chars.find(c) != boundary_chars.end()) {
                    int v = std::stoi(hdstr, nullptr, 10);
                    *out << "0x" << std::hex << v;
                    out->put(c);
                    state = 1;
                    hdstr.clear();
                } else {
                    out->write(hdstr.c_str(), hdstr.size());
                    out->put(c);
                    state = 0;
                    hdstr.clear();
                }
            } break;
            default:
                throw std::runtime_error("bad state");
        }
    }

    if (state == 2) {
        int v = stoi(hdstr);
        hdstr.clear();
        *out.get() << "0x" << std::hex << v;
    }

    return 0;
}
