#include <Windows.h>
#include <cxxopts.hpp>
#include <iostream>
#include <vector>
#include "transcript.h"
using namespace std;


static void kernel_object_ls(const vector<string> &dirs, const string& type, bool show_type)
{
    for (auto &dir : dirs)
    {
        if (dirs.size() > 1)
            cout << dir << ":" << endl;

        auto list = get_kernel_object_list(dir);
        size_t ml = 0;
        for (auto &obj : list)
            ml = max(ml, obj.name.size());
        ml += 4;

        for (auto &n : list)
        {
            if (!type.empty() && n.type != type)
                continue;

            cout << n.name;
            if (show_type)
                cout << string(ml - n.name.size(), ' ') << n.type;
            cout << endl;
        }

        if (dirs.size() > 1)
            cout << endl;
    }
}

int main(int argc, char* argv[]) {
    string path;
    string type;
    bool show_type = false;
    vector<string> dirs;

    cxxopts::Options options(argv[0], "send lines from stdin to udp address");
    options.add_options()
        ("q,query", "query object detailed information", cxxopts::value<string>(path), "<object>")
        ("a,show-type", "show kernel object type", cxxopts::value<bool>(show_type))
        ("t,type", "only show specified type", cxxopts::value<string>(type), "<type>")
        ("directories", "directories", cxxopts::value<vector<string>>(dirs))
        ("h,help", "print help");

    options.parse_positional("directories");
    options.positional_help("[directories]");
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

    if (!path.empty()) {
        try {
        auto type = get_kernel_object_type(path);
        cout << type << endl;
        return 0;
        } catch (std::exception e) {
            cerr << "failed to get kernel object type: " << e.what() << endl;
            return 1;
        }
    }

    if (dirs.empty()) {
        cerr << "specify object directory" << endl;
        cerr << options.help() << endl;
        return 1;
    }

    try
    {
        kernel_object_ls(dirs, type, show_type);
        return 0;
    }
    catch (std::runtime_error &e)
    {
        cerr << e.what() << endl;
        return 1;
    }

    return 0;
}
