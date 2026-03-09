#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

class Database
{
public:
    using Document = std::map<std::string, std::string>;
    using DocumentList = std::vector<Document>;

    virtual ~Database() = default;

    // Connection management
    virtual bool connect(const std::string& connectionString) = 0;
    virtual void disconnect() = 0;

    // Optional: commit/save (useful for XML)
    virtual bool flush() = 0;
};