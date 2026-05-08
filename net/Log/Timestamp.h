#ifndef CLEARMOON_NET_LOG_TIMESTAMP_H
#define CLEARMOON_NET_LOG_TIMESTAMP_H

#include <cstdint>
#include <string>
#include <sys/types.h>
namespace clearmoon
{
namespace net
{

class Timestamp
{
public:
    static const int kMicroSecondPerSecond;

    explicit Timestamp(int64_t mircroSeconds) : microSecondsSinceEpoch_(mircroSeconds)
    {
    }

    Timestamp() : microSecondsSinceEpoch_(0)
    {
    }
    ~Timestamp() = default;

    static Timestamp now();
    static Timestamp invalid() { return Timestamp(-1); } ;

    int64_t getMicroSecond() const { return  microSecondsSinceEpoch_; }
    void setMicroSecond(int64_t MicroSecond) { microSecondsSinceEpoch_ = MicroSecond; }


    bool isValid() const { return microSecondsSinceEpoch_ > 0;}

    //To string 
    std::string toFormattedString();   //YYYYMMDD hh:mm:ss.xxxx
    std::string toDateString();        //YYYYMMDD

    //Other operation interfaces
    int64_t differMicroSecond(const Timestamp& rhs) const
    {
        return std::abs(rhs.getMicroSecond() - microSecondsSinceEpoch_);
    }
    
private:
    int64_t microSecondsSinceEpoch_;
};

}

}

#endif