#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <thread>
#include <x/x.h>
#include "config.h"

namespace co {
class QtsServer {
 public:
    void Run();
    void Stop();

 private:
    void GetAllFile();
    void ParseSHFEHistoryData();
    void ParseDCEHistoryData();
    // 判断是否星期五
    bool IsFriday(int64_t date);

private:
    vector<string> all_files_;
    // shared_ptr<co::fbs::QTickT>
    std::map<std::string, shared_ptr<co::fbs::QTickT>> czce_data_;
    std::set<std::string> first_czce_code_;
};
}
