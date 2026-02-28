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

const std::size_t JobQueue::JOBS_PER_QUEUE_PAGE{ 8 };
const std::size_t JobQueue::JOBS_PER_DETAIL_PAGE{ 4 };

//---------------------------------------------------------------------------------------------------------------------
// \brief Constructor: Reads the XML and loads the job requests into the container
//---------------------------------------------------------------------------------------------------------------------
JobQueue::JobQueue()
{
    m_worker = std::jthread(&JobQueue::MutationWorker, this);

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
        const char* type = xmlNode->Attribute("Type");
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

//---------------------------------------------------------------------------------------------------------------------
// \brief Destructor. Frees the worker thread of its task and saves the queue as is to persistent storage.
//---------------------------------------------------------------------------------------------------------------------
JobQueue::~JobQueue()
{
    {
        std::lock_guard<std::mutex> lock(m_mtxMutQueue);
        m_bRunning = false;
    }

    m_cv.notify_all();

    SaveQueueToFile();
}

void JobQueue::EnqueueMutation(QueueMutation mutation)
{
    {
        std::lock_guard<std::mutex> lock(m_mtxMutQueue);
        m_mutations.emplace(std::move(mutation));
    }

    m_cv.notify_one();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
template<typename T>
std::future<T> JobQueue::EnqueueTaskWithResult(std::function<T(std::shared_ptr<JobQueue>)> task)
{
    auto promise = std::make_shared<std::promise<T>>();
    auto future = promise->get_future();

    EnqueueMutation(
        [task = std::move(task), promise](std::shared_ptr<JobQueue> queue)
        {
            promise->set_value(task(queue));
        }
    );

    return future;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void JobQueue::MutationWorker()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(m_mtxMutQueue);

        m_cv.wait(lock, [this] {
            return !m_mutations.empty() || !m_bRunning;
            });

        if (!m_bRunning && m_mutations.empty())
            break;

        auto mutation = std::move(m_mutations.front());
        m_mutations.pop();
        lock.unlock();

        mutation(shared_from_this());   // Worker owns execution context
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void JobQueue::RequestAddToQueue(std::shared_ptr<JobRequest> job)
{
    EnqueueMutation(
        [job = std::move(job)](std::shared_ptr<JobQueue> queue) mutable
        {
            std::unique_lock<std::shared_mutex> lock(queue->m_mtxShared);
            queue->m_vQueue.emplace_back(std::move(job));
            queue->SaveQueueToFile();
        }
    );
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
std::future<bool> JobQueue::RequestDeleteJobByGUID(const GUID& guid)
{
    return EnqueueTaskWithResult<bool>(
        [guid](std::shared_ptr<JobQueue> queue)
        {
            std::unique_lock<std::shared_mutex> lock(queue->m_mtxShared);

            for (auto itr = queue->m_vQueue.begin();
                itr != queue->m_vQueue.end();
                ++itr)
            {
                if ((*itr)->GetID() == guid)
                {
                    itr->reset();
                    queue->m_vQueue.erase(itr);
                    queue->SaveQueueToFile();
                    return true;
                }
            }

            return false;
        }
    );
}


//---------------------------------------------------------------------------------------------------------------------
// \brief 
/* Example usage:
    jobQueue.ModifyJob(guid, [](JobRequest& job)
    {
        job.SetWorkerID(12345);
        job.SetStatus(JobRequest::status::in_progress);
    });
*/
//---------------------------------------------------------------------------------------------------------------------
void JobQueue::RequestModifyJob(const GUID& guid, JobMutation mutator)
{
    EnqueueMutation(
        [guid, mutator = std::move(mutator)](std::shared_ptr<JobQueue> queue) mutable
        {
            std::unique_lock<std::shared_mutex> lock(queue->m_mtxShared);
            auto job = queue->GetJobByGUID_NoLock(guid);
            if (!job) { return; }
            mutator(job);
            queue->SaveQueueToFile();
        }
    );
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
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
std::shared_ptr<JobRequest> JobQueue::GetJobByGUID(const GUID& guid) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);
    return GetJobByGUID_NoLock(guid);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<JobRequest> JobQueue::GetJobByGUID(const std::string& guid) const
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
            "**Assigned**: {}\n"
            "**Type**: {}\n\n",
            position,
            JobRequest::PriorityToString(job->GetPriority()),
            utils::GuidToStringNoBrackets(job->GetID()),
            JobRequest::StatusToString(job->GetStatus()),
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
const std::string JobQueue::PrintQueueByStatus(dpp::cluster& cluster, const JobRequest::status filter, const dpp::snowflake& idGuild) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    fmt::memory_buffer buffer;
    std::size_t position = 0;

    for (const auto& job : m_vQueue)
    {
        if (job->GetStatus() == JobRequest::status::complete)
            continue;

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
const std::string JobQueue::PrintQueuePageByStatus(
    dpp::cluster& cluster, 
    const JobRequest::status filter, 
    const std::size_t page,
    const dpp::snowflake& idGuild) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    fmt::memory_buffer buffer;

    const std::size_t start_index = page * JOBS_PER_DETAIL_PAGE;
    const std::size_t end_index = start_index + JOBS_PER_DETAIL_PAGE;

    std::size_t filtered_index = 0;
    std::size_t display_pos = 0;
    std::size_t position = 0;

    for (const auto& job : m_vQueue)
    {
        if (job->GetStatus() == JobRequest::status::complete)
            continue;
        
        ++position;

        if (job->GetStatus() != filter)
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
                position,
                job->PrintJobDetails(cluster, idGuild)
            );
        }

        ++filtered_index;
    }

    return fmt::to_string(buffer);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief Method to print all job requests
//---------------------------------------------------------------------------------------------------------------------
const std::string JobQueue::PrintQueueByType(dpp::cluster& cluster, const std::size_t filter, const dpp::snowflake& idGuild) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    fmt::memory_buffer buffer;
    std::size_t position = 0;

    for (const auto& job : m_vQueue)
    {
        if (job->GetStatus() == JobRequest::status::complete)
            continue;

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
const std::string JobQueue::PrintQueuePageByType(
    dpp::cluster& cluster, 
    const std::size_t filter, 
    const std::size_t page, 
    const dpp::snowflake& idGuild) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    fmt::memory_buffer buffer;

    const std::size_t start_index = page * JOBS_PER_QUEUE_PAGE;
    const std::size_t end_index = start_index + JOBS_PER_QUEUE_PAGE;

    std::size_t filtered_index = 0;
    std::size_t display_pos = 0;

    for (const auto& job : m_vQueue)
    {
        if (job->GetStatus() == JobRequest::status::complete)
            continue;

        if (!job->SupportsType(filter))
            continue;

        // Only count filtered jobs
        if (filtered_index >= end_index)
            break;

        if (filtered_index >= start_index)
        {
            ++display_pos;

            fmt::format_to(
                std::back_inserter(buffer),
                "***Position: {}***\n"
                "ID (**{}**): {}\n"
                "**Status**: {}\n"
                "**Assigned**: {}\n"
                "**Type**: {}\n\n",
                start_index + display_pos,
                JobRequest::PriorityToString(job->GetPriority()),
                utils::GuidToStringNoBrackets(job->GetID()),
                JobRequest::StatusToString(job->GetStatus()),
                job->GetWorkerName(cluster, idGuild),
                job->JobTypeToString()
            );
        }

        ++filtered_index;
    }

    return fmt::to_string(buffer);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::vector<std::shared_ptr<JobRequest>> JobQueue::GetQueueByUser(const dpp::snowflake& userID,
                                                                        const std::size_t filter) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    std::vector<std::shared_ptr<JobRequest>> list;
    for (const auto& job : m_vQueue)
    {
        if (job->GetCustomerID() != userID || !job->SupportsType(filter))
            continue;

        list.push_back(job);
    }

    return list;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::string JobQueue::PrintQueueByUser(dpp::cluster& cluster, 
    const dpp::snowflake& userID, 
    const std::size_t filter, const dpp::snowflake& idGuild) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    fmt::memory_buffer buffer;
    std::size_t position = 0;

    for (const auto& job : m_vQueue)
    {
        if (job->GetStatus() == JobRequest::status::complete)
            continue;

        ++position;

        if (job->GetCustomerID() != userID || !job->SupportsType(filter))
            continue;

        fmt::format_to(
            std::back_inserter(buffer),
            "***Position: {}***\n{}\n",
            position,
            job->PrintJobDetails(cluster, idGuild)
        );
    }

    // Convert the buffer to a std::string once at the end
    return fmt::to_string(buffer);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::string JobQueue::PrintQueuePageByUser(dpp::cluster& cluster, 
                                                 const dpp::snowflake& userID,
                                                 const std::size_t filter,
                                                 const std::size_t page, const dpp::snowflake& idGuild) const
{
    const auto list = GetQueueByUser(userID, filter);
    if (list.empty()) return {};

    std::shared_lock<std::shared_mutex> lock(m_mtxShared);
    const std::size_t start_index = page * JOBS_PER_DETAIL_PAGE;
    const std::size_t end_index = start_index + JOBS_PER_DETAIL_PAGE;

    std::size_t filtered_index = 0;
    std::size_t display_pos = 0;

    std::stringstream ss;
    for (const auto& job : list)
    {
        if (job->GetCustomerID() != userID || !job->SupportsType(filter))
            continue;

        // Only count filtered jobs
        if (filtered_index >= end_index)
            break;

        if (filtered_index >= start_index)
        {
            ++display_pos;

            ss << job->PrintJobDetails(cluster, idGuild) << '\n';
        }

        ++filtered_index;
    }

    return ss.str();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::vector<std::shared_ptr<JobRequest>> JobQueue::GetQueueByWorker(const dpp::snowflake& userID) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    std::vector<std::shared_ptr<JobRequest>> list;
    for (const auto& job : m_vQueue)
    {
        if (job->GetWorkerID() != userID || job->GetStatus() == JobRequest::status::complete)
            continue;

        list.push_back(job);
    }

    return list;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::string JobQueue::PrintQueueByWorker(dpp::cluster& cluster, const dpp::snowflake& userID, const dpp::snowflake& idGuild) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);
    
    std::stringstream ss;
    for (const auto& job : m_vQueue)
    {
        if (job->GetWorkerID() != userID || job->GetStatus() == JobRequest::status::complete)
            continue;
        
        ss << job->PrintJobDetails(cluster, idGuild) << '\n';
        
    }

    return ss.str();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::string JobQueue::PrintQueuePageByWorker(dpp::cluster& cluster, const dpp::snowflake& userID, const std::size_t page, const dpp::snowflake& idGuild) const
{
    const auto list = GetQueueByWorker(userID);
    if (list.empty()) return {};

    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    const std::size_t start_index = page * JOBS_PER_DETAIL_PAGE;
    const std::size_t end_index = start_index + JOBS_PER_DETAIL_PAGE;

    std::size_t filtered_index = 0;
    std::size_t display_pos = 0;

    std::stringstream ss;
    for (const auto& job : list)
    {
        if (job->GetWorkerID() != userID || job->GetStatus() == JobRequest::status::complete)
            continue;

        // Only count filtered jobs
        if (filtered_index >= end_index)
            break;

        if (filtered_index >= start_index)
        {
            ++display_pos;

            ss << job->PrintJobDetails(cluster, idGuild) << '\n';
        }

        ++filtered_index;
    }

    return ss.str();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::string JobQueue::PrintFirstAssignment(dpp::cluster& cluster, const dpp::snowflake& userID, const dpp::snowflake& idGuild) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    for (const auto& job : m_vQueue)
    {
        if (job->GetWorkerID() != userID || job->GetStatus() == JobRequest::status::complete)
            continue;
            
        return job->PrintJobDetails(cluster, idGuild);
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<JobRequest> JobQueue::FirstAssignment(const dpp::snowflake& userID)
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
bool JobQueue::IsInQueue(const std::string& strID) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    const auto& itr = std::find_if(m_vQueue.cbegin(), m_vQueue.cend(), [&strID](const auto& job) -> 
                            bool { return utils::GuidToStringNoBrackets(job->GetID()) == strID; });
    return (itr != m_vQueue.cend());

}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::size_t JobQueue::GetFilteredQueueSizeByType(const std::size_t filter) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    return std::count_if(
        m_vQueue.begin(),
        m_vQueue.end(),
        [&filter](const auto& job) {
            return (job->SupportsType(filter) &&
                job->GetStatus() != JobRequest::status::complete);
        }
    );
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::size_t JobQueue::GetFilteredQueueSizeByStatus(const JobRequest::status filter) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    return std::count_if(
        m_vQueue.begin(),
        m_vQueue.end(),
        [&filter](const auto& job) {
            return job->GetStatus() == filter;
        }
    );

}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::size_t JobQueue::GetFilteredQueueSizeByWorker(const dpp::snowflake& worker) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    return std::count_if(
        m_vQueue.begin(),
        m_vQueue.end(),
        [&worker](const auto& job) {
            return (job->GetWorkerID() == worker &&
                job->GetStatus() != JobRequest::status::complete);
        }
    );
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const std::size_t JobQueue::GetFilteredQueueSizeByUser(const dpp::snowflake& user, const std::size_t filter) const
{
    std::shared_lock<std::shared_mutex> lock(m_mtxShared);

    return std::count_if(
        m_vQueue.begin(),
        m_vQueue.end(),
        [&user, &filter](const auto& job) {
            return (job->GetCustomerID() == user && job->SupportsType(filter));
        }
    );
}