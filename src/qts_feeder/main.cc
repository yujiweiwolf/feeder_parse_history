#include <iostream>
#include <sstream>
#include <stdio.h>
#include <boost/program_options.hpp>
#include <x/x.h>
#include "define.h"
#include "define.h"
#include "config.h"
#include "qts_server.h"

using namespace std;
using namespace co;
namespace po = boost::program_options;

const string kVersion = "v1.0.5";

int main(int argc, char* argv[]) {
    try {
        po::options_description desc("[qts_feeder] Usage");
        desc.add_options()
            ("passwd", po::value<std::string>(), "encode plain password")
            ("help,h", "show help message")
            ("version,v", "show version information");
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
        if (vm.count("passwd")) {
            cout << co::EncodePassword(vm["passwd"].as<std::string>()) << endl;
            return 0;
        } else if (vm.count("help")) {
            cout << desc << endl;
            return 0;
        } else if (vm.count("version")) {
            cout << kVersion << endl;
            return 0;
        }
        __info << "start insight feeder: version = " << kVersion << " ...";
        Config::Instance();
        QtsServer server;
        server.Run();
        __info << "server is stopped.";
    } catch (std::exception& e) {
        __fatal << "server is crashed, " << e.what();
        return 2;
    } catch (...) {
        __fatal << "server is crashed, unknown reason";
        return 3;
    }
    return 0;
}

