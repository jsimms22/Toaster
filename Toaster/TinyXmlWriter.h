#pragma once

#include "IDatabaseWriter.h"
// tinyxml
#include "tinyxml2.h"
// std library
#include <mutex>

class TinyXmlWriter : public IDatabaseWriter
{
public:
    TinyXmlWriter();
    virtual ~TinyXmlWriter() override;
    bool connect(const std::string& filePath) override;
    void disconnect() override;
    bool flush() override;

private:
    tinyxml2::XMLDocument m_doc;
    std::string m_filePath;
    std::mutex m_mutex;
};