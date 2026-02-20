#pragma once
#include <string>

class Trame
{
public:
    enum class Type : std::uint8_t {
        RSI
    };

    virtual ~Trame() = default;
    virtual bool isValid() const = 0;
    virtual std::string build() const = 0;
    virtual bool build(char* dst, std::size_t cap, std::size_t& outLen) const = 0;
    virtual Type type() const = 0;
};
