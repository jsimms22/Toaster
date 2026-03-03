#include "RefineryJobRequest.h"
// fmt
#include <fmt/format.h>

namespace xmlRequest
{
    constexpr const char* pszXMLRefineState{ "RefineType" };
    constexpr const char* pszXMLRefineList{ "RefineList" };
    constexpr const char* pszXMLRefineryLoc{ "Refinery" };
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::string RefineryJobRequest::StateToString(const RefineryJobRequest::state e)
{
    switch (e) {
    case RefineryJobRequest::state::RefinedMineable:    return "refined_mineable";
    case RefineryJobRequest::state::RefinedSalvage:     return "refined_salvage";
    case RefineryJobRequest::state::RefinedHarvest:     return "refined_harvest";
    default: throw std::invalid_argument("Invalid priority enum value");
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
RefineryJobRequest::state RefineryJobRequest::StringToState(const std::string& str)
{
    if (str == "refined_mineable")  return RefineryJobRequest::state::RefinedMineable;
    if (str == "refined_salvage")   return RefineryJobRequest::state::RefinedSalvage;
    if (str == "refined_harvest")   return RefineryJobRequest::state::RefinedHarvest;
    throw std::invalid_argument("Invalid status string value");
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void RefineryJobRequest::WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent, tinyxml2::XMLDocument* doc) const
{
    if (!xmlNode || !xmlParent || !doc)
    {
        return;
    }

    JobRequest::WriteAttributes(xmlNode, xmlParent, doc);

    xmlNode->SetAttribute(xmlRequest::pszXMLRefineState, static_cast<int>(m_eResourceState));
    xmlNode->SetAttribute(xmlRequest::pszXMLRefineList, m_strResourceList.c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLRefineryLoc, m_strRefinery.c_str());
    xmlParent->InsertEndChild(xmlNode);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void RefineryJobRequest::ReadAttributes(tinyxml2::XMLElement* xmlNode)
{
    if (!xmlNode)
    {
        return;
    }

    JobRequest::ReadAttributes(xmlNode);

    m_eResourceState = static_cast<state>(xmlNode->IntAttribute(xmlRequest::pszXMLRefineState, state::RefinedMineable));


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

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::string RefineryJobRequest::PrintJobDetails(dpp::cluster& cluster, const dpp::snowflake& idGuild) const
{
    std::string base = JobRequest::PrintJobDetails(cluster, idGuild);

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