#include "ResourceJobRequest.h"
// fmt
#include <fmt/format.h>

namespace xmlRequest
{
    constexpr const char* pszXMLResourceState{ "ResourceState" };
    constexpr const char* pszXMLResourceList{ "ResourceList" };
    constexpr const char* pszXMLResourceQuality{ "Quality" };
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
void ResourceJobRequest::WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent) const
{
    if (!xmlNode || !xmlParent)
    {
        return;
    }

    JobRequest::WriteAttributes(xmlNode, xmlParent);

    xmlNode->SetAttribute(xmlRequest::pszXMLResourceState, static_cast<int>(m_eResourceState));
    xmlNode->SetAttribute(xmlRequest::pszXMLResourceList, m_strResourceList.c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLResourceQuality, m_strQuality.c_str());
    xmlParent->InsertEndChild(xmlNode);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void ResourceJobRequest::ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    if (!xmlNode)
    {
        return;
    }

    JobRequest::ReadAttributes(xmlNode, xmlParent);

    m_eResourceState = static_cast<state>(xmlNode->IntAttribute(xmlRequest::pszXMLResourceState, state::UnrefinedMineable));

    const char* pszResourceList = xmlNode->Attribute(xmlRequest::pszXMLResourceList);
    if (pszResourceList)
    {
        m_strResourceList = pszResourceList;
    }

    const char* pszResourceQuality = xmlNode->Attribute(xmlRequest::pszXMLResourceQuality);
    if (pszResourceQuality)
    {
        m_strQuality = pszResourceQuality;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::string ResourceJobRequest::PrintJobDetails(dpp::cluster& cluster, const dpp::snowflake& idGuild) const
{
    std::string base = JobRequest::PrintJobDetails(cluster, idGuild);

    return fmt::format(
        "{}"
        "**Resource Type**: {}\n"
        "**Resource List**: \n{}\n"
        "**Expected Quality**: {}\n",
        base,
        StateToString(m_eResourceState),
        m_strResourceList,
        m_strQuality);
}