#include "qts_server.h"
#include <sys/types.h>
#include <dirent.h>
#include <boost/algorithm/string.hpp>

void InsertCzceCode(string& code) {
    int index = -1;
    for (size_t _i = 0; _i < code.length(); _i++) {
        char temp = code.at(_i);
        if (temp >= '0' && temp <= '9') {
            index = _i;
            break;
        }
    }
    if (index > 0) {
        code.insert(index, "2");
    }
}

namespace co {

    void QtsServer::Run() {
        QOptionsPtr opt = Config::Instance()->opt();
        QServer::Instance()->Init(opt);
        QServer::Instance()->Start();
        GetAllFile();
        int market = Config::Instance()->market();
        if (market == 1) {
            ParseSHFEHistoryData();
        } else {
            ParseDCEHistoryData();
        }
        QServer::Instance()->Wait();
        Stop();
        QServer::Instance()->Stop();
    };

    void QtsServer::Stop() {
    }

    void QtsServer::GetAllFile() {
        string data_dir = Config::Instance()->data_dir();
        string parse_date = Config::Instance()->parse_date();
        vector<string> vec_data;
        vec_data.clear();
        x::Split(&vec_data, parse_date, "|");
        for (auto& it : vec_data) {
            string path = data_dir + "/" + it;
            DIR *pDir;
            struct dirent* ptr;
            if(!(pDir = opendir(path.c_str())))
                return;
            while((ptr = readdir(pDir))!=0) {//返回当前目录的进入点，string path路径的进入点为当前目录 "." . 和 上一个目录  ".."
                if (strlen(ptr->d_name) > 8) {
                    all_files_.push_back(path + "/" + ptr->d_name);
                }
            }
            closedir(pDir);
        }
        std::sort(all_files_.begin(), all_files_.end(), [](string a, string b) {return a.compare(b) < 0;});
        for (const auto& it : all_files_) {
            __info << "add file: " << it;
        }
    }

    bool QtsServer::IsFriday(int64_t date) {
        int years = date / 10000;
        int left = date % 10000;
        int months = left / 100;
        int days = left % 100;
        int WeekDay=-1;
        if(1 == months || 2 == months) {
            months += 12;
            years--;
        }
        WeekDay=(days+1+2*months+3*(months+1)/5+years+years/4-years/100+years/400)%7;
        if (WeekDay == 5) {
            return true;
        } else {
            return false;
        }
    }

