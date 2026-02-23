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
    JobQueue();
    ~JobQueue() { SaveQueueToFile(); }

    std::shared_ptr<JobRequest> GetJobByGUID(const std::string& searchGUID);
    bool DeleteJobByGUID(const std::string& searchGUID);

    const std::string PrintQueue(dpp::cluster& cluster) const; 
    const std::string PrintQueueSummary(dpp::cluster& cluster) const;
    const std::string PrintQueueByStatus(dpp::cluster& cluster, const JobRequest::status filter) const;
    const std::string PrintQueueByType(dpp::cluster& cluster, const std::size_t filter) const;
    const std::string PrintQueueByUser(dpp::cluster& cluster, const dpp::snowflake& userID, const std::size_t filter) const;
    const std::string PrintQueueByWorker(dpp::cluster& cluster, const dpp::snowflake& userID) const;
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

private:
    std::vector<std::shared_ptr<JobRequest>> m_vQueue;
};




