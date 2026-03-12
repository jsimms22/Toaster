#pragma once
#include "Resource.h"
// fmt
#include <fmt//format.h>
// std library
#include <atomic>
#include <chrono>
#include <cstdint>
#include <string>
#include <type_traits>

class GuildSettings;

struct RequestID
{
    RequestID() = default;
    RequestID(const std::uint64_t val) : value{ val } {}
    RequestID(const RequestID& other) = default;
    RequestID(RequestID&& other) noexcept = default;
    RequestID& operator=(const RequestID& other) = default;
    RequestID& operator=(RequestID&& other) noexcept = default;
    ~RequestID() = default;

    uint64_t value{ ID_NULL };

    bool IsValid() const
    {
        return value != ID_NULL;
    }

    bool operator==(const std::uint64_t& other) const
    {
        return value == other;
    }

    bool operator<(const std::uint64_t& other) const
    {
        return value < other;
    }

    bool operator==(const RequestID& other) const
    {
        return value == other.value;
    }

    bool operator<(const RequestID& other) const
    {
        return value < other.value;
    }
};

static_assert(std::is_trivially_copyable<RequestID>::value, "RequestID must be trivially copyable");

class IDGenerator
{
public:
    static RequestID Generate(const std::uint64_t type, const std::uint64_t customerID, const std::shared_ptr<GuildSettings> settings);

private:
    static std::uint64_t GetTimeStamp();
};

std::string EncodeBase32(uint64_t value);
std::string JobTypePrefix(std::uint64_t type);
std::string ToString(RequestID id);