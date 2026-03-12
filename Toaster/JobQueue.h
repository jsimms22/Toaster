//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
// d++
#include <dpp/cluster.h>
#include <dpp/snowflake.h>
// std library
#include <cstdlib>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

class JobRequest;
class IJobRepo;
struct RequestID;

//---------------------------------------------------------------------------------------------------------------------
/// \class ToasterBot
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class JobQueue
{
public:
    using JobCompare = std::function<bool(const std::shared_ptr<const JobRequest>)>;
    using JobMutation = std::function<void(std::shared_ptr<JobRequest>)>;

    static const std::size_t JOBS_PER_QUEUE_PAGE;
    static const std::size_t JOBS_PER_DETAIL_PAGE;

    JobQueue(const dpp::snowflake& guildID, std::shared_ptr<IJobRepo> repo);
    ~JobQueue();

    // For Managers and Workers
    const std::string PrintQueue(dpp::cluster& cluster, const dpp::snowflake& idGuild) const;

    // Queue Summary Methods
    const std::string PrintQueueAdminSummary(dpp::cluster& cluster) const; 
    const std::string PrintQueueWorkerSummary(dpp::cluster& cluster, const dpp::snowflake& worker) const;
    const std::string PrintQueueSummary(dpp::cluster& cluster) const;

    // Print Queue Methods
    const std::string PrintQueue(dpp::cluster& cluster, const dpp::snowflake& idGuild, JobCompare compare) const;
    const std::string PrintQueueCompact(dpp::cluster& cluster, const dpp::snowflake& idGuild, JobCompare compare) const;
    const std::string PrintPagedQueue(dpp::cluster& cluster, const dpp::snowflake& idGuild, const std::size_t page, JobCompare compare) const;
    const std::string PrintPagedQueueCompact(dpp::cluster& cluster, const dpp::snowflake& idGuild, const std::size_t page, JobCompare compare) const;

    // Queue Size Methods
    const std::size_t GetQueueSize(JobCompare compare) const;
    const std::size_t GetQueueSize() const;

    // Retrieve Job Methods
    std::shared_ptr<const JobRequest> FirstAssignment(const dpp::snowflake& userID);
    const std::shared_ptr<const JobRequest> GetJobByID(const RequestID rID) const;
    const std::shared_ptr<const JobRequest> GetJobByID(const std::string& rID) const;

    // Job Mutation Methods
    void RequestModify(const RequestID rID, JobMutation mutator);
    void RequestAdd(std::shared_ptr<JobRequest> job);
    const bool RequestDelete(const RequestID rID);

private:
    // Worker thread methods
    std::shared_ptr<JobRequest> GetJobByID_NoLock(const RequestID rID) const;
    std::shared_ptr<JobRequest> GetJobByID_NoLock(const std::string& rID) const;

    // Mongo Database Methods
    void AddJobDB(const RequestID rID);
    void UpdateJobDB(const RequestID rID);
    void RemoveJobDB(const RequestID rID);

    dpp::snowflake m_guildID;
    std::vector<std::shared_ptr<JobRequest>> m_vQueue;
    mutable std::shared_mutex m_mtxShared;

    std::shared_ptr<IJobRepo> m_repo;
};




