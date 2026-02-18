#include "ResourceJobRequest.h"

namespace xmlRequest
{
    const char* pszXMLResourceState{ "ResourceState" };
    const char* pszXMLResourceList{ "ResourceList" };
    const char* pszXMLResourceQuality{ "Quality" };
}

// Function to convert priority enum to string
std::string ResourceJobRequest::StateToString(ResourceJobRequest::state e)
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

// Function to convert string to status enum
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

void ResourceJobRequest::WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    if (xmlNode == nullptr)
    {
        return;
    }

    JobRequest::WriteAttributes(xmlNode, xmlParent);

    xmlNode->SetAttribute(xmlRequest::pszXMLResourceState, StateToString(m_eResourceState).c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLResourceList, m_strResourceList.c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLResourceQuality, m_strQuality.c_str());
    xmlParent->InsertEndChild(xmlNode);
}

void ResourceJobRequest::ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    if (xmlNode == nullptr)
    {
        return;
    }

    JobRequest::ReadAttributes(xmlNode, xmlParent);

    const char* pszState = xmlNode->Attribute(xmlRequest::pszXMLResourceState);
    if (pszState)
    {
        m_eResourceState = ResourceJobRequest::StringToState(pszState);
    }

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

std::string ResourceJobRequest::PrintJobDetails() const
{
    std::string str = JobRequest::PrintJobDetails();
    std::stringstream ss;
    ss << "**Resource Type**: " << StateToString(m_eResourceState) << std::endl;
    ss << "**Resource List**: \n" << m_strResourceList << std::endl;
    ss << "**Expected Quality**: " << m_strQuality << std::endl;
    return str + ss.str();
}