#include "CraftingJobRequest.h"
// fmt
#include <fmt/format.h>

namespace xmlRequest
{
    constexpr const char* pszXMLItemDesc{ "Description" };
    constexpr const char* pszXMLItemQuantity{ "Quantity" };
    constexpr const char* pszXMLItemQuality{ "Quality" };
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void CraftingJobRequest::WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent, tinyxml2::XMLDocument* doc) const
{
    if (!xmlNode || !xmlParent || !doc)
    {
        return;
    }

    JobRequest::WriteAttributes(xmlNode, xmlParent, doc);

    xmlNode->SetAttribute(xmlRequest::pszXMLItemDesc, m_strItemDesc.c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLItemQuantity, m_strQuantity.c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLItemQuality, m_strQuality.c_str());
    xmlParent->InsertEndChild(xmlNode);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void CraftingJobRequest::ReadAttributes(tinyxml2::XMLElement* xmlNode)
{
    if (!xmlNode)
    {
        return;
    }

    JobRequest::ReadAttributes(xmlNode);

    const char* pszItemDesc = xmlNode->Attribute(xmlRequest::pszXMLItemDesc);
    if (pszItemDesc)
    {
        m_strItemDesc = pszItemDesc;
    }

    const char* pszItemQuantity = xmlNode->Attribute(xmlRequest::pszXMLItemQuantity);
    if (pszItemQuantity)
    {
        m_strQuantity = pszItemQuantity;
    }

    const char* pszItemQuality = xmlNode->Attribute(xmlRequest::pszXMLItemQuality);
    if (pszItemQuality)
    {
        m_strQuality = pszItemQuality;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::string CraftingJobRequest::PrintJobDetails(dpp::cluster& cluster, const dpp::snowflake& idGuild) const
{
    std::string base = JobRequest::PrintJobDetails(cluster, idGuild);

    return fmt::format(
        "{}"
        "**Item Description**: {}\n"
        "**Item Quantity**: {}\n"
        "**Expected Quality**: {}\n",
        base,
        m_strItemDesc,
        m_strQuantity,
        m_strQuality);
}