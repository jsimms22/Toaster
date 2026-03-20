#include "HazardousRequest.h"
// fmt
#include <fmt/format.h>
// bsoncxx
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp> 
#include <bsoncxx/document/element.hpp>
#include <bsoncxx/types.hpp>

namespace serial
{
    constexpr const char* pszHazItemList{ "hazmat_list" };
    constexpr const char* pszHazItemLoc{ "hazmat_location" };
    constexpr const char* pszThreat{ "hazmat_threat" };
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

    xmlNode->SetAttribute(serial::pszHazItemList, m_strHazItemList.c_str());
    xmlNode->SetAttribute(serial::pszHazItemLoc, m_strHazItemZone.c_str());
    xmlNode->SetAttribute(serial::pszThreat, static_cast<int>(m_threat));
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

    const char* pszItemList = xmlNode->Attribute(serial::pszHazItemList);
    if (pszItemList)
    {
        m_strHazItemList = pszItemList;
    }

    const char* pszItemZone = xmlNode->Attribute(serial::pszHazItemLoc);
    if (pszItemZone)
    {
        m_strHazItemZone = pszItemZone;
    }

    m_threat = static_cast<ThreatLevel>(xmlNode->IntAttribute(serial::pszThreat, ThreatLevel::Uncertain));
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
bsoncxx::builder::basic::document HazardousRequest::WriteAttributesBSON() const
{
    using namespace bsoncxx::builder::basic;
    auto doc = JobRequest::WriteAttributesBSON(); // base class attributes

    // add derived attributes
    doc.append(
        kvp(std::string{ serial::pszHazItemList }, m_strHazItemList),
        kvp(std::string{ serial::pszHazItemLoc }, m_strHazItemZone),
        kvp(std::string{ serial::pszThreat }, m_threat)
    );

    return doc;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void HazardousRequest::ReadAttributesBSON(const bsoncxx::document::view& doc)
{
    // Let parent read base fields AND notes
    JobRequest::ReadAttributesBSON(doc);

    // Read only derived fields
    if (auto elem = doc[serial::pszHazItemList]; elem && elem.type() == bsoncxx::type::k_string)
    {
        m_strHazItemList = std::string{ elem.get_string().value };
    }

    if (auto elem = doc[serial::pszHazItemLoc]; elem && elem.type() == bsoncxx::type::k_string)
    {
        m_strHazItemZone = std::string{ elem.get_string().value };
    }

    if (auto elem = doc[serial::pszThreat]; elem && elem.type() == bsoncxx::type::k_int32)
    {
        m_threat = static_cast<ThreatLevel>(elem.get_int32().value);
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::string HazardousRequest::PrintJobDetails(dpp::cluster& cluster, const dpp::snowflake& idGuild) const
{
    std::string base = JobRequest::PrintJobDetails(cluster, idGuild);

    return fmt::format(
        "{}"
        "Threat Level: {}\n"
        "Retrieval Zone: {}\n"
        "Hazardous Item List: {}",
        base,
        ThreatToString(m_threat),
        m_strHazItemZone,
        m_strHazItemList);
}