    void QtsServer::ParseSHFEHistoryData() {
        for (const auto& it : all_files_) {
            LOG_INFO << it;
            ifstream infile;
            infile.open(it.data());

            string line;
            // 解析文件中的每一行信息
            while (getline(infile, line)) {
                // __info << line;
                vector<string> vec_data;
                vec_data.clear();
                x::Split(&vec_data, line, ",");
                if (vec_data.size() > 20) {
                    string csv_code = x::Trim(vec_data[0]);
                    if (csv_code == "symbol") {
                        continue;
                    }
                    transform(csv_code.begin(), csv_code.end(), csv_code.begin(), ::tolower);
                    string code = csv_code + kSuffixSHFE;

                    QContextPtr ctx = QServer::Instance()->GetContext(code);
                    if (!ctx) {
                        ctx = QServer::Instance()->NewContext(code, code);
                    }

                    string trading_time = x::Trim(vec_data[3]);
                    boost::algorithm::replace_all(trading_time, "-", "");
                    boost::algorithm::replace_all(trading_time, ".", "");
                    boost::algorithm::replace_all(trading_time, ":", "");
                    boost::algorithm::replace_all(trading_time, " ", "");
                    int64_t timestamp = atoll(trading_time.c_str());
                    int64_t trading_day = atoll(x::Trim(vec_data[4]).c_str());

                    co::fbs::QTickT &m = ctx->PrepareQTick();
                    if (m.dtype <= 0) {
                        m.dtype = kDTypeFuture;
                        m.src = kSrcQTickLevel2;
                        m.market = kMarketSHFE;
                    }
                    m.timestamp = timestamp;
                    m.date = trading_day;
                    int index = 4;
                    m.new_price = atof(x::Trim(vec_data[++index]).c_str());
                    m.open = atof(x::Trim(vec_data[++index]).c_str());
                    m.high = atof(x::Trim(vec_data[++index]).c_str());
                    m.low = atof(x::Trim(vec_data[++index]).c_str());
                    m.new_volume = atoll(x::Trim(vec_data[++index]).c_str());
                    m.sum_volume = atoll(x::Trim(vec_data[++index]).c_str());
                    m.pre_open_interest = atoll(x::Trim(vec_data[++index]).c_str());
                    m.open_interest = atoll(x::Trim(vec_data[++index]).c_str());
                    m.sum_amount = atoll(x::Trim(vec_data[++index]).c_str());
                    m.new_amount = atof(x::Trim(vec_data[++index]).c_str());
                    m.upper_limit = atof(x::Trim(vec_data[++index]).c_str());
                    m.lower_limit = atof(x::Trim(vec_data[++index]).c_str());
                    m.pre_settle = atof(x::Trim(vec_data[++index]).c_str());
                    m.pre_close = atof(x::Trim(vec_data[++index]).c_str());
                    m.close = atof(x::Trim(vec_data[++index]).c_str());
                    m.settle = atof(x::Trim(vec_data[++index]).c_str());
                    ++index;
                    ++index;
                    double SellPrice01 = atof(x::Trim(vec_data[++index]).c_str());
                    double BuyPrice01 = atof(x::Trim(vec_data[++index]).c_str());
                    int64_t SellVolume01 = atoll(x::Trim(vec_data[++index]).c_str());
                    int64_t BuyVolume01 = atoll(x::Trim(vec_data[++index]).c_str());

                    double SellPrice05 = atof(x::Trim(vec_data[++index]).c_str());
                    double SellPrice04 = atof(x::Trim(vec_data[++index]).c_str());
                    double SellPrice03 = atof(x::Trim(vec_data[++index]).c_str());
                    double SellPrice02 = atof(x::Trim(vec_data[++index]).c_str());

                    double BuyPrice02 = atof(x::Trim(vec_data[++index]).c_str());
                    double BuyPrice03 = atof(x::Trim(vec_data[++index]).c_str());
                    double BuyPrice04 = atof(x::Trim(vec_data[++index]).c_str());
                    double BuyPrice05 = atof(x::Trim(vec_data[++index]).c_str());

                    int64_t SellVolume05 = atoll(x::Trim(vec_data[++index]).c_str());
                    int64_t SellVolume04 = atoll(x::Trim(vec_data[++index]).c_str());
                    int64_t SellVolume03 = atoll(x::Trim(vec_data[++index]).c_str());
                    int64_t SellVolume02 = atoll(x::Trim(vec_data[++index]).c_str());

                    int64_t BuyVolume02 = atoll(x::Trim(vec_data[++index]).c_str());
                    int64_t BuyVolume03 = atoll(x::Trim(vec_data[++index]).c_str());
                    int64_t BuyVolume04 = atoll(x::Trim(vec_data[++index]).c_str());
                    int64_t BuyVolume05 = atoll(x::Trim(vec_data[++index]).c_str());
                    ++index;
                    ++index;
                    m.multiple = atoll(x::Trim(vec_data[++index]).c_str());
                    ++index;
                    ++index;
                    ++index;
                    m.price_step = atof(x::Trim(vec_data[++index]).c_str());
                    if (m.settle > 0) {
                        m.close = atof(x::Trim(vec_data[++index]).c_str());
                        LOG_INFO << m.code << ", " << m.timestamp << ", close_price: " << m.close;
                    }

                    if (BuyVolume01 > 0) {
                        m.bp.emplace_back(BuyPrice01);
                        m.bv.emplace_back(BuyVolume01);
                        if (BuyVolume02 > 0) {
                            m.bp.emplace_back(BuyPrice02);
                            m.bv.emplace_back(BuyVolume02);
                            if (BuyVolume03 > 0) {
                                m.bp.emplace_back(BuyPrice03);
                                m.bv.emplace_back(BuyVolume03);
                                if (BuyVolume04 > 0) {
                                    m.bp.emplace_back(BuyPrice04);
                                    m.bv.emplace_back(BuyVolume04);
                                    if (BuyVolume05 > 0) {
                                        m.bp.emplace_back(BuyPrice05);
                                        m.bv.emplace_back(BuyVolume05);
                                    }
                                }
                            }
                        }
                    }

                    if (SellVolume01 > 0) {
                        m.ap.emplace_back(SellPrice01);
                        m.av.emplace_back(SellVolume01);
                        if (SellVolume02 > 0) {
                            m.ap.emplace_back(SellPrice02);
                            m.av.emplace_back(SellVolume02);
                            if (SellVolume03 > 0) {
                                m.ap.emplace_back(SellPrice03);
                                m.av.emplace_back(SellVolume03);
                                if (SellVolume04 > 0) {
                                    m.ap.emplace_back(SellPrice04);
                                    m.av.emplace_back(SellVolume04);
                                    if (SellVolume05 > 0) {
                                        m.ap.emplace_back(SellPrice05);
                                        m.av.emplace_back(SellVolume05);
                                    }
                                }
                            }
                        }
                    }
                    string out = ctx->FinishQTick();
                    QServer::Instance()->PushQTick(out);
                }
            }
            infile.close();
        }
    }

