//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "Toaster.h"
// d++
#include <dpp/cluster.h>
#include <dpp/snowflake.h>
// std library
#include <cstdint>
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
    const std::string PrintQueue(dpp::cluster& cluster, const dpp::snowflake& idGuild, const bool bShowComplete, JobCompare compare) const;
    const std::string PrintQueueCompact(dpp::cluster& cluster, const dpp::snowflake& idGuild, const bool bShowComplete, JobCompare compare) const;
    const std::string PrintPagedQueue(dpp::cluster& cluster, const dpp::snowflake& idGuild, const std::size_t page, const bool bShowComplete, JobCompare compare) const;
    const std::string PrintPagedQueueCompact(dpp::cluster& cluster, const dpp::snowflake& idGuild, const std::size_t page, const bool bShowComplete, JobCompare compare) const;
    // Print Request Methods
    const std::string PrintPagedRequest(dpp::cluster& cluster, const dpp::snowflake& idGuild, const std::size_t page, const bool bShowComplete, JobCompare compare) const;
    // Queue Size Methods
    const std::size_t GetQueueSize(const bool bShowComplete, JobCompare compare) const;
    const std::size_t GetQueueSize() const;

    // Archive Methods
    const std::string PrintArchive(dpp::cluster& cluster, const dpp::snowflake& idGuild, JobCompare compare) const;
    const std::size_t GetArchiveSize() const;

    // Retrieve Job Methods
    const std::shared_ptr<const JobRequest> FirstAssignment(JobCompare compare, const std::size_t offset = 0);
    const std::shared_ptr<const JobRequest> FirstAssignment(const dpp::snowflake& userID);
    const std::shared_ptr<const JobRequest> GetJobByID(const RequestID rID, const bool bArchived = false) const;
    const std::shared_ptr<const JobRequest> GetJobByID(const std::string& rID, const bool bArchived = false) const;

    // Job Mutation Methods
    void RequestModify(const RequestID rID, JobMutation mutator);
    void RequestAdd(std::shared_ptr<JobRequest> job, const bool bReOpenedJob = false);
    const bool RequestDelete(const RequestID rID);
    void ReOpenArchivedJob(const RequestID rID);

private:
    // Worker thread methods
    std::shared_ptr<JobRequest> GetJobByID_NoLock(const RequestID rID, const bool bArchived = false) const;
    std::shared_ptr<JobRequest> GetJobByID_NoLock(const std::string& rID, const bool bArchived = false) const;

    // Automatic processes
    void AutomatedQueueScan(const std::uint64_t archivalAge, const std::uint64_t stalledAge);

    // Mongo Database Methods
    void AddJobDB(const RequestID rID);
    void UpdateJobDB(const RequestID rID);
    void RemoveJobDB(const RequestID rID);
    void ArchiveJobsDB(const std::vector<RequestID>& ids);

    dpp::snowflake m_guildID;
    // Active queue
    std::vector<std::shared_ptr<JobRequest>> m_vQueue;
    mutable std::shared_mutex m_mtxShared;
    // Archived completed requests
    std::vector<std::shared_ptr<JobRequest>> m_vArchived;
    mutable std::shared_mutex m_mtxArchive;

    std::shared_ptr<IJobRepo> m_repo;

    friend ToasterBot;
};




