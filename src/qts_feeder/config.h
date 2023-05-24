#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include "feeder/feeder.h"

using namespace std;

namespace co {
    class Config {
    public:
        static Config* Instance();

        inline string data_dir() {
            return data_dir_;
        }
        inline string parse_date() {
            return parse_date_;
        }
        inline int market() {
            return market_;
        }
        inline QOptionsPtr opt() {
            return opt_;
        }

    protected:
        Config() = default;
        ~Config() = default;
        Config(const Config&) = delete;
        const Config& operator=(const Config&) = delete;

        void Init();

    private:
        static Config* instance_;

        QOptionsPtr opt_;
        string data_dir_;
        string parse_date_;
        int market_ = 0;
    };
}