    void QtsServer::ParseDCEHistoryData() {
        for (const auto& it : all_files_) {
            LOG_INFO << it;
            ifstream infile;
            infile.open(it.data());

            string line;
            // 解析文件中的每一行信息
            vector<co::fbs::QTickT> all_tick;
            while (getline(infile, line)) {
                // __info << line;
                vector<string> vec_data;
                vec_data.clear();
                x::Split(&vec_data, line, ",");
                if (vec_data.size() > 20) {
                    string csv_code = x::Trim(vec_data[0]);
                    transform(csv_code.begin(), csv_code.end(), csv_code.begin(), ::tolower);
                    string code = csv_code + kSuffixDCE;
                    string trading_time = x::Trim(vec_data[4]);
                    boost::algorithm::replace_all(trading_time, "-", "");
                    boost::algorithm::replace_all(trading_time, ".", "");
                    boost::algorithm::replace_all(trading_time, ":", "");
                    boost::algorithm::replace_all(trading_time, " ", "");
                    int64_t timestamp = atoll(trading_time.c_str());
                    int64_t trading_day = atoll(x::Trim(vec_data[3]).c_str());

                    co::fbs::QTickT m ;
                    m.code = code;
                    m.dtype = kDTypeFuture;
                    m.src = kSrcQTickLevel2;
                    m.market = kMarketDCE;
                    int64_t pre_timestamp = m.timestamp;
                    m.timestamp = timestamp;
                    m.date = trading_day;
                    int index = 4;
                    m.new_price = atof(x::Trim(vec_data[++index]).c_str());
                    m.high = atof(x::Trim(vec_data[++index]).c_str());
                    m.low = atof(x::Trim(vec_data[++index]).c_str());
                    ++index;
                    m.sum_volume = atoll(x::Trim(vec_data[++index]).c_str());
                    m.new_volume = atoll(x::Trim(vec_data[++index]).c_str());
                    m.pre_open_interest = atoll(x::Trim(vec_data[++index]).c_str());
                    m.open_interest = atoll(x::Trim(vec_data[++index]).c_str());
                    ++index;
                    m.sum_amount = atof(x::Trim(vec_data[++index]).c_str());
                    ++index;
                    m.upper_limit = atof(x::Trim(vec_data[++index]).c_str());
                    m.lower_limit = atof(x::Trim(vec_data[++index]).c_str());
                    m.pre_settle = atof(x::Trim(vec_data[++index]).c_str());
                    m.pre_close = atof(x::Trim(vec_data[++index]).c_str());
                    m.open = atof(x::Trim(vec_data[++index]).c_str());
                    m.close = atof(x::Trim(vec_data[++index]).c_str());
                    m.settle = atof(x::Trim(vec_data[++index]).c_str());
                    ++index;
                    ++index;
                    ++index;
                    ++index;
                    ++index;
                    double SellPrice01 = atof(x::Trim(vec_data[++index]).c_str());
                    double BuyPrice01 = atof(x::Trim(vec_data[++index]).c_str());
                    int64_t SellVolume01 = atoll(x::Trim(vec_data[++index]).c_str());
                    int64_t BuyVolume01 = atoll(x::Trim(vec_data[++index]).c_str());

                    double SellPrice05 = atof(x::Trim(vec_data[++index]).c_str());
                    double SellPrice04 = atof(x::Trim(vec_data[++index]).c_str());
                    double SellPrice03 = atof(x::Trim(vec_data[++index]).c_str());
                    double SellPrice02 = atof(x::Trim(vec_data[++index]).c_str());

                    double BuyPrice02 = atof(x::Trim(vec_data[++index]).c_str());
                    double BuyPrice03 = atof(x::Trim(vec_data[++index]).c_str());
                    double BuyPrice04 = atof(x::Trim(vec_data[++index]).c_str());
                    double BuyPrice05 = atof(x::Trim(vec_data[++index]).c_str());

                    int64_t SellVolume05 = atoll(x::Trim(vec_data[++index]).c_str());
                    int64_t SellVolume04 = atoll(x::Trim(vec_data[++index]).c_str());
                    int64_t SellVolume03 = atoll(x::Trim(vec_data[++index]).c_str());
                    int64_t SellVolume02 = atoll(x::Trim(vec_data[++index]).c_str());

                    int64_t BuyVolume02 = atoll(x::Trim(vec_data[++index]).c_str());
                    int64_t BuyVolume03 = atoll(x::Trim(vec_data[++index]).c_str());
                    int64_t BuyVolume04 = atoll(x::Trim(vec_data[++index]).c_str());
                    int64_t BuyVolume05 = atoll(x::Trim(vec_data[++index]).c_str());

                    if (BuyVolume01 > 0) {
                        m.bp.emplace_back(BuyPrice01);
                        m.bv.emplace_back(BuyVolume01);
                        if (BuyVolume02 > 0) {
                            m.bp.emplace_back(BuyPrice02);
                            m.bv.emplace_back(BuyVolume02);
                            if (BuyVolume03 > 0) {
                                m.bp.emplace_back(BuyPrice03);
                                m.bv.emplace_back(BuyVolume03);
                                if (BuyVolume04 > 0) {
                                    m.bp.emplace_back(BuyPrice04);
                                    m.bv.emplace_back(BuyVolume04);
                                    if (BuyVolume05 > 0) {
                                        m.bp.emplace_back(BuyPrice05);
                                        m.bv.emplace_back(BuyVolume05);
                                    }
                                }
                            }
                        }
                    }

                    if (SellVolume01 > 0) {
                        m.ap.emplace_back(SellPrice01);
                        m.av.emplace_back(SellVolume01);
                        if (SellVolume02 > 0) {
                            m.ap.emplace_back(SellPrice02);
                            m.av.emplace_back(SellVolume02);
                            if (SellVolume03 > 0) {
                                m.ap.emplace_back(SellPrice03);
                                m.av.emplace_back(SellVolume03);
                                if (SellVolume04 > 0) {
                                    m.ap.emplace_back(SellPrice04);
                                    m.av.emplace_back(SellVolume04);
                                    if (SellVolume05 > 0) {
                                        m.ap.emplace_back(SellPrice05);
                                        m.av.emplace_back(SellVolume05);
                                    }
                                }
                            }
                        }
                    }
                    all_tick.push_back(m);
                }
            }
            infile.close();
            sort(all_tick.begin(), all_tick.end(), [](const co::fbs::QTickT &t1, const co::fbs::QTickT &t2) {
                if (t1.timestamp < t2.timestamp) {
                    return true;
                } else {
                    return false;
                }
            });
            for (auto& it : all_tick) {
                string code = it.code;
                QContextPtr ctx = QServer::Instance()->GetContext(code);
                if (!ctx) {
                    ctx = QServer::Instance()->NewContext(code, code);
                }
                co::fbs::QTickT &m = ctx->PrepareQTick();
                m = it;
                string out = ctx->FinishQTick();
                QServer::Instance()->PushQTick(out);
            }
        }
    }
}
