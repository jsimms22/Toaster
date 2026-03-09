#include "JobQueue.h"

#include "JobRequest.h"
#include "BotUtility.h"
#include "IJobRepo.h"

#include "JobRequestFactory.h"
#include "CraftingJobRequest.h"
#include "BuildingJobRequest.h"
#include "ComponentJobRequest.h"
#include "ResourceJobRequest.h"
#include "HazardousRequest.h"
// d++
#include <dpp/cluster.h>
// fmt
#include <fmt/format.h>
// tinyxml
#include "tinyxml2.h"
// std library
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <stdexcept>

const std::size_t JobQueue::JOBS_PER_QUEUE_PAGE{ 6 };
const std::size_t JobQueue::JOBS_PER_DETAIL_PAGE{ 3 };

namespace
{
    bool ComparePriority(const std::shared_ptr<const JobRequest>& a, const std::shared_ptr<const JobRequest>& b)
    {
        if (a->GetPriority() == b->GetPriority())
        {
            return a->GetCreatedTime() < b->GetCreatedTime();
        }

        return a->GetPriority() > b->GetPriority();  // Higher priority comes first
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief Constructor: Reads the XML and loads the job requests into the container
//---------------------------------------------------------------------------------------------------------------------
JobQueue::JobQueue(const dpp::snowflake& guildID, std::shared_ptr<IJobRepo> repo)
    : m_guildID{guildID}, m_repo{repo}
{
    m_vQueue = m_repo->LoadJobs(m_guildID);
    std::sort(m_vQueue.begin(), m_vQueue.end(), ComparePriority);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief Destructor. Frees the worker thread of its task and saves the queue as is to persistent storage.
//---------------------------------------------------------------------------------------------------------------------
JobQueue::~JobQueue() = default;

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
/*
void JobQueue::LoadFromXml(tinyxml2::XMLElement* requestList)
{
    if (!requestList)
        return;

    for (tinyxml2::XMLElement* req = requestList->FirstChildElement("Request");
        req != nullptr;
        req = req->NextSiblingElement("Request"))
    {
        const char* typeAttr = req->Attribute("Type");
        if (!typeAttr)
            continue;

        auto job = JobRequestFactory::Create(std::stoul(typeAttr), m_guildID);
        if (!job)
            continue;

        job->ReadAttributes(req);
        m_vQueue.emplace_back(std::move(job));
    }
}
*/

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void JobQueue::RequestAddToQueue(std::shared_ptr<JobRequest> job)
{
    std::unique_lock<std::shared_mutex> lock(m_mtxShared, std::defer_lock);

    lock.lock();

    if (!job) { return; }
    GUID guid = job->GetID();
    m_vQueue.emplace_back(std::move(job));

    std::sort(m_vQueue.begin(), m_vQueue.end(), ComparePriority);

    lock.unlock();

    AddJobDB(guid);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const bool JobQueue::RequestDeleteJobByGUID(const GUID& guid)
{
    std::unique_lock<std::shared_mutex> lock(m_mtxShared, std::defer_lock);

    lock.lock();

    bool bResult{ false };
    for (auto itr = m_vQueue.begin(); itr != m_vQueue.end(); ++itr)
    {
        if ((*itr)->GetID() == guid)
        {
            itr->reset();
            m_vQueue.erase(itr);
            bResult = true;
            break;
        }
    }

    std::sort(m_vQueue.begin(), m_vQueue.end(), ComparePriority);

    lock.unlock();

    RemoveJobDB(guid);

    return bResult;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void JobQueue::RequestModifyJob(const GUID& guid, JobMutation mutator)
{
    std::unique_lock<std::shared_mutex> lock(m_mtxShared, std::defer_lock);

    lock.lock();

    auto job = GetJobByGUID_NoLock(guid);

    if (!job) { return; }

    mutator(job);
    job->SetLastEditTime(utils::GetEpochTimestamp());

    std::sort(m_vQueue.begin(), m_vQueue.end(), ComparePriority);

    lock.unlock();

    UpdateJobDB(guid);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void JobQueue::AddJobDB(const GUID& guid)
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);
    auto job = GetJobByGUID_NoLock(guid);
    m_repo->InsertJob(m_guildID, job);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void JobQueue::UpdateJobDB(const GUID& guid)
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);
    auto job = GetJobByGUID_NoLock(guid);
    m_repo->UpdateJob(job);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void JobQueue::RemoveJobDB(const GUID& guid)
{
    m_repo->DeleteJob(guid);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<JobRequest> JobQueue::GetJobByGUID_NoLock(const GUID& guid) const
{
    auto it = std::find_if(
        m_vQueue.begin(),
        m_vQueue.end(),
        [&guid](const std::shared_ptr<JobRequest>& job)
        {
            return job && job->GetID() == guid;
        }
    );

    if (it != m_vQueue.end())
        return *it;

    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::shared_ptr<const JobRequest> JobQueue::GetJobByGUID(const GUID& guid) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);
    return GetJobByGUID_NoLock(guid);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::shared_ptr<const JobRequest> JobQueue::GetJobByGUID(const std::string& guid) const
{
    GUID search;
    if (guid.at(0) != '{')
        search = utils::StringToGuid("{" + guid + "}");
    else
        search = utils::StringToGuid(guid);

    std::shared_lock<std::shared_mutex> lock(m_mtxShared);
    return GetJobByGUID_NoLock(search);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief Method to print all job requests
//---------------------------------------------------------------------------------------------------------------------
const std::string JobQueue::PrintQueue(dpp::cluster& cluster, const dpp::snowflake& idGuild) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

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
            "**Customer**: {}\n"
            "**Assigned**: {}\n"
            "**Type**: {}\n\n",
            position,
            JobRequest::PriorityToString(job->GetPriority()),
            utils::GuidToStringNoBrackets(job->GetID()),
            JobRequest::StatusToString(job->GetStatus()),
            job->GetCustomerName(cluster, idGuild),
            job->GetWorkerName(cluster, idGuild),
            job->JobTypeToString()
        );
    }

    // Convert buffer to std::string once at the end
    return fmt::to_string(buffer);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::string JobQueue::PrintQueueAdminSummary(dpp::cluster& cluster) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    std::map<std::size_t, std::size_t> typeCounts;
    std::map<JobRequest::status, std::size_t> statusCounts;
    std::map<JobRequest::priority, std::size_t> priorityCounts;
    std::size_t unassignedCounts{ 0 };

    // Collect counts
    for (const auto& job : m_vQueue)
    {
        typeCounts[job->JobType()]++;
        statusCounts[job->GetStatus()]++;
        priorityCounts[job->GetPriority()]++;

        if ((job->GetWorkerID() == "0" || job->GetWorkerID() == "") && job->GetStatus() < JobRequest::status::hold)
            ++unassignedCounts;
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

    // Convert memory_buffer to std::string
    return fmt::to_string(buffer);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::string JobQueue::PrintQueueWorkerSummary(dpp::cluster& cluster, const dpp::snowflake& worker) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    std::map<std::size_t, std::size_t> typeCounts;
    std::map<JobRequest::status, std::size_t> statusCounts;
    std::map<JobRequest::priority, std::size_t> priorityCounts;
    std::vector<GUID> assignedActiveIDs;

    // Collect counts
    for (const auto& job : m_vQueue)
    {
        if ((job->GetWorkerID() != worker))
            continue;

        typeCounts[job->JobType()]++;
        statusCounts[job->GetStatus()]++;
        priorityCounts[job->GetPriority()]++;

        if (job->GetStatus() < JobRequest::status::hold && assignedActiveIDs.size() < 5)
            assignedActiveIDs.push_back(job->GetID());
    }

    fmt::memory_buffer buffer;

    // Type summary
    fmt::format_to(std::back_inserter(buffer), "**Jobs in Queue (By Type)**:\n");
    for (const auto& [type, count] : typeCounts)
    {
        fmt::format_to(std::back_inserter(buffer), "  {}: {}\n", utils::JobTypeToString(type), count);
    }

    // Status summary
    fmt::format_to(std::back_inserter(buffer), "\n**Jobs in Queue (By Status)**:\n");
    for (const auto& [status, count] : statusCounts)
    {
        fmt::format_to(std::back_inserter(buffer), "  {}: {}\n", JobRequest::StatusToString(status), count);
    }

    // Priority summary
    fmt::format_to(std::back_inserter(buffer), "\n**Jobs in Queue (By Priority)**:\n");
    for (const auto& [priority, count] : priorityCounts)
    {
        fmt::format_to(std::back_inserter(buffer), "  {}: {}\n", JobRequest::PriorityToString(priority), count);
    }

    // Stalled job IDs
    fmt::format_to(std::back_inserter(buffer), "\n**Top 5 Active Job IDs**:\n");
    for (const auto& id : assignedActiveIDs)
    {
        fmt::format_to(std::back_inserter(buffer), "  {}\n", utils::GuidToStringNoBrackets(id));
    }

    // Convert memory_buffer to std::string
    return fmt::to_string(buffer);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::string JobQueue::PrintQueueSummary(dpp::cluster& cluster) const
{
    const std::string strAdminPortion = PrintQueueAdminSummary(cluster);
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    std::vector<GUID> stalledJobs;

    // Collect counts
    for (const auto& job : m_vQueue)
    {
        if (job->GetStatus() == JobRequest::status::stalled)
            stalledJobs.push_back(job->GetID());
    }

    fmt::memory_buffer buffer;

    // Stalled job IDs
    fmt::format_to(std::back_inserter(buffer), "\n**Stalled Job IDs**:\n");
    for (const auto& id : stalledJobs)
    {
        fmt::format_to(std::back_inserter(buffer), "  {}\n", utils::GuidToStringNoBrackets(id));
    }

    // Convert memory_buffer to std::string
    return strAdminPortion + fmt::to_string(buffer);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::string JobQueue::PrintQueue(dpp::cluster& cluster, const dpp::snowflake& idGuild, JobCompare compare) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    std::size_t position = 0;

    fmt::memory_buffer buffer;
    for (const auto& job : m_vQueue)
    {
        ++position;

        if (!compare(job))
            continue;

        fmt::format_to(
            std::back_inserter(buffer),
            "***Position: {}***\n{}\n",
            position,
            job->PrintJobDetails(cluster, idGuild)
        );
    }

    // Convert buffer to std::string once at the end
    return fmt::to_string(buffer);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::string JobQueue::PrintQueueCompact(dpp::cluster& cluster, const dpp::snowflake& idGuild, JobCompare compare) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    std::size_t position = 0;

    fmt::memory_buffer buffer;
    for (const auto& job : m_vQueue)
    {
        ++position;

        if (!compare(job))
            continue;

        fmt::format_to(
            std::back_inserter(buffer),
            "***Position: {}***\n{}\n",
            position,
            job->PrintJobDetailsCompact(cluster, idGuild)
        );
    }

    // Convert buffer to std::string once at the end
    return fmt::to_string(buffer);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::string JobQueue::PrintPagedQueue(dpp::cluster& cluster,
                                            const dpp::snowflake& idGuild,
                                            const std::size_t page,
                                            JobCompare compare) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    const std::size_t start_index = page * JOBS_PER_DETAIL_PAGE;
    const std::size_t end_index = start_index + JOBS_PER_DETAIL_PAGE;

    std::size_t filtered_index = 0;
    std::size_t display_pos = 0;

    fmt::memory_buffer buffer;
    for (const auto& job : m_vQueue)
    {
        if (!compare(job))
            continue;

        // Only count filtered jobs
        if (filtered_index >= end_index)
            break;

        if (filtered_index >= start_index)
        {
            ++display_pos;

            fmt::format_to(
                std::back_inserter(buffer),
                "***Position: {}***\n{}\n",
                start_index + display_pos,
                job->PrintJobDetails(cluster, idGuild)
            );
        }

        ++filtered_index;
    }

    return fmt::to_string(buffer);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::string JobQueue::PrintPagedQueueCompact(dpp::cluster& cluster,
                                                   const dpp::snowflake& idGuild,
                                                   const std::size_t page,
                                                   JobCompare compare) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    const std::size_t start_index = page * JOBS_PER_QUEUE_PAGE;
    const std::size_t end_index = start_index + JOBS_PER_QUEUE_PAGE;

    std::size_t filtered_index = 0;
    std::size_t display_pos = 0;

    fmt::memory_buffer buffer;
    for (const auto& job : m_vQueue)
    {
        if (!compare(job))
            continue;

        // Only count filtered jobs
        if (filtered_index >= end_index)
            break;

        if (filtered_index >= start_index)
        {
            ++display_pos;

            fmt::format_to(
                std::back_inserter(buffer),
                "***Position: {}***\n{}\n",
                start_index + display_pos,
                job->PrintJobDetailsCompact(cluster, idGuild)
            );
        }

        ++filtered_index;
    }

    return fmt::to_string(buffer);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<const JobRequest> JobQueue::FirstAssignment(const dpp::snowflake& userID)
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    for (std::shared_ptr<JobRequest>& job : m_vQueue)
    {
        if (job->GetWorkerID() != userID || job->GetStatus() == JobRequest::status::complete)
            continue;
           
        return job;
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::size_t JobQueue::GetQueueSize(JobCompare compare) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    return std::count_if(
        m_vQueue.begin(),
        m_vQueue.end(),
        [compare](const auto& job) {
            return compare(job);
        }
    );
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::size_t JobQueue::GetQueueSize() const
{
    std::shared_lock lock(m_mtxShared);
    return m_vQueue.size();
}