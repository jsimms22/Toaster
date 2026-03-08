#pragma once
#include "IJobRepo.h"
// mongo
#include <mongocxx/database.hpp>

class MongoJobRepo : public IJobRepo
{
public:
    MongoJobRepo(mongocxx::database db);
    virtual ~MongoJobRepo() = default;

    void InsertJob(const dpp::snowflake& guildID, const std::shared_ptr<const JobRequest>& job) override;
    void UpdateJob(const std::shared_ptr<const JobRequest>& job) override;
    void DeleteJob(const GUID& id) override;

    std::vector<dpp::snowflake> GetGuildsWithJobs() override;
    std::vector<std::shared_ptr<JobRequest>> LoadJobs(dpp::snowflake guildID) override;

private:
    void CreateIndexes();

    mongocxx::collection m_collection;
};