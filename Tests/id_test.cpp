#include <iostream>
#include <unordered_set>
#include <cassert>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <string>
#include <type_traits>
#include <thread>
#include <random>
#include <array>

struct RequestID
{
    RequestID() = default;
    RequestID(const std::uint64_t val) : value{ val } {}
    RequestID(const RequestID& other) = default;
    RequestID(RequestID&& other) noexcept = default;
    RequestID& operator=(const RequestID& other) = default;
    RequestID& operator=(RequestID&& other) noexcept = default;
    ~RequestID() = default;

    uint64_t value{ 0 };
};

class IDGenerator
{
public:
    static RequestID Generate(const std::uint64_t type, const std::uint64_t customerID);

    static std::array<std::atomic<std::uint32_t>, 7> s_counter;
private:
    static std::uint64_t GetTimeStamp();
};

std::array<std::atomic<std::uint32_t>, 7> IDGenerator::s_counter;

RequestID IDGenerator::Generate(const std::uint64_t type, const std::uint64_t customerID)
{
    const std::uint64_t time = GetTimeStamp();

    const std::uint64_t id =
        ((time & 0x1FFFFFFFFFF) << 21) |    // 41 bits timestamp
        ((type & 0x3F) << 15) |             // 6 bits type
        ((customerID & 0x3F) << 9) |        // 6 bits customer
        (s_counter[type].fetch_add(1) & 0x1FF);   // 9 bits counter

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
    case 2:  return "CRFT";
    case 3:  return "BLDG";
    case 4:  return "RES";
    case 5:  return "REF";
    case 6: return "COMP";
    case 7:    return "HAZ";
    case 1:
    default:                 return "JOB";
    }
}

std::string ToString(RequestID id)
{
    const std::uint64_t type = (id.value >> 15) & 0x3F;
    // Take the lowest 50 bits for encoding
    //uint64_t shortPart = id.value & 0x3FFFFFFFFFFFF; // 50 bits
    const uint64_t shortPart = id.value & 0x3FFFFFFFFFFFFF;  // last 20 bits
    std::string encoded = EncodeBase32(shortPart);

    // Pad with zeros if too short
    if (encoded.size() < 8)
        encoded = std::string(8 - encoded.size(), '0') + encoded;

    return std::string(JobTypePrefix(type)) + "-" +
           encoded.substr(0, 4) + "-" +
           encoded.substr(4, 4);
}


void TestIDGenerator()
{
    uint64_t TEST_TYPE;
    uint64_t TEST_CUSTOMER;
    constexpr int TEST_COUNT = 100000;

    IDGenerator generator;
    std::unordered_set<uint64_t> ids;
    ids.reserve(TEST_COUNT);

    uint64_t previous = 0;

    for (int i = 0; i < TEST_COUNT; i++)
    {
        TEST_TYPE = 1 + rand() % 7;
        TEST_CUSTOMER = 1 + rand() % 64;
        //TEST_CUSTOMER = std::uniform_int_distribution<int>(1, 73709551615)(std::mt19937{std::random_device{}()});
        RequestID id = IDGenerator::Generate(TEST_TYPE, TEST_CUSTOMER);

        uint64_t value = id.value;

        auto itr = std::find_if(ids.cbegin(), ids.cend(), [value](const auto id) -> bool { return ToString(value) == ToString(id); });
        if (itr != ids.cend())
        {
            std::cout << "duplicate: " << ToString(value) << std::endl;
            std::cout << "id size:" << std::to_string(ids.size()) << std::endl;
            std::cout << "craft:" << std::to_string(IDGenerator::s_counter[1]) << std::endl;
            std::cout << "base:" << std::to_string(IDGenerator::s_counter[2]) << std::endl;
            std::cout << "comp:" << std::to_string(IDGenerator::s_counter[3]) << std::endl;
            std::cout << "resource:" << std::to_string(IDGenerator::s_counter[4]) << std::endl;
            std::cout << "refine:" << std::to_string(IDGenerator::s_counter[5]) << std::endl;
            std::cout << "hazmat:" << std::to_string(IDGenerator::s_counter[6]) << std::endl;
            std::cerr << "Duplicate ID detected!\n";
            assert(false);
        }

        // 1. Ensure uniqueness
        if (!ids.insert(value).second)
        {
            std::cout << "duplicate: " << std::to_string(value) << std::endl;
            std::cout << "id size:" << std::to_string(ids.size()) << std::endl;
            std::cerr << "Duplicate ID detected!\n";
            assert(false);
        }

        previous = value;

        // 4. Ensure safe for MongoDB int64
        int64_t signedValue = static_cast<int64_t>(value);
        assert(signedValue >= 0);

        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::cout << ToString(value) << std::endl;
    }

    std::cout << "ID generation test passed (" << TEST_COUNT << " IDs)\n";
}

int main()
{

    TestIDGenerator();
    
}