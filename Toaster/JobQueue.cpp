#include "JobQueue.h"

#include "BotUtility.h"

#include "JobRequestFactory.h"
#include "CraftingJobRequest.h"
#include "BuildingJobRequest.h"
#include "ComponentJobRequest.h"
#include "ResourceJobRequest.h"
// d++
#include <dpp/cluster.h>
// fmt
#include <fmt/format.h>
// tinyxml
#include "tinyxml2.h"
// std library
#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace xmlQueue
{
    const char* pszXMLRequestUser{ "Requester" };
    const char* pszXMLJobWorker{ "Worker" };
    const char* pszXMLRequestSCHandle{ "GameHandle" };
    const char* pszXMLJobPriority{ "Priority" };
    const char* pszXMLJobStatus{ "Status" };
    const char* pszXMLJobGUID{ "GUID" };
    const char* pszXMLJobType{ "Type" };
}

// Constructor: Reads the XML and loads the job requests into the container
JobQueue::JobQueue()
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile("queue.xml");

    // Get the root element (<RequestQueue>)
    tinyxml2::XMLElement* root = doc.RootElement();
    if (!root)
    {
        root = doc.NewElement("RequestList");
        doc.InsertEndChild(root);
    }

    // Iterate over all <Request> elements within each <User>
    for (tinyxml2::XMLElement* xmlNode = root->FirstChildElement("Request"); xmlNode != nullptr; xmlNode = xmlNode->NextSiblingElement("Request"))
    {
        const char* type = xmlNode->Attribute(xmlQueue::pszXMLJobType);
        if (type)
        {
            std::shared_ptr<JobRequest> job = JobRequestFactory::Create(std::stoul(type));
            if (!job)
            {
                continue;
            }

            job->ReadAttributes(xmlNode, root);
            m_vQueue.emplace_back(std::move(job));
        }
    }
}

std::shared_ptr<JobRequest> JobQueue::GetJobByGUID(const std::string& searchGUID)
{
    for (std::shared_ptr<JobRequest>& job : m_vQueue)
    {
        if (utils::GuidToStringNoBrackets(job->GetID()) == searchGUID)
        {
            return job;
        }
    }

    return {};
}

bool JobQueue::DeleteJobByGUID(const std::string& searchGUID)
{
    for (auto itr = m_vQueue.begin(); itr < m_vQueue.cend(); ++itr)
    {
        if (utils::GuidToStringNoBrackets((*itr)->GetID()) == searchGUID)
        {
            (*itr).reset();
            m_vQueue.erase(itr);
            return true;
        }
    }

    return false;
}

// Method to print all job requests
const std::string JobQueue::PrintQueue(dpp::cluster& cluster) const
{
    fmt::memory_buffer buffer;
    std::size_t position = 0;

    for (const auto& job : m_vQueue)
    {
        ++position;

        fmt::format_to(
            std::back_inserter(buffer),
            "***Position: {}***\n"
            "ID (**{}**): {}\n"
            "**Status**: {}\n"
            "**Assigned**: {}\n"
            "**Type**: {}\n\n",
            position,
            JobRequest::PriorityToString(job->GetPriority()),
            utils::GuidToStringNoBrackets(job->GetID()),
            JobRequest::StatusToString(job->GetStatus()),
            job->GetWorkerName(cluster),
            job->JobTypeToString()
        );
    }

    // Convert buffer to std::string once at the end
    return fmt::to_string(buffer);
}

const std::string JobQueue::PrintQueueSummary(dpp::cluster& cluster) const
{
    std::map<std::size_t, std::size_t> typeCounts;
    std::map<JobRequest::status, std::size_t> statusCounts;
    std::map<JobRequest::priority, std::size_t> priorityCounts;
    std::size_t unassignedCounts{ 0 };
    std::vector<GUID> stalledJobs;

    // Collect counts
    for (const auto& job : m_vQueue)
    {
        typeCounts[job->JobType()]++;
        statusCounts[job->GetStatus()]++;
        priorityCounts[job->GetPriority()]++;

        if ((job->GetWorkerID() == "0" || job->GetWorkerID() == "") && job->GetStatus() < JobRequest::status::hold)
            ++unassignedCounts;

        if (job->GetStatus() == JobRequest::status::stalled)
            stalledJobs.push_back(job->GetID());
    }

    fmt::memory_buffer buffer;

    // Type summary
    fmt::format_to(std::back_inserter(buffer), "**Job Types**:\n");
    for (const auto& [type, count] : typeCounts)
    {
        fmt::format_to(std::back_inserter(buffer), "  {}: {}\n", utils::JobTypeToString(type), count);
    }

    // Status summary
    fmt::format_to(std::back_inserter(buffer), "\n**Job Statuses**:\n");
    for (const auto& [status, count] : statusCounts)
    {
        fmt::format_to(std::back_inserter(buffer), "  {}: {}\n", JobRequest::StatusToString(status), count);
    }

    // Priority summary
    fmt::format_to(std::back_inserter(buffer), "\n**Job Priorities**:\n");
    for (const auto& [priority, count] : priorityCounts)
    {
        fmt::format_to(std::back_inserter(buffer), "  {}: {}\n", JobRequest::PriorityToString(priority), count);
    }

    // Unassigned summary
    fmt::format_to(std::back_inserter(buffer), "\n**Open Jobs Unassigned**:\n");
    fmt::format_to(std::back_inserter(buffer), "  Unassigned: {}\n", unassignedCounts);

    // Stalled job IDs
    fmt::format_to(std::back_inserter(buffer), "\n**Stalled Job IDs**:\n");
    for (const auto& id : stalledJobs)
    {
        fmt::format_to(std::back_inserter(buffer), "  {}\n", utils::GuidToStringNoBrackets(id));
    }

    // Convert memory_buffer to std::string
    return fmt::to_string(buffer);
}

