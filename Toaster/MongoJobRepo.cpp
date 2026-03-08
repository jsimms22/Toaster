#include "MongoJobRepo.h"
#include "BotUtility.h"
#include "JobRequestFactory.h"
// mongo
#include <mongocxx/cursor.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/document/view_or_value.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
// std library
#include <cstdint>
#include <cstdlib>

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
MongoJobRepo::MongoJobRepo(mongocxx::database db)
    : m_collection(db["job_queue"])
{
    CreateIndexes();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::InsertJob(const dpp::snowflake& guildID, const std::shared_ptr<const JobRequest>& job)
{
    using namespace bsoncxx::builder::basic;

    auto doc = job->WriteAttributesBSON();

    //std::cout << bsoncxx::to_json(doc.view()) << std::endl;

    m_collection.insert_one(doc.view());
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::UpdateJob(const std::shared_ptr<const JobRequest>& job)
{
    using namespace bsoncxx::builder::basic;

    auto filter = make_document(
        kvp("_id", utils::GuidToStringNoBrackets(job->GetID()))
    );

    m_collection.replace_one(filter.view(), job->WriteAttributesBSON().view());
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::DeleteJob(const GUID& id)
{
    using namespace bsoncxx::builder::basic;

    auto filter = make_document(
        kvp("_id", utils::GuidToStringNoBrackets(id))
    );

    m_collection.delete_one(filter.view());
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::vector<dpp::snowflake> MongoJobRepo::GetGuildsWithJobs()
{
    std::vector<dpp::snowflake> guilds;

    auto cursor = m_collection.distinct("_guild", {});

    for (auto&& doc : cursor)
    {
        auto&& val_elem = doc["values"];
        for (auto&& val : val_elem.get_array().value)
        {
            guilds.emplace_back(val.get_int64().value);
        }
    }

    return guilds;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<JobRequest>> MongoJobRepo::LoadJobs(dpp::snowflake guildID)
{
    using namespace bsoncxx::builder::basic;

    std::vector<std::shared_ptr<JobRequest>> jobs;

    auto filter = make_document(kvp("_guild", static_cast<int64_t>(guildID)));

    auto cursor = m_collection.find(filter.view());

    for (auto&& doc : cursor)
    {
        if (doc.find("job_type") != doc.end() && doc["job_type"].type() == bsoncxx::type::k_int32)
        {
            std::size_t type = doc["job_type"].get_int32().value;
            auto job = JobRequestFactory::Create(type, guildID);
            job->ReadAttributesBSON(doc);
            jobs.push_back(job);
        }
        else
        {
            continue;
        }

    }

    return jobs;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::CreateIndexes()
{
    using namespace bsoncxx::builder::basic;

    try
    {
        m_collection.create_index(
            make_document(kvp("_guild", 1))
        );
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to create MongoDB indexes: " << e.what() << std::endl;
    }
}