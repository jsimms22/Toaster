#include "JobQueue.h"

#include "JobRequest.h"
#include "BotUtility.h"
#include "IJobRepo.h"

#include "RequestID.h"
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
#include <iterator>
#include <stdexcept>

const std::size_t JobQueue::JOBS_PER_QUEUE_PAGE{ 5 };
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

    bool CompareEditTime(const std::shared_ptr<const JobRequest>& a, const std::shared_ptr<const JobRequest>& b)
    {
        return a->GetLastEditTime() < b->GetLastEditTime();
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

    m_vArchived = m_repo->LoadArchived(m_guildID);
    std::sort(m_vArchived.begin(), m_vArchived.end(), CompareEditTime);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief Destructor. Frees the worker thread of its task and saves the queue as is to persistent storage.
//---------------------------------------------------------------------------------------------------------------------
JobQueue::~JobQueue() = default;

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void JobQueue::RequestAdd(std::shared_ptr<JobRequest> job)
{
    std::unique_lock<std::shared_mutex> lock(m_mtxShared, std::defer_lock);

    lock.lock();

    if (!job) { return; }
    RequestID rID = job->GetID();
    m_vQueue.emplace_back(std::move(job));

    std::sort(m_vQueue.begin(), m_vQueue.end(), ComparePriority);

    lock.unlock();

    AddJobDB(rID);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const bool JobQueue::RequestDelete(const RequestID rID)
{
    std::unique_lock<std::shared_mutex> lock(m_mtxShared, std::defer_lock);

    lock.lock();

    bool bResult{ false };
    for (auto itr = m_vQueue.begin(); itr != m_vQueue.end(); ++itr)
    {
        if ((*itr)->GetID() == rID)
        {
            itr->reset();
            m_vQueue.erase(itr);
            bResult = true;
            break;
        }
    }

    std::sort(m_vQueue.begin(), m_vQueue.end(), ComparePriority);

    lock.unlock();

    RemoveJobDB(rID);

    return bResult;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void JobQueue::RequestModify(const RequestID rID, JobMutation mutator)
{
    std::unique_lock<std::shared_mutex> lock(m_mtxShared, std::defer_lock);

    lock.lock();

    auto job = GetJobByID_NoLock(rID);

    if (!job) { return; }

    mutator(job);
    job->SetLastEditTime(utils::GetEpochTimestamp());

    std::sort(m_vQueue.begin(), m_vQueue.end(), ComparePriority);

    lock.unlock();

    UpdateJobDB(rID);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void JobQueue::AutomatedQueueScan(std::uint64_t archivalAge, std::uint64_t stalledAge)
{
    const std::uint64_t now = utils::GetEpochTimestamp();
    const std::uint64_t archiveCutoff = now - archivalAge;
    const std::uint64_t stalledCutoff = now - stalledAge;

    std::vector<RequestID> archiveIDs;
    std::vector<RequestID> stalledIDs;
    std::vector<std::shared_ptr<JobRequest>> toArchive;

    {
        std::unique_lock<std::shared_mutex> lock(m_mtxShared);

        for (auto itr = m_vQueue.begin(); itr != m_vQueue.end(); )
        {
            auto& job = *itr;
            if (!job)
            {
                ++itr;
                continue;
            }

            /* Check if job needs to be archived */
            if (job->GetStatus() == JobRequest::status::complete &&
                job->GetLastEditTime() < archiveCutoff)
            {
                archiveIDs.push_back(job->GetID());

                // Move to local container for now, move to archive later under lock
                toArchive.push_back(std::move(job));

                // Remove from queue
                itr = m_vQueue.erase(itr);

                // Do not iterate after deleting
            }
            /* Check if job needs to be updated as stalled */
            else if ((job->GetStatus() == JobRequest::status::open ||
                job->GetStatus() == JobRequest::status::assigned) &&
                job->GetLastEditTime() < stalledCutoff)
            {
                stalledIDs.push_back(job->GetID());
                ++itr;
            }
            else
            {
                ++itr;
            }
        }
    }

    if (!archiveIDs.empty())
        m_repo->ArchiveJobs(archiveIDs);
    
    if (!stalledIDs.empty())
    {
        for (const auto id : stalledIDs)
        {
            RequestModify(id, [](const std::shared_ptr<JobRequest> job)
                {
                    job->SetStatus(JobRequest::status::stalled);
                });
        }
    }

    if (!toArchive.empty())
    {
        std::unique_lock<std::shared_mutex> lock(m_mtxArchive);
        m_vArchived.insert(m_vArchived.begin(),
            std::make_move_iterator(toArchive.begin()),
            std::make_move_iterator(toArchive.end()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void JobQueue::AddJobDB(const RequestID rID)
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);
    auto job = GetJobByID_NoLock(rID);
    m_repo->InsertJob(job);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void JobQueue::UpdateJobDB(const RequestID rID)
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);
    auto job = GetJobByID_NoLock(rID);
    m_repo->UpdateJob(job);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void JobQueue::RemoveJobDB(const RequestID rID)
{
    m_repo->DeleteJob(rID);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void JobQueue::ArchiveJobsDB(const std::vector<RequestID>& ids)
{
    if (ids.empty())
        return;

    m_repo->ArchiveJobs(ids);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<JobRequest> JobQueue::GetJobByID_NoLock(const RequestID rID) const
{
    auto it = std::find_if(
        m_vQueue.begin(),
        m_vQueue.end(),
        [&rID](const std::shared_ptr<JobRequest>& job)
        {
            return job && job->GetID() == rID;
        }
    );

    if (it != m_vQueue.end())
        return *it;

    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<JobRequest> JobQueue::GetJobByID_NoLock(const std::string& strID) const
{
    auto is_all_numbers = [](const std::string& s) -> bool {
        return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) {
            return std::isdigit(c);
            });
        };

    auto it = std::find_if(
        m_vQueue.begin(),
        m_vQueue.end(),
        [&strID, &is_all_numbers](const std::shared_ptr<JobRequest>& job)
        {
            if (!is_all_numbers(strID))
                return job && ToString(job->GetID()) == strID;
            else
                return job && job->GetID() == std::stoull(strID);
        }
    );

    if (it != m_vQueue.end())
        return *it;

    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::shared_ptr<const JobRequest> JobQueue::GetJobByID(const RequestID rID) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);
    return GetJobByID_NoLock(rID);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::shared_ptr<const JobRequest> JobQueue::GetJobByID(const std::string& strID) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);
    return GetJobByID_NoLock(strID);
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
            "{}",
            job->PrintJobDetails(cluster, idGuild)
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

        if (job->GetWorkerIDs().empty() && job->GetStatus() < JobRequest::status::hold)
            ++unassignedCounts;
    }

    fmt::memory_buffer buffer;

    // Type summary
    fmt::format_to(std::back_inserter(buffer), "### Jobs in Queue (By Type)\n");
    for (const auto& [type, count] : typeCounts)
    {
        fmt::format_to(std::back_inserter(buffer), "- {}: {}\n", utils::JobTypeToString(type), count);
    }

    // Status summary
    fmt::format_to(std::back_inserter(buffer), "\n### Jobs in Queue (By Status)\n");
    for (const auto& [status, count] : statusCounts)
    {
        fmt::format_to(std::back_inserter(buffer), "- {}: {}\n", JobRequest::StatusToString(status), count);
    }

    // Priority summary
    fmt::format_to(std::back_inserter(buffer), "\n### Jobs in Queue (By Priority)\n");
    for (const auto& [priority, count] : priorityCounts)
    {
        fmt::format_to(std::back_inserter(buffer), "- {}: {}\n", JobRequest::PriorityToString(priority), count);
    }

    // Unassigned summary
    fmt::format_to(std::back_inserter(buffer), "\n### Unassigned Open Jobs\n");
    fmt::format_to(std::back_inserter(buffer), "- Unassigned: {}\n", unassignedCounts);

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
    std::vector<RequestID> assignedActiveIDs;

    // Collect counts
    for (const auto& job : m_vQueue)
    {
        if (!job->IsWorker(worker))
            continue;

        typeCounts[job->JobType()]++;
        statusCounts[job->GetStatus()]++;
        priorityCounts[job->GetPriority()]++;

        if (job->GetStatus() < JobRequest::status::hold && assignedActiveIDs.size() < 5)
            assignedActiveIDs.push_back(job->GetID());
    }

    fmt::memory_buffer buffer;

    // Type summary
    fmt::format_to(std::back_inserter(buffer), "### Jobs in Queue (By Type)\n");
    for (const auto& [type, count] : typeCounts)
    {
        fmt::format_to(std::back_inserter(buffer), "- {}: {}\n", utils::JobTypeToString(type), count);
    }

    // Status summary
    fmt::format_to(std::back_inserter(buffer), "\n### Jobs in Queue (By Status)\n");
    for (const auto& [status, count] : statusCounts)
    {
        fmt::format_to(std::back_inserter(buffer), "- {}: {}\n", JobRequest::StatusToString(status), count);
    }

    // Priority summary
    fmt::format_to(std::back_inserter(buffer), "\n### Jobs in Queue (By Priority)\n");
    for (const auto& [priority, count] : priorityCounts)
    {
        fmt::format_to(std::back_inserter(buffer), "- {}: {}\n", JobRequest::PriorityToString(priority), count);
    }

    // Stalled job IDs
    fmt::format_to(std::back_inserter(buffer), "\n### Top 5 Active Job IDs\n");
    for (const auto& id : assignedActiveIDs)
    {
        fmt::format_to(std::back_inserter(buffer), "- {}\n", ToString(id));
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

    std::vector<RequestID> stalledJobs;

    // Collect counts
    for (const auto& job : m_vQueue)
    {
        if (job->GetStatus() == JobRequest::status::stalled)
            stalledJobs.push_back(job->GetID());

        if (stalledJobs.size() >= 5)
            break;
    }

    fmt::memory_buffer buffer;

    // Stalled job IDs
    fmt::format_to(std::back_inserter(buffer), "\n### Highest Priority Stalled Jobs:\n");
    for (const auto& id : stalledJobs)
    {
        fmt::format_to(std::back_inserter(buffer), "- {}\n", ToString(id));
    }

    // Convert memory_buffer to std::string
    return strAdminPortion + fmt::to_string(buffer);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::string JobQueue::PrintQueue(dpp::cluster& cluster, const dpp::snowflake& idGuild, const bool bShowComplete, JobCompare compare) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    std::size_t display_pos = 0;

    fmt::memory_buffer buffer;
    for (const auto& job : m_vQueue)
    {
        // This will consider completed tasks, even if they are not being shown
        ++display_pos;

        if (!bShowComplete && job->GetStatus() == JobRequest::status::complete)
            continue;

        if (!compare(job))
            continue;

        fmt::format_to(
            std::back_inserter(buffer),
            "### Position: {}\n{}\n",
            display_pos,
            job->PrintJobDetails(cluster, idGuild)
        );
    }

    // Convert buffer to std::string once at the end
    return fmt::to_string(buffer);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::string JobQueue::PrintQueueCompact(
    dpp::cluster& cluster, 
    const dpp::snowflake& idGuild, 
    const bool bShowComplete, 
    JobCompare compare) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    std::size_t display_pos = 0;

    fmt::memory_buffer buffer;
    for (const auto& job : m_vQueue)
    {
        // This will consider completed tasks, even if they are not being shown
        ++display_pos;

        if (!bShowComplete && job->GetStatus() == JobRequest::status::complete)
            continue;

        if (!compare(job))
            continue;

        fmt::format_to(
            std::back_inserter(buffer),
            "### --------- Position: {} ---------\n{}\n",
            display_pos,
            job->PrintJobDetailsCompact(cluster, idGuild)
        );
    }

    // Convert buffer to std::string once at the end
    return fmt::to_string(buffer);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::string JobQueue::PrintPagedRequest(
    dpp::cluster& cluster,
    const dpp::snowflake& idGuild,
    const std::size_t page,
    const bool bShowComplete,
    JobCompare compare) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    const std::size_t start_index = page * 1;
    const std::size_t end_index = start_index + 1;

    std::size_t filtered_index = 0;
    std::size_t display_pos = 0;

    fmt::memory_buffer buffer;
    for (const auto& job : m_vQueue)
    {
        // This will consider completed tasks, even if they are not being shown
        ++display_pos;

        if (!bShowComplete && job->GetStatus() == JobRequest::status::complete)
            continue;
        
        if (!compare(job))
            continue;

        // Only count filtered jobs
        if (filtered_index >= end_index)
            break;

        if (filtered_index >= start_index)
        {
            fmt::format_to(
                std::back_inserter(buffer),
                "### --------- Position: {} ---------\n{}\n",
                display_pos,
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
const std::string JobQueue::PrintPagedQueue(
    dpp::cluster& cluster,
    const dpp::snowflake& idGuild,
    const std::size_t page,
    const bool bShowComplete,
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
        // This will consider completed tasks, even if they are not being shown
        ++display_pos;

        if (!bShowComplete && job->GetStatus() == JobRequest::status::complete)
            continue;

        if (!compare(job))
            continue;

        // Only count filtered jobs
        if (filtered_index >= end_index)
            break;

        if (filtered_index >= start_index)
        {
            fmt::format_to(
                std::back_inserter(buffer),
                "### --------- Position: {} ---------\n{}\n",
                display_pos,
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
const std::string JobQueue::PrintPagedQueueCompact(
    dpp::cluster& cluster,
    const dpp::snowflake& idGuild,
    const std::size_t page, 
    const bool bShowComplete,
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
        // This will consider completed tasks, even if they are not being shown
        ++display_pos;

        if (!bShowComplete && job->GetStatus() == JobRequest::status::complete)
            continue;

        if (!compare(job))
            continue;

        // Only count filtered jobs
        if (filtered_index >= end_index)
            break;

        if (filtered_index >= start_index)
        {

            fmt::format_to(
                std::back_inserter(buffer),
                "### --------- Position: {} ---------\n{}\n",
                display_pos,
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
const std::shared_ptr<const JobRequest> JobQueue::FirstAssignment(JobCompare compare, const std::size_t offset)
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    std::size_t counter = 0;
    for (std::shared_ptr<JobRequest>& job : m_vQueue)
    {
        if (!compare(job))
            continue;

        ++counter;

        if (counter > offset)
            return job;
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::shared_ptr<const JobRequest> JobQueue::FirstAssignment(const dpp::snowflake& userID)
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    for (std::shared_ptr<JobRequest>& job : m_vQueue)
    {
        if (!job->IsWorker(userID) || job->GetStatus() == JobRequest::status::complete)
            continue;
           
        return job;
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::size_t JobQueue::GetQueueSize(const bool bShowComplete, JobCompare compare) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    return std::count_if(
        m_vQueue.begin(),
        m_vQueue.end(),
        [bShowComplete, compare](const auto& job) {
            bool bRet = compare(job);
            if (!bShowComplete)
            {
                bRet &= job->GetStatus() != JobRequest::status::complete;
            }
            return bRet;
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