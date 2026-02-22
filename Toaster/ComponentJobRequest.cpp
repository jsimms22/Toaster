#include "ComponentJobRequest.h"
// fmt
#include <fmt/format.h>

namespace xmlRequest
{
    constexpr const char* pszXMLCompList{ "ComponentList" };
}

void ComponentJobRequest::WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    if (xmlNode == nullptr)
    {
        return;
    }

    JobRequest::WriteAttributes(xmlNode, xmlParent);

    xmlNode->SetAttribute(xmlRequest::pszXMLCompList, m_strComponentList.c_str());
    xmlParent->InsertEndChild(xmlNode);
}

void ComponentJobRequest::ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    if (xmlNode == nullptr)
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

std::string ComponentJobRequest::PrintJobDetails(dpp::cluster* cluster) const
{
    if (!cluster)
        return {};

    std::string base = JobRequest::PrintJobDetails(cluster);

    return fmt::format(
        "{}"
        "**Component List**: \n{}\n",
        base,
        m_strComponentList);
}