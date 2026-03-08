#include "CraftingJobRequest.h"
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
    constexpr const char* pszItemDesc{ "craft_list" };
    constexpr const char* pszItemQuantity{ "craft_quantity" };
    constexpr const char* pszItemQuality{ "craft_quality" };
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

    xmlNode->SetAttribute(serial::pszItemDesc, m_strItemDesc.c_str());
    xmlNode->SetAttribute(serial::pszItemQuantity, m_strQuantity.c_str());
    xmlNode->SetAttribute(serial::pszItemQuality, m_strQuality.c_str());
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

    const char* pszItemDesc = xmlNode->Attribute(serial::pszItemDesc);
    if (pszItemDesc)
    {
        m_strItemDesc = pszItemDesc;
    }

    const char* pszItemQuantity = xmlNode->Attribute(serial::pszItemQuantity);
    if (pszItemQuantity)
    {
        m_strQuantity = pszItemQuantity;
    }

    const char* pszItemQuality = xmlNode->Attribute(serial::pszItemQuality);
    if (pszItemQuality)
    {
        m_strQuality = pszItemQuality;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
bsoncxx::builder::basic::document CraftingJobRequest::WriteAttributesBSON() const
{
    using namespace bsoncxx::builder::basic;
    auto doc = JobRequest::WriteAttributesBSON(); // base class attributes

    // add derived attributes
    doc.append(
        kvp(std::string{ serial::pszItemDesc }, m_strItemDesc),
        kvp(std::string{ serial::pszItemQuantity }, m_strQuantity),
        kvp(std::string{ serial::pszItemQuality }, m_strQuality)
    );

    return doc;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void CraftingJobRequest::ReadAttributesBSON(const bsoncxx::document::view & doc)
{
    // Let parent read base fields AND notes
    JobRequest::ReadAttributesBSON(doc);

    // Read only derived fields
    if (auto elem = doc[std::string{ serial::pszItemDesc }])
        m_strItemDesc = std::string{ elem.get_string().value };

    if (auto elem = doc[std::string{ serial::pszItemQuantity }])
        m_strQuantity = std::string{ elem.get_string().value };

    if (auto elem = doc[std::string{ serial::pszItemQuality }])
        m_strQuality = std::string{ elem.get_string().value };
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