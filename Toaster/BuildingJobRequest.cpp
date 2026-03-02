#include "BuildingJobRequest.h"
// fmt
#include <fmt/format.h>

namespace xmlRequest
{
    constexpr const char* pszXMLBuildDesignation{ "Designation" };
    constexpr const char* pszXMLBuildRequirements{ "Requirements" };
    constexpr const char* pszXMLBuildZone{ "Zone" };
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void BuildingJobRequest::WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent) const
{
    if (!xmlNode || !xmlParent)
    {
        return;
    }

    JobRequest::WriteAttributes(xmlNode, xmlParent);

    xmlNode->SetAttribute(xmlRequest::pszXMLBuildDesignation, m_strBldgDesignation.c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLBuildRequirements, m_strBldgRequires.c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLBuildZone, m_strBldgZone.c_str());
    xmlParent->InsertEndChild(xmlNode);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void BuildingJobRequest::ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    if (!xmlNode)
    {
        return;
    }

    JobRequest::ReadAttributes(xmlNode, xmlParent);

    const char* pszDesignation = xmlNode->Attribute(xmlRequest::pszXMLBuildDesignation);
    if (pszDesignation)
    {
        m_strBldgDesignation = pszDesignation;
    }

    const char* pszRequires = xmlNode->Attribute(xmlRequest::pszXMLBuildRequirements);
    if (pszRequires)
    {
        m_strBldgRequires = pszRequires;
    }

    const char* pszBuildZone = xmlNode->Attribute(xmlRequest::pszXMLBuildZone);
    if (pszBuildZone)
    {
        m_strBldgZone = pszBuildZone;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::string BuildingJobRequest::PrintJobDetails(dpp::cluster& cluster, const dpp::snowflake& idGuild) const
{
    std::string base = JobRequest::PrintJobDetails(cluster, idGuild);

    return fmt::format(
        "{}"
        "**Building Designation**: {}\n"
        "**Building Requirements**: \n{}\n"
        "**Building Zone**: {}\n",
        base,
        m_strBldgDesignation,
        m_strBldgRequires,
        m_strBldgZone);
}