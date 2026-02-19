#pragma once
#include "JobRequest.h"
// Microsoft
#include <guiddef.h>

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

    std::string PrintQueue() const;
    std::string PrintQueueByUser(const std::string username) const;

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




