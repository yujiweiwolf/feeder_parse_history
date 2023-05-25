#include <mutex>
#include <thread>

#include <x/x.h>
#include <coral/coral.h>

#include "config.h"
#include "yaml-cpp/yaml.h"

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
        // 读取配置
        auto getStr = [&](const YAML::Node& node, const std::string& name) {
            try {
                return node[name] && !node[name].IsNull() ? node[name].as<std::string>() : "";
            } catch (std::exception& e) {
                LOG_ERROR << "load configuration failed: name = " << name << ", error = " << e.what();
                throw std::runtime_error(e.what());
            }
        };
        auto getStrings = [&](std::vector<std::string>* ret, const YAML::Node& node, const std::string& name, bool drop_empty = false) {
            try {
                if (node[name] && !node[name].IsNull()) {
                    for (auto item : node[name]) {
                        std::string s = x::Trim(item.as<std::string>());
                        if (!drop_empty || !s.empty()) {
                            ret->emplace_back(s);
                        }
                    }
                }
            } catch (std::exception& e) {
                LOG_ERROR << "load configuration failed: name = " << name << ", error = " << e.what();
                throw std::runtime_error(e.what());
            }
        };
        auto getInt = [&](const YAML::Node& node, const std::string& name, const int64_t& default_value = 0) {
            try {
                return node[name] && !node[name].IsNull() ? node[name].as<int64_t>() : default_value;
            } catch (std::exception& e) {
                LOG_ERROR << "load configuration failed: name = " << name << ", error = " << e.what();
                throw std::runtime_error(e.what());
            }
        };
        auto getBool = [&](const YAML::Node& node, const std::string& name) {
            try {
                return node[name] && !node[name].IsNull() ? node[name].as<bool>() : false;
            } catch (std::exception& e) {
                LOG_ERROR << "load configuration failed: name = " << name << ", error = " << e.what();
                throw std::runtime_error(e.what());
            }
        };

        auto filename = x::FindFile("feeder.yaml");
        YAML::Node root = YAML::LoadFile(filename);
        opt_ = QOptions::Load(filename);

        auto wind_feeder = root["qts"];
        data_dir_ = getStr(wind_feeder, "data_dir");
        parse_date_ = getStr(wind_feeder, "parse_date");
        market_ = getInt(wind_feeder,"market");
        stringstream ss;
        ss << "+-------------------- configuration begin --------------------+" << endl;
        ss << opt_->ToString() << endl;
        ss << endl;
        ss << "qts:" << endl
            << "data_dir: " << data_dir_ << endl
            << "parse_date: " << parse_date_ << endl
            << "market: " << market_ << endl;
        __info << endl << ss.str();
        __warn << endl << ss.str();
        __error << endl << ss.str();
        __trace << endl << ss.str();
    }
}