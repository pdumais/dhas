#pragma once
#include <string>

class UDev
{
public:
    static std::string findDevice(const std::string& vendorstr, const std::string& productstr);
};
