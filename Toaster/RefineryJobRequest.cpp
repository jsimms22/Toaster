#include "RefineryJobRequest.h"
// fmt
#include <fmt/format.h>

namespace xmlRequest
{
    constexpr const char* pszXMLRefineState{ "RefineType" };
    constexpr const char* pszXMLRefineList{ "RefineList" };
    constexpr const char* pszXMLRefineryLoc{ "Refinery" };
}

// Function to convert priority enum to string
std::string RefineryJobRequest::StateToString(RefineryJobRequest::state e)
{
    switch (e) {
    case RefineryJobRequest::state::RefinedMineable:    return "refined_mineable";
    case RefineryJobRequest::state::RefinedSalvage:     return "refined_salvage";
    case RefineryJobRequest::state::RefinedHarvest:     return "refined_harvest";
    default: throw std::invalid_argument("Invalid priority enum value");
    }
}

// Function to convert string to status enum
RefineryJobRequest::state RefineryJobRequest::StringToState(const std::string& str)
{
    if (str == "refined_mineable")  return RefineryJobRequest::state::RefinedMineable;
    if (str == "refined_salvage")   return RefineryJobRequest::state::RefinedSalvage;
    if (str == "refined_harvest")   return RefineryJobRequest::state::RefinedHarvest;
    throw std::invalid_argument("Invalid status string value");
}

void RefineryJobRequest::WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    if (xmlNode == nullptr)
    {
        return;
    }

    JobRequest::WriteAttributes(xmlNode, xmlParent);

    xmlNode->SetAttribute(xmlRequest::pszXMLRefineState, RefineryJobRequest::StateToString(m_eResourceState).c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLRefineList, m_strResourceList.c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLRefineryLoc, m_strRefinery.c_str());
    xmlParent->InsertEndChild(xmlNode);
}

void RefineryJobRequest::ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    if (xmlNode == nullptr)
    {
        return;
    }

    JobRequest::ReadAttributes(xmlNode, xmlParent);

    const char* pszState = xmlNode->Attribute(xmlRequest::pszXMLRefineState);
    if (pszState)
    {
        m_eResourceState = RefineryJobRequest::StringToState(pszState);
    }

    const char* pszResourceList = xmlNode->Attribute(xmlRequest::pszXMLRefineList);
    if (pszResourceList)
    {
        m_strResourceList = pszResourceList;
    }

    const char* pszRefineryloc = xmlNode->Attribute(xmlRequest::pszXMLRefineryLoc);
    if (pszRefineryloc)
    {
        m_strRefinery = pszRefineryloc;
    }
}

std::string RefineryJobRequest::PrintJobDetails(dpp::cluster* cluster) const
{
    if (!cluster)
        return {};

    std::string base = JobRequest::PrintJobDetails(cluster);

    return fmt::format(
        "{}"
        "**Resource Type**: {}\n"
        "**Resource List**:\n{}\n"
        "**Refinery Site**: {}\n",
        base,
        RefineryJobRequest::StateToString(m_eResourceState),
        m_strResourceList,
        m_strRefinery);
}