#include "JobRequest.h"
#include "BotUtility.h"

#include <dpp/dpp.h>

#include <string>
#include <iostream>
#include <stdexcept>

namespace xmlRequest
{
    const char* pszXMLRequestUser{ "Requester" };
    const char* pszXMLJobWorker{ "Worker" };
    const char* pszXMLRequestSCHandle{ "GameHandle" };
    const char* pszXMLJobPriority{ "Priority" };
    const char* pszXMLJobStatus{ "Status" };
    const char* pszXMLJobGUID{ "GUID" };
    const char* pszXMLJobType{ "Type" };
}

// Function to convert status enum to string
std::string JobRequest::StatusToString(JobRequest::status s) {
    switch (s) {
    case JobRequest::status::open:     return "open";
    case JobRequest::status::stalled:  return "stalled";
    case JobRequest::status::active:   return "active";
    case JobRequest::status::hold:     return "hold";
    case JobRequest::status::complete: return "complete";
    default: throw std::invalid_argument("Invalid status enum value");
    }
}

// Function to convert priority enum to string
std::string JobRequest::PriorityToString(JobRequest::priority p) {
    switch (p) {
    case JobRequest::priority::low:      return "low";
    case JobRequest::priority::med:      return "med";
    case JobRequest::priority::high:     return "high";
    case JobRequest::priority::critical: return "critical";
    default: throw std::invalid_argument("Invalid priority enum value");
    }
}

// Function to convert string to status enum
JobRequest::status JobRequest::StringToStatus(const std::string& str) {
    if (str == "open")     return JobRequest::status::open;
    if (str == "stalled")  return JobRequest::status::stalled;
    if (str == "active")   return JobRequest::status::active;
    if (str == "hold")     return JobRequest::status::hold;
    if (str == "complete") return JobRequest::status::complete;
    throw std::invalid_argument("Invalid status string value");
}

// Function to convert string to priority enum
JobRequest::priority JobRequest::StringToPriority(const std::string& str) {
    if (str == "low")      return JobRequest::priority::low;
    if (str == "med")      return JobRequest::priority::med;
    if (str == "high")     return JobRequest::priority::high;
    if (str == "critical") return JobRequest::priority::critical;
    throw std::invalid_argument("Invalid priority string value");
}

JobRequest::JobRequest() : m_id(utils::CreateGUID()) {}

void JobRequest::WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    // Check if the xmlNode is valid.
    if (xmlNode == nullptr)
    {
        return;
    }

    // Create a new <Request> element for this submission
    xmlNode->SetAttribute(xmlRequest::pszXMLJobGUID, utils::GuidToString(m_id).c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLJobPriority, m_eJobPriority);
    xmlNode->SetAttribute(xmlRequest::pszXMLJobStatus, m_eJobStatus);
    xmlNode->SetAttribute(xmlRequest::pszXMLJobType, JobType());
    xmlNode->SetAttribute(xmlRequest::pszXMLJobWorker, m_userWorker.c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLRequestUser, m_userAuthor.c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLRequestSCHandle, m_strSCHandle.c_str());
    xmlParent->InsertEndChild(xmlNode);
}

void JobRequest::ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    // Check if the xmlNode is valid.
    if (xmlNode == nullptr)
    {
        return;
    }

    // Read the attributes and set the member variables
    const char* jobGUID = xmlNode->Attribute(xmlRequest::pszXMLJobGUID);
    if (jobGUID)
    {
        m_id = utils::StringToGuid(jobGUID);  // Assuming you have a utility function to convert GUID string to an appropriate type
    }

    const char* requestUser = xmlNode->Attribute(xmlRequest::pszXMLRequestUser);
    if (requestUser)
    {
        m_userAuthor = requestUser;
    }

    const char* requestSCHandle = xmlNode->Attribute(xmlRequest::pszXMLRequestSCHandle);
    if (requestSCHandle)
    {
        m_strSCHandle = requestSCHandle;
    }

    const char* jobPriority = xmlNode->Attribute(xmlRequest::pszXMLJobPriority);
    if (jobPriority)
    {
        m_eJobPriority = static_cast<JobRequest::priority>(std::stoi(jobPriority));
    }

    const char* jobStatus = xmlNode->Attribute(xmlRequest::pszXMLJobStatus);
    if (jobStatus)
    {
        m_eJobStatus = static_cast<JobRequest::status>(std::stoi(jobStatus));
    }

    const char* jobWorker = xmlNode->Attribute(xmlRequest::pszXMLJobWorker);
    if (jobWorker)
    {
        m_userWorker = jobWorker;
    }
}

std::string JobRequest::PrintJobDetails() const
{
    std::stringstream ss;
    ss << "ID: " << utils::GuidToStringNoBrackets(m_id) << std::endl;
    ss << "Type: " << JobTypeToString() << std::endl;
    ss << "Status: " << StatusToString(m_eJobStatus) << std::endl;
    ss << "priority: " << PriorityToString(m_eJobPriority) << std::endl;
    return ss.str();
}