const std::string JobQueue::PrintQueueByStatus(dpp::cluster& cluster, const JobRequest::status filter) const
{
    fmt::memory_buffer buffer;
    std::size_t position = 0;

    for (const auto& job : m_vQueue)
    {
        ++position;

        if (job->GetStatus() != filter)
            continue;

        fmt::format_to(
            std::back_inserter(buffer),
            "***Position: {}***\n"
            "ID (**{}**): {}\n"
            "**Status**: {}\n"
            "**Assigned**: {}\n"
            "**Type**: {}\n\n",
            position,
            JobRequest::PriorityToString(job->GetPriority()),
            utils::GuidToStringNoBrackets(job->GetID()),
            JobRequest::StatusToString(job->GetStatus()),
            job->GetWorkerName(cluster),
            job->JobTypeToString()
        );
    }

    // Convert buffer to std::string once at the end
    return fmt::to_string(buffer);
}

// Method to print all job requests
const std::string JobQueue::PrintQueueByType(dpp::cluster& cluster, const std::size_t filter) const
{
    fmt::memory_buffer buffer;
    std::size_t position = 0;

    for (const auto& job : m_vQueue)
    {
        ++position;

        if (!job->SupportsType(filter))
            continue;

        fmt::format_to(
            std::back_inserter(buffer),
            "***Position: {}***\n"
            "ID (**{}**): {}\n"
            "**Status**: {}\n"
            "**Assigned**: {}\n"
            "**Type**: {}\n\n",
            position,
            JobRequest::PriorityToString(job->GetPriority()),
            utils::GuidToStringNoBrackets(job->GetID()),
            JobRequest::StatusToString(job->GetStatus()),
            job->GetWorkerName(cluster),
            job->JobTypeToString()
        );
    }

    // Convert buffer to std::string once at the end
    return fmt::to_string(buffer);
}

const std::string JobQueue::PrintQueueByUser(dpp::cluster& cluster, const dpp::snowflake& userID, const std::size_t filter) const
{
    fmt::memory_buffer buffer;
    std::size_t position = 0;

    for (const auto& job : m_vQueue)
    {
        ++position;

        if (!job->SupportsType(filter))
            continue;

        if (job->GetCustomerID() == userID)
        {
            fmt::format_to(
                std::back_inserter(buffer),
                "***Position: {}***\n{}\n",
                position,
                job->PrintJobDetails(cluster)
            );
        }
    }

    // Convert the buffer to a std::string once at the end
    return fmt::to_string(buffer);
}

const std::string JobQueue::PrintQueueByWorker(dpp::cluster& cluster, const dpp::snowflake& userID) const
{
    std::stringstream ss;
    for (const auto& job : m_vQueue)
    {
        if (job->GetWorkerID() == userID)
        {
            ss << job->PrintJobDetails(cluster) << '\n';
        }
    }

    return ss.str();
}

const std::string JobQueue::PrintFirstAssignment(dpp::cluster& cluster, const dpp::snowflake& userID) const
{
    for (const auto& job : m_vQueue)
    {
        if (job->GetWorkerID() == userID)
        {
            return job->PrintJobDetails(cluster);
        }
    }

    return {};
}

std::shared_ptr<JobRequest> JobQueue::FirstAssignment(const dpp::snowflake& userID)
{
    for (std::shared_ptr<JobRequest>& job : m_vQueue)
    {
        if (job->GetWorkerID() == userID)
        {
            return job;
        }
    }

    return {};
}

void JobQueue::SaveQueueToFile()
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile("queue.xml");
    doc.Clear();

    // Get the root element (<RequestQueue>)
    tinyxml2::XMLElement* root = doc.RootElement();
    if (!root)
    {
        root = doc.NewElement("RequestList");
        doc.InsertEndChild(root);
    }

    std::sort(m_vQueue.begin(), m_vQueue.end(), [](const std::shared_ptr<JobRequest>& a, const std::shared_ptr<JobRequest>& b)
        {
            return a->GetPriority() > b->GetPriority();  // Higher priority comes first
        });

    for (const auto& job : m_vQueue)
    {
        tinyxml2::XMLElement* xmlNode = root->InsertNewChildElement("Request");
        job->WriteAttributes(xmlNode, root);
        root->InsertEndChild(xmlNode);
    }

    // Save the updated XML to a file
    doc.SaveFile("queue.xml");
}

bool JobQueue::IsInQueue(const std::string& strID) const
{
    const auto& itr = std::find_if(m_vQueue.cbegin(), m_vQueue.cend(), [&strID](const auto& job) -> 
                            bool { return utils::GuidToStringNoBrackets(job->GetID()) == strID; });
    return (itr != m_vQueue.cend());

}
