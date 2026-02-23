#include "JobRequest.h"

#include "BotUtility.h"
// d++
#include <dpp/cache.h>
#include <dpp/user.h>
#include <dpp/unicode_emoji.h>
// fmt
#include <fmt/format.h>
// std library
#include <future>
#include <iostream>
#include <stdexcept>
#include <string>

namespace xmlRequest
{
    constexpr const char* pszXMLCreation{ "Created" };
    constexpr const char* pszXMLLastEdit{ "LastEdit" };
    constexpr const char* pszXMLRequestUser{ "Requester" };
    constexpr const char* pszXMLJobWorker{ "Worker" };
    constexpr const char* pszXMLRequestSCHandle{ "GameHandle" };
    constexpr const char* pszXMLJobPriority{ "Priority" };
    constexpr const char* pszXMLJobStatus{ "Status" };
    constexpr const char* pszXMLJobGUID{ "GUID" };
    constexpr const char* pszXMLJobType{ "Type" };
}

// Function to convert priority enum to string
std::string JobRequest::PriorityToString(JobRequest::priority p)
{
    switch (p) {
    case JobRequest::priority::low:         return "low";
    case JobRequest::priority::medium:      return "medium";
    case JobRequest::priority::high:        return "high";
    case JobRequest::priority::critical:    return "critical";
    default: throw std::invalid_argument("Unexpected priority enum value");
    }
}

// Function to convert string to priority enum
JobRequest::priority JobRequest::StringToPriority(const std::string& str)
{
    if (str == "low")       return JobRequest::priority::low;
    if (str == "medium")    return JobRequest::priority::medium;
    if (str == "high")      return JobRequest::priority::high;
    if (str == "critical")  return JobRequest::priority::critical;
    throw std::invalid_argument("Unexpected priority string value");
}

const char* JobRequest::PriorityToEmoji(JobRequest::priority p)
{
    switch (p) {
    case JobRequest::priority::low:         return dpp::unicode_emoji::green_circle;
    case JobRequest::priority::medium:      return dpp::unicode_emoji::yellow_circle;
    case JobRequest::priority::high:        return dpp::unicode_emoji::orange_circle;
    case JobRequest::priority::critical:    return dpp::unicode_emoji::red_circle;
    default:                                return dpp::unicode_emoji::black_circle;
    }
}

// Function to convert status enum to string
std::string JobRequest::StatusToString(JobRequest::status s)
{
    switch (s) {
    case JobRequest::status::open:      return "open";
    case JobRequest::status::stalled:   return "stalled";
    case JobRequest::status::assigned:  return "assigned";
    case JobRequest::status::active:    return "active";
    case JobRequest::status::hold:      return "on hold";
    case JobRequest::status::complete:  return "complete";
    default: throw std::invalid_argument("Unexpected status enum value");
    }
}

// Function to convert string to status enum
JobRequest::status JobRequest::StringToStatus(const std::string& str)
{
    if (str == "open")      return JobRequest::status::open;
    if (str == "stalled")   return JobRequest::status::stalled;
    if (str == "assigned")  return JobRequest::status::assigned;
    if (str == "active")    return JobRequest::status::active;
    if (str == "on hold")   return JobRequest::status::hold;
    if (str == "complete")  return JobRequest::status::complete;
    throw std::invalid_argument("Unexpected status string value");
}

const char* JobRequest::StatusToEmoji(JobRequest::status s)
{
    switch (s) {
    case JobRequest::status::open:      return dpp::unicode_emoji::white_circle;
    case JobRequest::status::stalled:   return dpp::unicode_emoji::orange_circle;
    case JobRequest::status::assigned:  return dpp::unicode_emoji::yellow_circle;
    case JobRequest::status::active:    return dpp::unicode_emoji::green_circle;
    case JobRequest::status::hold:      return dpp::unicode_emoji::red_circle;
    case JobRequest::status::complete:  return dpp::unicode_emoji::blue_circle;
    default:                            return dpp::unicode_emoji::black_circle;
    }
}

JobRequest::JobRequest() 
    : m_id(utils::CreateGUID()) 
{
    m_timeCreated = utils::GetEpochTimestamp();
    m_timeLastEdit = m_timeCreated;
}

void JobRequest::WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    // Check if the xmlNode is valid.
    if (xmlNode == nullptr)
    {
        return;
    }

    // Create a new <Request> element for this submission
    xmlNode->SetAttribute(xmlRequest::pszXMLCreation, m_timeCreated);
    xmlNode->SetAttribute(xmlRequest::pszXMLLastEdit, m_timeLastEdit);
    xmlNode->SetAttribute(xmlRequest::pszXMLJobGUID, utils::GuidToString(m_id).c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLJobPriority, m_eJobPriority);
    xmlNode->SetAttribute(xmlRequest::pszXMLJobStatus, m_eJobStatus);
    xmlNode->SetAttribute(xmlRequest::pszXMLJobType, JobType());
    xmlNode->SetAttribute(xmlRequest::pszXMLJobWorker, m_idWorker);
    xmlNode->SetAttribute(xmlRequest::pszXMLRequestUser, m_idCustomer);
    xmlNode->SetAttribute(xmlRequest::pszXMLRequestSCHandle, m_strSCHandle.c_str());
    xmlParent->InsertEndChild(xmlNode);
}

void JobRequest::ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    if (xmlNode == nullptr)
    {
        return;
    }

    const char* created = xmlNode->Attribute(xmlRequest::pszXMLCreation);
    if (created)
    {
        m_timeCreated = std::stoull(created);
    }

    const char* lastEdit = xmlNode->Attribute(xmlRequest::pszXMLLastEdit);
    if (lastEdit)
    {
        m_timeLastEdit = std::stoull(lastEdit);
    }

    const char* jobGUID = xmlNode->Attribute(xmlRequest::pszXMLJobGUID);
    if (jobGUID)
    {
        m_id = utils::StringToGuid(jobGUID);
    }

    const char* customerID = xmlNode->Attribute(xmlRequest::pszXMLRequestUser);
    if (customerID)
    {
        m_idCustomer = customerID;
    }

    const char* workerID = xmlNode->Attribute(xmlRequest::pszXMLJobWorker);
    if (workerID)
    {
        m_idWorker = workerID;
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
}

std::string JobRequest::PrintJobDetails(dpp::cluster& cluster) const
{
    fmt::memory_buffer buffer;

    return fmt::format(
        "**ID**: {}\n"
        "**Customer**: {}\n"
        "**Type**: {}\n"
        "**Status**: {}\n"
        "**Priority**: {}\n"
        "**Created**: <t:{}:F>\n"
        "**Last Edit**: <t:{}:F>\n",
        utils::GuidToStringNoBrackets(m_id),
        GetCustomerName(cluster),
        JobTypeToString(),
        StatusToString(m_eJobStatus),
        PriorityToString(m_eJobPriority),
        m_timeCreated,
        m_timeLastEdit);
}

const std::string JobRequest::GetCustomerName(dpp::cluster& cluster) const
{
    return utils::FindUserByID(cluster, m_idCustomer).username;
}

const std::string JobRequest::GetWorkerName(dpp::cluster& cluster) const
{
    return utils::FindUserByID(cluster, m_idWorker).username;
}