#pragma once

#include "IDatabaseWriter.h"
// mongo
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
// std library
#include <memory>
#include <mutex>

class MongoWriter : public IDatabaseWriter
{
public:
    MongoWriter();
    virtual ~MongoWriter() override;
    bool connect(const std::string& connectionString) override;
    void disconnect() override;
    bool flush() override;
    mongocxx::database GetDB() const;

private:
    mongocxx::instance m_instance;
    std::unique_ptr<mongocxx::client> m_client;
    mongocxx::database m_db;

    std::mutex m_mutex;
};