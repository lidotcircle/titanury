#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <set>
#include <cxxopts.hpp>
using namespace std;


const static set<char> hexdigits = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f',
    'A', 'B', 'C', 'D', 'E', 'F'
};

int hexstr2int(const string& hexstr) {
    int result = 0;
    for (char c : hexstr) {
        if (hexdigits.find(c) == hexdigits.end()) {
            throw runtime_error("invalid hex string");
        }

        if (c <= '9' && c >= '0')
            result = result * 16 + (c - '0');
        else if (c <= 'F' && c >= 'A')
            result = result * 16 + (c - 'A' + 10);
        else
            result = result * 16 + (c - 'a' + 10);
    }
    return result;
}

int main(int argc, char *argv[])
{
    string input_file;
    string output_file;

    cxxopts::Options options(argv[0], "translate hex number (start with 0x) to decimal in text file");
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
     * 0: sparse
     * 1: get 0
     * 2: get x
     * 3: read digits
     */
    int state = 0;
    std::string hdstr;
    while (!in->eof()) {
        char c = in->get();

        switch (state) {
            case 0:
            {
                if (c == '0')
                    state = 1;
                else
                    out->write(&c, 1);
            } break;
            case 1:
            {
                if (c == 'x') {
                    state = 2;
                } else {
                    out->write("0", 1);
                    out->write(&c,  1);
                    state = 0;
                }
            } break;
            case 2:
            {
                if (hexdigits.find(c) != hexdigits.end()) {
                    hdstr += c;
                    state = 3;
                } else {
                    out->write("0x", 2);
                    out->write(&c, 1);
                    state = 0;
                }
            } break;
            case 3:
            {
                if (hexdigits.find(c) != hexdigits.end()) {
                    hdstr += c;
                } else {
                    int v = hexstr2int(hdstr);
                    hdstr.clear();
                    *out.get() << v;
                    out->write(&c, 1);
                    state = 0;
                }
            } break;
            default:
                throw std::runtime_error("bad state");
        }
    }

    if (state == 3) {
        int v = hexstr2int(hdstr);
        hdstr.clear();
        *out.get() << v;
    }

    return 0;
}
