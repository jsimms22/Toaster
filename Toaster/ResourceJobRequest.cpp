#include "ResourceJobRequest.h"
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
    constexpr const char* pszResourceState{ "ResourceState" };
    constexpr const char* pszResourceList{ "ResourceList" };
    constexpr const char* pszResourceQuality{ "Quality" };
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::string ResourceJobRequest::StateToString(const ResourceJobRequest::state e)
{
    switch (e) {
    case ResourceJobRequest::state::UnrefinedMineable:  return "mineable";
    case ResourceJobRequest::state::RefinedMineable:    return "refined_mineable";
    case ResourceJobRequest::state::UnrefinedSalvage:   return "salvage";
    case ResourceJobRequest::state::RefinedSalvage:     return "refined_salvage";
    case ResourceJobRequest::state::UnrefinedHarvest:   return "harvestable";
    case ResourceJobRequest::state::RefinedHarvest:     return "refined_harvest";
    default: throw std::invalid_argument("Invalid priority enum value");
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
ResourceJobRequest::state ResourceJobRequest::StringToState(const std::string& str)
{
    if (str == "mineable")          return ResourceJobRequest::state::UnrefinedMineable;
    if (str == "refined_mineable")  return ResourceJobRequest::state::RefinedMineable;
    if (str == "salvage")           return ResourceJobRequest::state::UnrefinedSalvage;
    if (str == "refined_salvage")   return ResourceJobRequest::state::RefinedSalvage;
    if (str == "harvestable")       return ResourceJobRequest::state::UnrefinedHarvest;
    if (str == "refined_harvest")   return ResourceJobRequest::state::RefinedHarvest;
    throw std::invalid_argument("Invalid status string value");
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void ResourceJobRequest::WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent, tinyxml2::XMLDocument* doc) const
{
    if (!xmlNode || !xmlParent || !doc)
    {
        return;
    }

    JobRequest::WriteAttributes(xmlNode, xmlParent, doc);

    xmlNode->SetAttribute(serial::pszResourceState, static_cast<int>(m_eResourceState));
    xmlNode->SetAttribute(serial::pszResourceList, m_strResourceList.c_str());
    xmlNode->SetAttribute(serial::pszResourceQuality, m_strQuality.c_str());
    xmlParent->InsertEndChild(xmlNode);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void ResourceJobRequest::ReadAttributes(tinyxml2::XMLElement* xmlNode)
{
    if (!xmlNode)
    {
        return;
    }

    JobRequest::ReadAttributes(xmlNode);

    m_eResourceState = static_cast<state>(xmlNode->IntAttribute(serial::pszResourceState, state::UnrefinedMineable));

    const char* pszResourceList = xmlNode->Attribute(serial::pszResourceList);
    if (pszResourceList)
    {
        m_strResourceList = pszResourceList;
    }

    const char* pszResourceQuality = xmlNode->Attribute(serial::pszResourceQuality);
    if (pszResourceQuality)
    {
        m_strQuality = pszResourceQuality;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
bsoncxx::builder::basic::document ResourceJobRequest::WriteAttributesBSON() const
{
    using namespace bsoncxx::builder::basic;
    auto doc = JobRequest::WriteAttributesBSON(); // base class attributes

    // add derived attributes
    doc.append(
        kvp(std::string{ serial::pszResourceList }, m_strResourceList),
        kvp(std::string{ serial::pszResourceQuality }, m_strQuality),
        kvp(std::string{ serial::pszResourceState }, m_eResourceState)
    );

    return doc;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void ResourceJobRequest::ReadAttributesBSON(const bsoncxx::document::view& doc)
{
    // Let parent read base fields AND notes
    JobRequest::ReadAttributesBSON(doc);

    // Read only derived fields
    if (auto elem = doc[std::string{ serial::pszResourceList }])
        m_strResourceList = std::string{ elem.get_string().value };

    if (auto elem = doc[std::string{ serial::pszResourceQuality }])
        m_strQuality = std::string{ elem.get_string().value };

    if (auto elem = doc[std::string{ serial::pszResourceState }])
        m_eResourceState = static_cast<state>(static_cast<int>(elem.get_int32().value));
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::string ResourceJobRequest::PrintJobDetails(dpp::cluster& cluster, const dpp::snowflake& idGuild) const
{
    std::string base = JobRequest::PrintJobDetails(cluster, idGuild);

    return fmt::format(
        "{}"
        "Resource Type: {}\n"
        "Resource List: {}\n"
        "Expected Quality: {}",
        base,
        StateToString(m_eResourceState),
        m_strResourceList,
        m_strQuality);
}