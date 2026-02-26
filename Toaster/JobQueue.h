#pragma once
#include "JobRequest.h"
// d++
#include <dpp/snowflake.h>
// microsoft
#include <guiddef.h>
// std library
#include <cstdlib>
#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

class dpp::cluster;

class JobQueue
{
public:
    static const std::size_t JOBS_PER_QUEUE_PAGE;
    static const std::size_t JOBS_PER_DETAIL_PAGE;

    JobQueue();
    ~JobQueue() { SaveQueueToFile(); }

    std::shared_ptr<JobRequest> GetJobByGUID(const std::string& searchGUID);
    bool DeleteJobByGUID(const std::string& searchGUID);

    // For Managers and Workers
    const std::string PrintQueue(dpp::cluster& cluster, 
                                 const dpp::snowflake& idGuild) const;
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

    void SaveQueueToFile();
    void AddToQueue(std::shared_ptr<JobRequest> job);
    bool IsInQueue(const std::string& strID) const;
    const std::size_t GetQueueSize() const { return m_vQueue.size(); }
    const std::size_t GetFilteredQueueSizeByType(const std::size_t filter) const;
    const std::size_t GetFilteredQueueSizeByStatus(const JobRequest::status filter) const;
    const std::size_t GetFilteredQueueSizeByWorker(const dpp::snowflake& worker) const;
    const std::size_t GetFilteredQueueSizeByUser(const dpp::snowflake& user, const std::size_t filter) const;

private:
    std::vector<std::shared_ptr<JobRequest>> m_vQueue;
    mutable std::shared_mutex m_mutex;
};




