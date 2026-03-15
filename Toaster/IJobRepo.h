#pragma once
// dpp
#include <dpp/snowflake.h>
// std library
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <stop_token>
#include <thread>
#include <vector>

class GuildSettings;
class JobRequest;
struct RequestID;

class IJobRepo
{
public:
    using Mutation = std::function<void(IJobRepo*)>;

    IJobRepo() = default;
    virtual ~IJobRepo()
    {
        if (m_worker.joinable())
        {
            m_worker.request_stop();
            m_cv.notify_all();
        }
    }

    // Job Queue
    virtual std::vector<dpp::snowflake> GetGuildsWithJobs() = 0;
    virtual std::vector<std::shared_ptr<JobRequest>> LoadJobs(const dpp::snowflake& guild) = 0;

    virtual void InsertJob(const std::shared_ptr<const JobRequest>& job) = 0;
    virtual void UpdateJob(const std::shared_ptr<const JobRequest>& job) = 0;
    virtual void DeleteJob(const RequestID) = 0;
    virtual void ArchiveJobs(const std::vector<RequestID>& ids) = 0;

    // Guild Settings
    virtual std::vector<dpp::snowflake> GetGuildsWithSettings() = 0;
    virtual bool LoadGuildSettings(const std::shared_ptr<GuildSettings>& settings) = 0;

    virtual void InsertGuild(const std::shared_ptr<const GuildSettings>& settings) = 0;
    virtual void UpdateGuild(const dpp::snowflake& guildID, const std::shared_ptr<const GuildSettings>& settings) = 0;
    virtual void DeleteGuild(const dpp::snowflake& guildID) = 0;

protected:
    // Worker Thread
    virtual void StartWorker() = 0;
    virtual void EnqueueDBMutation(Mutation mut) = 0;
    virtual void DatabaseWorker(std::stop_token stopToken) = 0;

    std::queue<Mutation> m_mutations;
    std::mutex m_mtxMutQueue;
    std::condition_variable m_cv;
    std::jthread m_worker;
};

