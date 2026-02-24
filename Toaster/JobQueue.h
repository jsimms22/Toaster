#pragma once
#include "JobRequest.h"
// d++
#include <dpp/snowflake.h>
// microsoft
#include <guiddef.h>
// std library
#include <cstdlib>
#include <memory>
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
    const std::string PrintQueue(dpp::cluster& cluster) const; 
    const std::string PrintQueueSummary(dpp::cluster& cluster) const;
    // By Status
    const std::string PrintQueueByStatus(dpp::cluster& cluster, const JobRequest::status filter) const;
    const std::string PrintQueuePageByStatus(dpp::cluster& cluster, 
                                             const JobRequest::status filter, 
                                             const std::size_t page) const;
    // By Type
    const std::string PrintQueueByType(dpp::cluster& cluster, const std::size_t filter) const;
    const std::string PrintQueuePageByType(dpp::cluster& cluster,
                                           const std::size_t filter,
                                           const std::size_t page) const;
    // For Everyone
    const std::string PrintQueueByUser(dpp::cluster& cluster, const dpp::snowflake& userID, const std::size_t filter) const;

    // For workers
    const std::vector<std::shared_ptr<JobRequest>> GetQueueByWorker(const dpp::snowflake& userID) const;
    const std::string PrintQueueByWorker(dpp::cluster& cluster, const dpp::snowflake& userID) const;
    const std::string PrintQueuePageByWorker(dpp::cluster& cluster, const dpp::snowflake& userID, const std::size_t page) const;
    const std::string PrintFirstAssignment(dpp::cluster& cluster, const dpp::snowflake& userID) const;
    std::shared_ptr<JobRequest> FirstAssignment(const dpp::snowflake& userID);

    void AddToQueue(std::shared_ptr<JobRequest> job)
    {
        m_vQueue.emplace_back(std::move(job));
        SaveQueueToFile();
    }
    bool IsInQueue(const std::string& strID) const;

    void SaveQueueToFile();

    const std::size_t GetQueueSize() const { return m_vQueue.size(); }
    const std::size_t GetFilteredQueueSize(const std::size_t filter) const
    {
        return std::count_if(
            m_vQueue.begin(),
            m_vQueue.end(),
            [filter](const auto& job) {
                return (job->SupportsType(filter) &&
                        job->GetStatus() != JobRequest::status::complete);
            }
        );
    }
    const std::size_t GetFilteredQueueSize(const JobRequest::status filter) const
    {
        return std::count_if(
            m_vQueue.begin(),
            m_vQueue.end(),
            [filter](const auto& job) {
                return job->GetStatus() == filter;
            }
        );
    }
    const std::size_t GetFilteredQueueSize(const dpp::snowflake& worker) const
    {
        return std::count_if(
            m_vQueue.begin(),
            m_vQueue.end(),
            [worker](const auto& job) {
                return (job->GetWorkerID() == worker && 
                        job->GetStatus() != JobRequest::status::complete);
            }
        );
    }

private:
    std::vector<std::shared_ptr<JobRequest>> m_vQueue;
};




