#include "MongoDatabase.h"
// mongo
#include <mongocxx/uri.hpp>
// std library
#include <exception>

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
MongoDatabase::MongoDatabase()
    : m_instance{}
{
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
MongoDatabase::~MongoDatabase()
{
    disconnect();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
bool MongoDatabase::connect(const std::string& connectionString)
{
    using namespace bsoncxx::builder::basic;

    std::lock_guard<std::mutex> lock(m_mutex);

    try
    {
        mongocxx::options::client client_options;
        const auto api = mongocxx::options::server_api{ mongocxx::options::server_api::version::k_version_1 };
        client_options.server_api_opts(api);

        m_client = std::make_unique<mongocxx::client>(mongocxx::uri{ connectionString }, client_options);
        m_db = m_client->database("Toaster");

        const auto ping_cmd = make_document(kvp("ping", 1));
        m_db.run_command(ping_cmd.view());

        if (m_logger)
            m_logger->info("{}", "Connected to MongoDB cluster.");

        return true;
    }
    catch (const std::exception& e)
    {
        if (m_logger)
            m_logger->error("{}", fmt::format("Caught exception while connecting to the database: {}", e.what()));

        return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoDatabase::disconnect()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_client.reset();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
bool MongoDatabase::flush()
{ 
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
mongocxx::database MongoDatabase::GetDB() const
{ 
    return m_db; 
}