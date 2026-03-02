#include "ComponentJobRequest.h"
// fmt
#include <fmt/format.h>

namespace xmlRequest
{
    constexpr const char* pszXMLCompList{ "ComponentList" };
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void ComponentJobRequest::WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent) const
{
    if (!xmlNode || !xmlParent)
    {
        return;
    }

    JobRequest::WriteAttributes(xmlNode, xmlParent);

    xmlNode->SetAttribute(xmlRequest::pszXMLCompList, m_strComponentList.c_str());
    xmlParent->InsertEndChild(xmlNode);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void ComponentJobRequest::ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    if (!xmlNode)
    {
        return;
    }

    JobRequest::ReadAttributes(xmlNode, xmlParent);

    const char* pszCompList = xmlNode->Attribute(xmlRequest::pszXMLCompList);
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
        "**Component List**: \n{}\n",
        base,
        m_strComponentList);
}