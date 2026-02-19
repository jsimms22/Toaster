#pragma once
#include "JobRequest.h"
// Microsoft
#include <guiddef.h>

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

class JobQueue
{
public:
    JobQueue();
    ~JobQueue() { SaveQueueToFile(); }

    std::shared_ptr<JobRequest> GetJobByGUID(const std::string& searchGUID);
    bool DeleteJobByGUID(const std::string& searchGUID);

    const std::string PrintQueue() const; 
    const std::string PrintQueueByType(const std::size_t filter) const;
    const std::string PrintQueueByUser(const std::string username, const std::size_t filter) const;
    const std::string PrintQueueByWorker(const std::string worker) const; 
    const std::string PrintFirstAssignment(const std::string worker) const;

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




