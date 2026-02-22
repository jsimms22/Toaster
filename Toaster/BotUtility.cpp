#include "BotUtility.h"
// microsoft
#include <objbase.h>
// fmt
#include <fmt/format.h>
// std library
#include <chrono>
#include <cstdlib>
#include <fstream>

namespace utils
{
    std::string LoadSecret(const std::string& filename, const std::string& find)
    {
        std::ifstream file(filename);
        std::string line;

        while (std::getline(file, line))
        {
            if (line.find(find + "=") == 0)
            {
                return line.substr(std::string(find + "=").length());
            }
        }

        throw std::runtime_error(fmt::format("[{}] not found in config file",find));
    }

    // Helper function to convert string priority (e.g., "low", "med", "high") to numeric values
    int PriorityToString(const std::string& priority_str) 
    {
        if (priority_str == "critical") return 4;
        if (priority_str == "high") return 3;
        if (priority_str == "med") return 2;
        if (priority_str == "low") return 1;
        return -1;  // Default to -1 for unknown priorities
    }

    std::string GuidToString(const GUID& guid) 
    {
        char guidString[39]; // 36 characters + null terminator

        // Format the GUID as a string "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}"
        snprintf(guidString, sizeof(guidString),
            "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            guid.Data1, guid.Data2, guid.Data3,
            guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
            guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

        return std::string(guidString);
    }

    std::string GuidToStringNoBrackets(const GUID& guid)
    {
        char guidString[37]; // 36 characters + null terminator

        // Format the GUID as a string "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
        snprintf(guidString, sizeof(guidString),
            "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            guid.Data1, guid.Data2, guid.Data3,
            guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
            guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

        return std::string(guidString);
    }

    const GUID StringToGuid(const std::string& guidStr)
    {
        GUID guid;
        // Convert the GUID string to a GUID structure using IIDFromString
        HRESULT hr = IIDFromString(std::wstring(guidStr.begin(), guidStr.end()).c_str(), &guid);

        // Check if the conversion was successful
        if (FAILED(hr))
        {
            throw std::invalid_argument("Invalid GUID string format");
        }

        return guid;
    }

    const GUID CreateGUID() 
    {
        GUID guid;
        HRESULT hr = CoCreateGuid(&guid);

        // Check if the conversion was successful
        if (FAILED(hr))
        {
            throw std::invalid_argument("Invalid GUID string format");
        }

        return guid;
    }

    const std::size_t GetEpochTimestamp()
    {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }
}
