#include "MongoJobRepo.h"
#include "JobRequest.h"
#include "GuildSettings.h"
#include "BotUtility.h"
#include "JobRequestFactory.h"
#include "RequestID.h"
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
    : m_jobsCollection{ db["job_queue"] }, m_guildsCollection{ db["guild_settings"] }
{
    CreateIndexes();
    StartWorker();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::StartWorker()
{
    if (m_worker.joinable())
        return;

    m_worker = std::jthread([this](std::stop_token st) { DatabaseWorker(st); });
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::DatabaseWorker(std::stop_token stopToken)
{
    while (!stopToken.stop_requested())
    {
        std::unique_lock<std::mutex> lock(m_mtxMutQueue);

        m_cv.wait(lock, [this, &stopToken] {
            return !m_mutations.empty() || stopToken.stop_requested();
            });

        if (stopToken.stop_requested())
            break;

        Mutation mutation = std::move(m_mutations.front());
        m_mutations.pop();
        lock.unlock();

        mutation(this);
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::EnqueueDBMutation(Mutation mutation)
{
    {
        std::lock_guard<std::mutex> lock(m_mtxMutQueue);
        m_mutations.emplace(std::move(mutation));
    }

    m_cv.notify_one();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::InsertJob(const std::shared_ptr<const JobRequest>& job)
{
    auto doc = job->WriteAttributesBSON().extract();
    EnqueueDBMutation([doc](IJobRepo* repo)
        {
            using namespace bsoncxx::builder::basic;
            auto mongoRepo = static_cast<MongoJobRepo*>(repo);
            mongoRepo->Insert("job_queue", doc.view());
        }
    );
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::UpdateJob(const std::shared_ptr<const JobRequest>& job)
{
    auto rID = job->GetID();
    auto doc = job->WriteAttributesBSON().extract();
    EnqueueDBMutation([rID, doc](IJobRepo* repo)
        {
            using namespace bsoncxx::builder::basic;
            auto mongoRepo = static_cast<MongoJobRepo*>(repo);
            auto filter = make_document(kvp("_id", static_cast<std::int64_t>(rID.value)));
            mongoRepo->Update("job_queue", filter.view(), doc.view());
        }
    );
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::DeleteJob(const RequestID rID)
{
    using namespace bsoncxx::builder::basic;
    EnqueueDBMutation([rID](IJobRepo* repo)
        {
            using namespace bsoncxx::builder::basic;
            auto mongoRepo = static_cast<MongoJobRepo*>(repo);
            auto filter = make_document(kvp("_id", static_cast<std::int64_t>(rID.value)));
            mongoRepo->Delete("job_queue", filter.view());
        }
    );
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::InsertGuild(const std::shared_ptr<const GuildSettings>& settings)
{
    auto doc = settings->WriteAttributesBSON().extract();
    EnqueueDBMutation([doc](IJobRepo* repo)
        {
            using namespace bsoncxx::builder::basic;
            auto mongoRepo = static_cast<MongoJobRepo*>(repo);
            mongoRepo->Insert("guild_settings", doc.view());
        }
    );
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::UpdateGuild(const dpp::snowflake& guildID, const std::shared_ptr<const GuildSettings>& settings)
{
    auto doc = settings->WriteAttributesBSON().extract();
    EnqueueDBMutation([guildID, doc](IJobRepo* repo)
        {
            using namespace bsoncxx::builder::basic;
            auto mongoRepo = static_cast<MongoJobRepo*>(repo);
            auto filter = make_document(kvp("_id", static_cast<std::int64_t>(guildID)));
            mongoRepo->Update("guild_settings", filter.view(), doc.view());
        }
    );
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::DeleteGuild(const dpp::snowflake& guildID)
{
    using namespace bsoncxx::builder::basic;
    EnqueueDBMutation([guildID](IJobRepo* repo)
        {
            using namespace bsoncxx::builder::basic;
            auto mongoRepo = static_cast<MongoJobRepo*>(repo);
            auto filter = make_document(kvp("_id", static_cast<std::int64_t>(guildID)));
            mongoRepo->Delete("guild_settings", filter.view());
        }
    );
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::vector<dpp::snowflake> MongoJobRepo::GetGuildsWithJobs()
{
    std::vector<dpp::snowflake> guilds;

    auto cursor = m_jobsCollection.distinct("_guild", {});

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
std::vector<std::shared_ptr<JobRequest>> MongoJobRepo::LoadJobs(const dpp::snowflake& guildID)
{
    using namespace bsoncxx::builder::basic;

    std::vector<std::shared_ptr<JobRequest>> jobs;

    auto filter = make_document(kvp("_guild", static_cast<int64_t>(guildID)));

    auto cursor = m_jobsCollection.find(filter.view());

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
std::vector<dpp::snowflake> MongoJobRepo::GetGuildsWithSettings()
{
    std::vector<dpp::snowflake> guilds;

    auto cursor = m_guildsCollection.distinct("_id", {});

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
bool MongoJobRepo::LoadGuildSettings(const std::shared_ptr<GuildSettings>& settings)
{
    using namespace bsoncxx::builder::basic;

    auto filter = make_document(kvp("_id", static_cast<int64_t>(settings->GetGuildID())));

    auto cursor = m_guildsCollection.find(filter.view());

    bool bResult = false;
    for (auto&& doc : cursor)
    {
        settings->ReadAttributesBSON(doc);
        bResult = true;
    }

    return bResult;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::CreateIndexes()
{
    using namespace bsoncxx::builder::basic;

    try
    {
        m_jobsCollection.create_index(make_document(kvp("_guild", 1)));
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to create MongoDB indexes: " << e.what() << std::endl;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::Insert(const std::string& name, bsoncxx::document::view_or_value document)
{
    if (name == m_jobsCollection.name())
    {
        m_jobsCollection.insert_one(document);
    }
    else if (name == m_guildsCollection.name())
    {
        m_guildsCollection.insert_one(document);
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::Update(const std::string& name, bsoncxx::document::view_or_value filter, bsoncxx::document::view_or_value replacement)
{
    if (name == m_jobsCollection.name())
    {
        m_jobsCollection.replace_one(filter, replacement);
    }
    else if (name == m_guildsCollection.name())
    {
        m_guildsCollection.replace_one(filter, replacement);
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::Delete(const std::string& name, bsoncxx::document::view_or_value filter)
{
    if (name == m_jobsCollection.name())
    {
        m_jobsCollection.delete_one(filter);
    }
    else if (name == m_guildsCollection.name())
    {
        m_guildsCollection.delete_one(filter);
    }
}