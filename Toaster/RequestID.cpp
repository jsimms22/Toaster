#include "RequestID.h"
#include "GuildSettings.h"
// std library
#include "memory"

RequestID IDGenerator::Generate(const std::uint64_t type, const std::uint64_t customerID, const std::shared_ptr<GuildSettings> settings)
{
    const std::uint64_t time = GetTimeStamp();

    const std::uint64_t id =
        ((time & 0x1FFFFFFFFFF) << 21) |                    // 41 bits timestamp
        ((type & 0x3F) << 15) |                             // 6 bits type
        ((customerID & 0x3F) << 9) |                        // 6 bits worker
        (settings->g_counter[type].fetch_add(1) & 0x1FF);   // 9 bits counter

    return RequestID{ id };
}

std::uint64_t IDGenerator::GetTimeStamp()
{
    using namespace std::chrono;

    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

static const char* BASE32 = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";

std::string EncodeBase32(uint64_t value)
{
    std::string out;

    for (int i = 0; i < 8; i++)
    {
        out += BASE32[value & 31];
        value >>= 5;
    }

    return out;
}

std::string JobTypePrefix(std::uint64_t type)
{
    switch (type)
    {
    case JOB_TYPE_CRAFTING:  return "CRFT";
    case JOB_TYPE_BUILDING:  return "BLDG";
    case JOB_TYPE_RESOURCE:  return "RES";
    case JOB_TYPE_REFINERY:  return "REF";
    case JOB_TYPE_COMPONENT: return "COMP";
    case JOB_TYPE_HAZARD:    return "HAZ";
    case JOB_TYPE_GENERAL:
    default:                 return "JOB";
    }
}

std::string ToString(RequestID id)
{
    const std::uint64_t type = static_cast<std::uint64_t>((id.value >> 15) & 0x3F);

    const uint64_t shortPart = id.value & 0x3FFFFFFFFFFFFF; // last 50-bits

    std::string encoded = EncodeBase32(shortPart);

    if (encoded.size() < 8)
        encoded = std::string(8 - encoded.size(), '0') + encoded;

    return fmt::format("{}-{}-{}",
        JobTypePrefix(type),
        fmt::string_view(encoded.data(), 4),
        fmt::string_view(encoded.data() + 4, 4));
}