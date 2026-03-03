#include "HazardousRequest.h"
// fmt
#include <fmt/format.h>

namespace xmlRequest
{
    constexpr const char* pszXMLHazItemList{ "HazItemList" };
    constexpr const char* pszXMLHazItemLoc{ "HazItemLoc" };
    constexpr const char* pszXMLThreat{ "ThreatLeavel" };
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::string HazardousRequest::ThreatToString(const HazardousRequest::ThreatLevel t)
{
    switch (t) {
    case HazardousRequest::ThreatLevel::Permissive:  return "permissive";
    case HazardousRequest::ThreatLevel::Minimal:     return "minimal";
    case HazardousRequest::ThreatLevel::Uncertain:   return "uncertain";
    case HazardousRequest::ThreatLevel::Hostile:     return "hostile";
    default: throw std::invalid_argument("Invalid threat enum value");
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
HazardousRequest::ThreatLevel HazardousRequest::StringToThreat(const std::string& str)
{
    if (str == "permissive")    return HazardousRequest::ThreatLevel::Permissive;
    if (str == "minimal")       return HazardousRequest::ThreatLevel::Minimal;
    if (str == "uncertain")     return HazardousRequest::ThreatLevel::Uncertain;
    if (str == "hostile")       return HazardousRequest::ThreatLevel::Hostile;
    throw std::invalid_argument("Invalid threat string value");
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void HazardousRequest::WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent, tinyxml2::XMLDocument* doc) const
{
    if (!xmlNode || !xmlParent || !doc)
    {
        return;
    }

    JobRequest::WriteAttributes(xmlNode, xmlParent, doc);

    xmlNode->SetAttribute(xmlRequest::pszXMLHazItemList, m_strHazItemList.c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLHazItemLoc, m_strHazItemZone.c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLThreat, static_cast<int>(m_threat));
    xmlParent->InsertEndChild(xmlNode);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void HazardousRequest::ReadAttributes(tinyxml2::XMLElement* xmlNode)
{
    if (!xmlNode)
    {
        return;
    }

    JobRequest::ReadAttributes(xmlNode);

    const char* pszItemList = xmlNode->Attribute(xmlRequest::pszXMLHazItemList);
    if (pszItemList)
    {
        m_strHazItemList = pszItemList;
    }

    const char* pszItemZone = xmlNode->Attribute(xmlRequest::pszXMLHazItemLoc);
    if (pszItemZone)
    {
        m_strHazItemZone = pszItemZone;
    }

    m_threat = static_cast<ThreatLevel>(xmlNode->IntAttribute(xmlRequest::pszXMLThreat, ThreatLevel::Uncertain));
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::string HazardousRequest::PrintJobDetails(dpp::cluster& cluster, const dpp::snowflake& idGuild) const
{
    std::string base = JobRequest::PrintJobDetails(cluster, idGuild);

    return fmt::format(
        "{}"
        "**Threat Level**: {}\n"
        "**Retrieval Zone**: {}\n"
        "**Hazardous Item List**: \n{}\n",
        base,
        ThreatToString(m_threat),
        m_strHazItemZone,
        m_strHazItemList);
}