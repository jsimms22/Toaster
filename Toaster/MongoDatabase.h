#pragma once

#include "Database.h"
// mongo
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
// std library
#include <memory>
#include <mutex>

class MongoDatabase : public Database
{
public:
    MongoDatabase();
    virtual ~MongoDatabase() override;
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