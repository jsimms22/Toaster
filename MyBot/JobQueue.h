#pragma once
#include "Common.h"
#include "JobRequest.h"

#include <vector>
#include <string>
#include <memory>

class JobQueue
{
public:
    // Constructor: Reads the XML and loads the job requests into the container
    JobQueue();
    ~JobQueue() 
    {
        SaveQueueToFile();
    }

    // Method to find a job request by GUID
    std::shared_ptr<JobRequest> FindRequestByGUID(const GUID& searchGUID);
    std::shared_ptr<JobRequest> GetJobByGUID(const std::string& searchGUID);
    bool DeleteJobByGUID(const std::string& searchGUID);

    // Method to print all job requests
    std::string PrintQueue() const;
    std::string PrintQueueByUser(const std::string username) const;
    bool IsInQueue(const std::string& strID) const;

    void AddToQueue(std::shared_ptr<JobRequest> job)
    {
        // Lastly add the job to the queue map and pass ownership
        m_vQueue.emplace_back(std::move(job));
        // Reorder and update file persistence
        SaveQueueToFile();
    }

    void SaveQueueToFile();

    const std::size_t GetQueueSize() const { return m_vQueue.size(); }

private:
    // <priority, guid>
    std::vector<std::shared_ptr<JobRequest>> m_vQueue;  // Container to store JobRequest objects
};




