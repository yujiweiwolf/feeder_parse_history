#include <mutex>
#include <thread>

#include <x/x.h>
#include <coral/coral.h>

#include "config.h"


namespace co {

    Config* Config::instance_ = 0;

    Config* Config::Instance() {
        static std::once_flag flag;
        std::call_once(flag, [&]() {
            if (instance_ == 0) {
                instance_ = new Config();
                instance_->Init();
            }
        });
        return instance_;
    }

    void Config::Init() {
        string filename = x::FindFile("config.ini");
        x::INI ini = x::Ini(filename);
        opt_ = QOptions::Load(filename);

        data_dir_ = ini.get<string>("qts.data_dir");
        parse_date_ = ini.get<string>("qts.parse_date");
        market_ = ini.get<int>("qts.market");
        stringstream ss;
        ss << "+-------------------- configuration begin --------------------+" << endl;
        ss << opt_->ToString() << endl;
        ss << endl;
        ss << "qts:" << endl
            << "data_dir: " << data_dir_ << endl
            << "parse_date: " << parse_date_ << endl
            << "market: " << market_ << endl;
        __info << endl << ss.str();
    }
}