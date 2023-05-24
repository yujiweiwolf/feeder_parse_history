#include <iostream>
#include <boost/program_options.hpp>
#include <x/x.h>
#include "coral/wal_reader.h"
#include "feeder/feeder.h"


using namespace std;
using namespace co;
namespace po = boost::program_options;

// const string file = "/home/work/sys/develop/feeder_parse_czce_history/data/feeder_20200813_20220630104445171.wal";

int main(int argc, char* argv[]) {
    string file = argv[1];
    try {
        co::WALReader reader;
        reader.Open(file.c_str());
        while (true) {
            std::string raw;
            int64_t type = reader.Read(&raw);
            if (raw.empty()) {
                break;
            }
            switch (type) {
                case kFBPrefixQTick: {
                    auto q = flatbuffers::GetRoot<co::fbs::QTick>(raw.data());
                    std::string code = q->code() ? q->code()->str() : "";
                    int64_t date = q->date() > 0 ? q->date() : q->timestamp() / 1000000000LL;
                    int64_t timestamp = q->timestamp();
                    int64_t sum_volume = q->sum_volume();
                    int64_t sum_amount = q->sum_amount();
                    int64_t new_volume = q->new_volume();
                    int64_t new_amount = q->new_amount();
                    __info << "code: " << code << ", timestamp: " << timestamp;
//                    int64_t timestamp = q->timestamp();
//                    int64_t timestamp = q->timestamp();
//                    int64_t timestamp = q->timestamp();
//                    int64_t timestamp = q->timestamp();
//                    int64_t timestamp = q->timestamp();
//                    int64_t timestamp = q->timestamp();
//                    int64_t timestamp = q->timestamp();
//                    int64_t timestamp = q->timestamp();
//                    int64_t timestamp = q->timestamp();
//                    int64_t timestamp = q->timestamp();
                    break;
                }
            }
        }
    } catch (std::exception& e) {
        __fatal << "server is crashed, " << e.what();
        return 2;
    } catch (...) {
        __fatal << "server is crashed, unknown reason";
        return 3;
    }
    return 0;
}

