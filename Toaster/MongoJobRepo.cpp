#include "MongoJobRepo.h"
#include "JobRequest.h"
#include "GuildSettings.h"
#include "BotUtility.h"
#include "JobRequestFactory.h"
#include "RequestID.h"
// mongo
#include <mongocxx/cursor.hpp>
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/document/view_or_value.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
// std library
#include <cstdint>
#include <cstdlib>
#include <unordered_set>

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
MongoJobRepo::MongoJobRepo(mongocxx::database db)
    : m_jobsCollection{ db["job_queue"] }, 
      m_guildsCollection{ db["guild_settings"] },
      m_archiveCollection{ db["job_archive"] }
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
    {
        return;
    }

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

        m_cv.wait(lock, [this, &stopToken] { return !m_mutations.empty() || stopToken.stop_requested(); });

        if (stopToken.stop_requested() && m_mutations.empty())
        {
            break;
        }

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
            mongoRepo->Insert("job_queue", doc);
        }
    );

    if (m_logger)
    {
        m_logger->debug("{}", "Added 'InsertJob' job to database mutation queue.");
    }
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
            mongoRepo->Update("job_queue", filter, doc);
        }
    );

    if (m_logger)
    {
        m_logger->debug("{}", "Added 'UpdateJob' job to database mutation queue.");
    }
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
            mongoRepo->Delete("job_queue", filter);
        }
    );

    if (m_logger)
    {
        m_logger->debug("{}", "Added 'DeleteJob' job to database mutation queue.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::ArchiveJobs(const std::vector<RequestID>& ids)
{
    EnqueueDBMutation([ids](IJobRepo* repo)
        {
            using namespace bsoncxx::builder::basic;

            auto mongoRepo = static_cast<MongoJobRepo*>(repo);

            array ids_array;

            for (const auto& id : ids)
            {
                ids_array.append(static_cast<std::int64_t>(id.value));
            }

            auto filter = make_document(
                kvp("_id", make_document(
                    kvp("$in", ids_array)
                ))
            );

            mongoRepo->ArchiveMany("job_queue", "job_archive", filter);
        });

    if (m_logger)
    {
        m_logger->debug("{}", "Added 'ArchiveMany' job to database mutation queue.");
    }
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
            mongoRepo->Insert("guild_settings", doc);
        }
    );

    if (m_logger)
    {
        m_logger->debug("{}", "Added 'InsertGuild' job to database mutation queue.");
    }
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
            mongoRepo->Update("guild_settings", filter, doc);
        }
    );

    if (m_logger)
    {
        m_logger->debug("{}", "Added 'UpdateGuild' job to database mutation queue.");
    }
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
            mongoRepo->Delete("guild_settings", filter);
        }
    );

    if (m_logger)
    {
        m_logger->debug("{}", "Added 'DeleteGuild' job to database mutation queue.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::vector<dpp::snowflake> MongoJobRepo::GetGuildsWithJobs()
{
    std::unordered_set<dpp::snowflake> guildSet;

    auto cursor = m_jobsCollection.distinct("_guild", {});
    for (auto&& doc : cursor)
    {
        auto&& val_elem = doc["values"];
        for (auto&& val : val_elem.get_array().value)
        {
            guildSet.insert(val.get_int64().value);
        }
    }

    cursor = m_archiveCollection.distinct("_guild", {});
    for (auto&& doc : cursor)
    {
        auto&& val_elem = doc["values"];
        for (auto&& val : val_elem.get_array().value)
        {
            guildSet.insert(val.get_int64().value);
        }
    }

    std::vector<dpp::snowflake> guilds(guildSet.begin(), guildSet.end());

    if (m_logger)
    {
        m_logger->info("{}", fmt::format("Found {} guild(s) with jobs in the database.", guilds.size()));
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

    if (m_logger)
    {
        m_logger->info("{}", fmt::format("Bootstrapped {} job(s) associated with guild {}.", jobs.size(), guildID));
    }

    return jobs;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::vector<std::shared_ptr<JobRequest>> MongoJobRepo::LoadArchived(const dpp::snowflake& guildID)
{
    using namespace bsoncxx::builder::basic;

    std::vector<std::shared_ptr<JobRequest>> jobs;

    auto filter = make_document(kvp("_guild", static_cast<int64_t>(guildID)));

    auto cursor = m_archiveCollection.find(filter.view());

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

    if (m_logger)
    {
        m_logger->info("{}", fmt::format("Bootstrapped {} archived job(s) associated with guild {}.", jobs.size(), guildID));
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

    if (m_logger)
    {
        m_logger->info("{}", fmt::format("Found settings associated with {} guild(s) in the database.", guilds.size()));
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

    if (m_logger)
    {
        m_logger->info("{}", fmt::format("Bootstrapped persistent settings associated with guild {}.", settings->GetGuildID()));
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
        m_archiveCollection.create_index(make_document(kvp("_guild", 1)));

        if (m_logger)
        {
            m_logger->info("{}", "Created database indexes.");
        }

    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to create MongoDB indexes: " << e.what() << std::endl;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::Insert(const std::string& name, bsoncxx::document::value document)
{
    auto col = GetCollection(name);
    if (!col)
    {
        return;
    }

    col->insert_one(document.view());
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::InsertMany(const std::string& name, const std::vector<bsoncxx::document::value>& documents)
{
    if (documents.empty())
    {
        return;
    }

    std::vector<bsoncxx::document::view> views;
    views.reserve(documents.size());

    for (const auto& doc : documents)
        views.push_back(doc);

    auto col = GetCollection(name);
    if (!col)
    {
        return;
    }

    col->insert_many(views);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::Update(const std::string& name, bsoncxx::document::value filter, bsoncxx::document::value replacement)
{
    auto col = GetCollection(name);
    if (!col)
    {
        return;
    }

    col->replace_one(filter.view(), replacement.view());
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::Delete(const std::string& name, bsoncxx::document::value filter)
{
    auto col = GetCollection(name);
    if (!col)
    {
        return;
    }

    col->delete_one(filter.view());
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::DeleteMany(const std::string& name, bsoncxx::document::value filter)
{
    auto col = GetCollection(name);
    if (!col)
    {
        return;
    }

    col->delete_many(filter.view());
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void MongoJobRepo::ArchiveMany(const std::string& src, const std::string& dst, bsoncxx::document::value filter)
{
    auto srcCol = GetCollection(src);
    auto dstCol = GetCollection(dst);

    if (!srcCol || !dstCol)
    {
        return;
    }

    std::vector<bsoncxx::document::value> docs;
    std::vector<std::int64_t> ids;

    auto cursor = srcCol->find(filter.view());

    for (auto&& doc : cursor)
    {
        docs.emplace_back(bsoncxx::document::value(doc));

        auto idElem = doc["_id"];
        if (idElem && idElem.type() == bsoncxx::type::k_int64)
        {
            ids.push_back(idElem.get_int64().value);
        }
    }

    if (docs.empty())
    {
        return;
    }

    InsertMany(dst, docs);

    using namespace bsoncxx::builder::basic;

    array ids_array;

    for (auto id : ids)
    {
        ids_array.append(id);
    }

    auto deleteFilter = make_document(
        kvp("_id", make_document(
            kvp("$in", ids_array.view())
        ))
    );

    DeleteMany(src, deleteFilter);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
mongocxx::collection* MongoJobRepo::GetCollection(const std::string& name)
{
    if (name == m_jobsCollection.name())
    {
        return &m_jobsCollection;
    }

    if (name == m_guildsCollection.name())
    {
        return &m_guildsCollection;
    }

    if (name == m_archiveCollection.name())
    {
        return &m_archiveCollection;
    }

    return nullptr;
}