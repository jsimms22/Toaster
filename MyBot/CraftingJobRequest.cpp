#include "CraftingJobRequest.h"

#include <sstream>

namespace xmlRequest
{
    const char* pszXMLItemDesc{ "Description" };
    const char* pszXMLItemQuantity{ "Quantity" };
    const char* pszXMLItemQuality{ "Quality" };
}

void CraftingJobRequest::WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    if (xmlNode == nullptr)
    {
        return;
    }

    JobRequest::WriteAttributes(xmlNode, xmlParent);

    xmlNode->SetAttribute(xmlRequest::pszXMLItemDesc, m_strItemDesc.c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLItemQuantity, std::to_string(m_ulQuantity).c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLItemQuality, m_strQuality.c_str());
    xmlParent->InsertEndChild(xmlNode);
}

void CraftingJobRequest::ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    if (xmlNode == nullptr)
    {
        return;
    }

    JobRequest::ReadAttributes(xmlNode, xmlParent);

    const char* pszItemDesc = xmlNode->Attribute(xmlRequest::pszXMLItemDesc);
    if (pszItemDesc)
    {
        m_strItemDesc = pszItemDesc;
    }

    const char* pszItemQuantity = xmlNode->Attribute(xmlRequest::pszXMLItemQuantity);
    if (pszItemQuantity)
    {
        m_ulQuantity = std::stoull(pszItemQuantity);
    }

    const char* pszItemQuality = xmlNode->Attribute(xmlRequest::pszXMLItemQuality);
    if (pszItemQuality)
    {
        m_strQuality = pszItemQuality;
    }
}

std::string CraftingJobRequest::PrintJobDetails() const
{
    std::string str = JobRequest::PrintJobDetails();
    std::stringstream ss;
    ss << "**Item Description**: " << m_strItemDesc << std::endl;
    ss << "**Item Quantity**: " << m_ulQuantity << std::endl;
    ss << "**Expected Quality**: " << m_strQuality << std::endl;
    return str + ss.str();
}