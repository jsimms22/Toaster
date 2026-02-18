#include "ComponentJobRequest.h"

namespace xmlRequest
{
    const char* pszXMLCompList{ "ComponentList" };
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

std::string ComponentJobRequest::PrintJobDetails() const
{
    std::string str = JobRequest::PrintJobDetails();
    std::stringstream ss;
    ss << "**Component List**: \n" << m_strComponentList << std::endl;
    return str + ss.str();
}