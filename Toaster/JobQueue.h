//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "JobRequest.h"
// d++
#include <dpp/snowflake.h>
// microsoft
#include <guiddef.h>
// std library
#include <condition_variable>
#include <cstdlib>
#include <future>
#include <memory>
#include <queue>
#include <shared_mutex>
#include <string>
#include <vector>

class dpp::cluster;

//---------------------------------------------------------------------------------------------------------------------
/// \class ToasterBot
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class JobQueue : public std::enable_shared_from_this<JobQueue>
{
public:
    using JobMutation = std::function<void(std::shared_ptr<JobRequest>)>;
    using QueueMutation = std::function<void(std::shared_ptr<JobQueue>)>;

    static const std::size_t JOBS_PER_QUEUE_PAGE;
    static const std::size_t JOBS_PER_DETAIL_PAGE;

    JobQueue();
    ~JobQueue();


    // For Managers and Workers
    const std::string PrintQueue(dpp::cluster& cluster, 
                                 const dpp::snowflake& idGuild) const; 
    const std::string PrintQueueAdminSummary(dpp::cluster& cluster) const;
    const std::string PrintQueueSummary(dpp::cluster& cluster) const;

    // By Status
    const std::string PrintQueueByStatus(dpp::cluster& cluster, 
                                         const JobRequest::status filter, 
                                         const dpp::snowflake& idGuild) const;
    const std::string PrintQueuePageByStatus(dpp::cluster& cluster, 
                                             const JobRequest::status filter, 
                                             const std::size_t page, 
                                             const dpp::snowflake& idGuild) const;

    // By Type
    const std::string PrintQueueByType(dpp::cluster& cluster, 
                                       const std::size_t filter, 
                                       const dpp::snowflake& idGuild) const;
    const std::string PrintQueuePageByType(dpp::cluster& cluster,
                                           const std::size_t filter,
                                           const std::size_t page, const dpp::snowflake& idGuild) const;

    // For Everyone
    const std::vector<std::shared_ptr<JobRequest>> GetQueueByUser(const dpp::snowflake& userID,
                                                                  const std::size_t filter) const;
    const std::string PrintQueueByUser(dpp::cluster& cluster, 
                                       const dpp::snowflake& userID, 
                                       const std::size_t filter, 
                                       const dpp::snowflake& idGuild) const;
    const std::string PrintQueuePageByUser(dpp::cluster& cluster, 
                                           const dpp::snowflake& userID,
                                           const std::size_t filter,
                                           const std::size_t page, 
                                           const dpp::snowflake& idGuild) const;
    // For workers
    const std::vector<std::shared_ptr<JobRequest>> GetQueueByWorker(const dpp::snowflake& userID) const;
    const std::string PrintQueueByWorker(dpp::cluster& cluster, const dpp::snowflake& userID, const dpp::snowflake& idGuild) const;
    const std::string PrintQueuePageByWorker(dpp::cluster& cluster, const dpp::snowflake& userID, const std::size_t page, const dpp::snowflake& idGuild) const;
    const std::string PrintFirstAssignment(dpp::cluster& cluster, const dpp::snowflake& userID, const dpp::snowflake& idGuild) const;
    std::shared_ptr<JobRequest> FirstAssignment(const dpp::snowflake& userID);

    bool IsInQueue(const std::string& strID) const;
    const std::size_t GetQueueSize() const { return m_vQueue.size(); }
    const std::size_t GetFilteredQueueSizeByType(const std::size_t filter) const;
    const std::size_t GetFilteredQueueSizeByStatus(const JobRequest::status filter) const;
    const std::size_t GetFilteredQueueSizeByWorker(const dpp::snowflake& worker) const;
    const std::size_t GetFilteredQueueSizeByUser(const dpp::snowflake& user, const std::size_t filter) const;

    std::shared_ptr<JobRequest> GetJobByGUID(const GUID& guid) const;
    std::shared_ptr<JobRequest> GetJobByGUID(const std::string& guid) const;

    void EnqueueMutation(QueueMutation mut);
    void RequestModifyJob(const GUID& guid, JobMutation mutator);
    void RequestAddToQueue(std::shared_ptr<JobRequest> job);
    std::future<bool> RequestDeleteJobByGUID(const GUID& guid);

private: // Methods
    std::shared_ptr<JobRequest> GetJobByGUID_NoLock(const GUID& guid) const;
    template<typename T>
    std::future<T> EnqueueTaskWithResult(std::function<T(std::shared_ptr<JobQueue>)> task);
    void MutationWorker();

    void SaveQueueToFile();

private: // Variables
    std::vector<std::shared_ptr<JobRequest>> m_vQueue;
    mutable std::shared_mutex m_mtxShared;

    std::queue<QueueMutation> m_mutations;
    std::mutex m_mtxMutQueue;
    std::condition_variable m_cv;
    std::jthread m_worker;
    bool m_bRunning = true;
};




