#include "RefineryJobRequest.h"
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
    constexpr const char* pszRefineState{ "refine_type" };
    constexpr const char* pszRefineList{ "refine_list" };
    constexpr const char* pszRefineryLoc{ "refine_location" };
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

    xmlNode->SetAttribute(serial::pszRefineState, static_cast<int>(m_eResourceState));
    xmlNode->SetAttribute(serial::pszRefineList, m_strResourceList.c_str());
    xmlNode->SetAttribute(serial::pszRefineryLoc, m_strRefinery.c_str());
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

    m_eResourceState = static_cast<state>(xmlNode->IntAttribute(serial::pszRefineState, state::RefinedMineable));


    const char* pszResourceList = xmlNode->Attribute(serial::pszRefineList);
    if (pszResourceList)
    {
        m_strResourceList = pszResourceList;
    }

    const char* pszRefineryloc = xmlNode->Attribute(serial::pszRefineryLoc);
    if (pszRefineryloc)
    {
        m_strRefinery = pszRefineryloc;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
bsoncxx::builder::basic::document RefineryJobRequest::WriteAttributesBSON() const
{
    using namespace bsoncxx::builder::basic;
    auto doc = JobRequest::WriteAttributesBSON(); // base class attributes

    // add derived attributes
    doc.append(
        kvp(std::string{ serial::pszRefineList }, m_strResourceList),
        kvp(std::string{ serial::pszRefineryLoc }, m_strRefinery),
        kvp(std::string{ serial::pszRefineState }, m_eResourceState)
    );

    return doc;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void RefineryJobRequest::ReadAttributesBSON(const bsoncxx::document::view& doc)
{
    // Let parent read base fields AND notes
    JobRequest::ReadAttributesBSON(doc);

    // Read only derived fields
    if (auto elem = doc[serial::pszRefineList]; elem && elem.type() == bsoncxx::type::k_string)
    {
        m_strResourceList = std::string{ elem.get_string().value };
    }

    if (auto elem = doc[serial::pszRefineryLoc]; elem && elem.type() == bsoncxx::type::k_string)
    {
        m_strRefinery = std::string{ elem.get_string().value };
    }

    if (auto elem = doc[serial::pszRefineState]; elem && elem.type() == bsoncxx::type::k_int32)
    {
        m_eResourceState = static_cast<state>(elem.get_int32().value);
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::string RefineryJobRequest::PrintJobDetails(dpp::cluster& cluster, const dpp::snowflake& idGuild, const bool bPrintNames) const
{
    std::string base = JobRequest::PrintJobDetails(cluster, idGuild, bPrintNames);

    return fmt::format(
        "{}"
        "Resource Type: {}\n"
        "Resource List:{}\n"
        "Refinery Site: {}",
        base,
        RefineryJobRequest::StateToString(m_eResourceState),
        m_strResourceList,
        m_strRefinery);
}