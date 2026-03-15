#include "ComponentJobRequest.h"
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
    constexpr const char* pszCompList{ "comp_list" };
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void ComponentJobRequest::WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent, tinyxml2::XMLDocument* doc) const
{
    if (!xmlNode || !xmlParent || !doc)
    {
        return;
    }

    JobRequest::WriteAttributes(xmlNode, xmlParent, doc);

    xmlNode->SetAttribute(serial::pszCompList, m_strComponentList.c_str());
    xmlParent->InsertEndChild(xmlNode);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void ComponentJobRequest::ReadAttributes(tinyxml2::XMLElement* xmlNode)
{
    if (!xmlNode)
    {
        return;
    }

    JobRequest::ReadAttributes(xmlNode);

    const char* pszCompList = xmlNode->Attribute(serial::pszCompList);
    if (pszCompList)
    {
        m_strComponentList = pszCompList;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::string ComponentJobRequest::PrintJobDetails(dpp::cluster& cluster, const dpp::snowflake& idGuild) const
{
    std::string base = JobRequest::PrintJobDetails(cluster, idGuild);

    return fmt::format(
        "{}"
        "Component List: {}",
        base,
        m_strComponentList);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
bsoncxx::builder::basic::document ComponentJobRequest::WriteAttributesBSON() const
{
    using namespace bsoncxx::builder::basic;
    auto doc = JobRequest::WriteAttributesBSON(); // base class attributes

    // add derived attributes
    doc.append(
        kvp(std::string{ serial::pszCompList }, m_strComponentList)
    );

    return doc;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void ComponentJobRequest::ReadAttributesBSON(const bsoncxx::document::view& doc)
{
    // Let parent read base fields AND notes
    JobRequest::ReadAttributesBSON(doc);

    // Read only derived fields
    if (auto elem = doc[std::string{ serial::pszCompList }])
        m_strComponentList = std::string{ elem.get_string().value };
}