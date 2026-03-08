#include "MongoWriter.h"
// mongo
#include <mongocxx/uri.hpp>
// std library
#include <exception>

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
MongoWriter::MongoWriter()
    : m_instance{}
{
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
MongoWriter::~MongoWriter()
{
    disconnect();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
bool MongoWriter::connect(const std::string& connectionString)
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
        std::cout << "Pinged your deployment. You successfully connected to MongoDB!" << std::endl;

        return true;
    }
    catch (const std::exception& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
        return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoWriter::disconnect()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_client.reset();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
bool MongoWriter::flush() 
{ 
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
mongocxx::database MongoWriter::GetDB() const 
{ 
    return m_db; 
}