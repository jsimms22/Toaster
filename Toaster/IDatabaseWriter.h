#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

class IDatabaseWriter
{
public:
    using Document = std::map<std::string, std::string>;
    using DocumentList = std::vector<Document>;

    virtual ~IDatabaseWriter() = default;

    // Connection management
    virtual bool connect(const std::string& connectionString) = 0;
    virtual void disconnect() = 0;

    // Optional: commit/save (useful for XML)
    virtual bool flush() = 0;
};