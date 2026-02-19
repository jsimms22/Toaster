#include "JobQueue.h"

#include "BotUtility.h"

#include "CraftingJobRequest.h"
#include "BuildingJobRequest.h"
#include "ComponentJobRequest.h"
#include "ResourceJobRequest.h"

#include "tinyxml2.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <optional>
#include <sstream>
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
            std::shared_ptr<JobRequest> job;
            switch (std::stoul(type))
            {
                case JOB_TYPE_CRAFTING:
                {
                    job = std::make_shared<CraftingJobRequest>();
                }
                break;
                case JOB_TYPE_BUILDING:
                {
                    job = std::make_shared<BuildingJobRequest>();
                }
                break;
                case JOB_TYPE_COMPONENT:
                {
                    job = std::make_shared<ComponentJobRequest>();
                }
                break;
                case JOB_TYPE_RESOURCE:
                {
                    job = std::make_shared<ResourceJobRequest>();
                }
                break;
                case JOB_TYPE_REFINERY:
                {
                    job = std::make_shared<JobRequest>();
                }
                break;
                default:
                {
                    job = std::make_shared<JobRequest>();
                }
            }

            job->ReadAttributes(xmlNode, root);
            m_vQueue.emplace_back(std::move(job));
        }
    }
}

std::shared_ptr<JobRequest> JobQueue::GetJobByGUID(const std::string& searchGUID)
{
    for (const std::shared_ptr<JobRequest>& job : m_vQueue)
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
std::string JobQueue::PrintQueue() const
{
    std::stringstream ss;
    for (const auto& job : m_vQueue)
    {
        ss << "Request (**" << JobRequest::PriorityToString(job->GetPriority()) << "**): "
           << utils::GuidToStringNoBrackets(job->GetID())
           << "\n**Status**: " << JobRequest::StatusToString(job->GetStatus()) 
           << "\n**Assigned**: " << job->GetWorker()
           << "\n**Type**: " << job->JobTypeToString() << "\n\n";
    }

    return ss.str();
}

std::string JobQueue::PrintQueueByUser(const std::string username) const
{
    std::stringstream ss;
    for (const auto& job : m_vQueue)
    {
        if (job->GetAuthor() == username)
        {
            ss << job->PrintJobDetails() << '\n';
        }
    }

    return ss.str();
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
