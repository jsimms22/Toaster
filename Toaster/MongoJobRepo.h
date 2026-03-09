#pragma once
#include "IJobRepo.h"
// mongo
#include <mongocxx/database.hpp>
#include <bsoncxx/document/view_or_value.hpp>
// std library
#include <string>

class MongoJobRepo : public IJobRepo
{
public:
    MongoJobRepo(mongocxx::database db);
    virtual ~MongoJobRepo() = default;

    // Job Queue
    std::vector<dpp::snowflake> GetGuildsWithJobs() override;
    std::vector<std::shared_ptr<JobRequest>> LoadJobs(const dpp::snowflake& guildID) override;

    void InsertJob(const dpp::snowflake& guildID, const std::shared_ptr<const JobRequest>& job) override;
    void UpdateJob(const std::shared_ptr<const JobRequest>& job) override;
    void DeleteJob(const GUID& id) override;

    // Guild Settings
    std::vector<dpp::snowflake> GetGuildsWithSettings() override;
    bool LoadGuildSettings(const std::shared_ptr<GuildSettings>& settings) override;

    void InsertGuild(const std::shared_ptr<const GuildSettings>& settings) override;
    void UpdateGuild(const dpp::snowflake& guildID, const std::shared_ptr<const GuildSettings>& settings) override;
    void DeleteGuild(const dpp::snowflake& guildID) override;

    // Custom Mongo Interfaces
    void Insert(const std::string& name, bsoncxx::document::view_or_value document);
    void Update(const std::string& name, bsoncxx::document::view_or_value filter, bsoncxx::document::view_or_value replacement);
    void Delete(const std::string& name, bsoncxx::document::view_or_value filter);

protected:
    // Worker Thread
    void StartWorker() override;
    void EnqueueDBMutation(Mutation mut) override;
    void DatabaseWorker(std::stop_token stopToken) override;

private:
    void CreateIndexes();

    mongocxx::collection m_jobsCollection;
    mongocxx::collection m_guildsCollection;
};