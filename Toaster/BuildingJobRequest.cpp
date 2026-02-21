#include "BuildingJobRequest.h"

#include <sstream>

namespace xmlRequest
{
    const char* pszXMLBuildDesignation{ "Designation" };
    const char* pszXMLBuildRequirements{ "Requirements" };
    const char* pszXMLBuildZone{ "Zone" };
}

void BuildingJobRequest::WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    if (xmlNode == nullptr)
    {
        return;
    }

    JobRequest::WriteAttributes(xmlNode, xmlParent);

    xmlNode->SetAttribute(xmlRequest::pszXMLBuildDesignation, m_strBldgDesignation.c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLBuildRequirements, m_strBldgRequires.c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLBuildZone, m_strBldgZone.c_str());
    xmlParent->InsertEndChild(xmlNode);
}

void BuildingJobRequest::ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    if (xmlNode == nullptr)
    {
        return;
    }

    JobRequest::ReadAttributes(xmlNode, xmlParent);

    const char* pszDesignation = xmlNode->Attribute(xmlRequest::pszXMLBuildDesignation);
    if (pszDesignation)
    {
        m_strBldgDesignation = pszDesignation;
    }

    const char* pszRequires = xmlNode->Attribute(xmlRequest::pszXMLBuildRequirements);
    if (pszRequires)
    {
        m_strBldgRequires = pszRequires;
    }

    const char* pszBuildZone = xmlNode->Attribute(xmlRequest::pszXMLBuildZone);
    if (pszBuildZone)
    {
        m_strBldgZone = pszBuildZone;
    }
}

std::string BuildingJobRequest::PrintJobDetails(dpp::cluster* cluster) const
{
    if (!cluster)
        return {};

    std::string str = JobRequest::PrintJobDetails(cluster);
    std::stringstream ss;
    ss << "**Building Designation**: " << m_strBldgDesignation << std::endl;
    ss << "**Building Requirements**: \n" << m_strBldgRequires << std::endl;
    ss << "**Building Zone**: " << m_strBldgZone << std::endl;
    return str + ss.str();
}