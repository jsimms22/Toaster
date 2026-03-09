#pragma once

#include "Database.h"
// tinyxml
#include "tinyxml2.h"
// std library
#include <mutex>

class TinyXmlDatabase : public Database
{
public:
    TinyXmlDatabase();
    virtual ~TinyXmlDatabase() override;
    bool connect(const std::string& filePath) override;
    void disconnect() override;
    bool flush() override;

private:
    tinyxml2::XMLDocument m_doc;
    std::string m_filePath;
    std::mutex m_mutex;
};