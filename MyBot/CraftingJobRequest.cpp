#include "CraftingJobRequest.h"

namespace xmlRequest
{
    const char* pszXMLItemDesc{ "Description" };
    const char* pszXMLItemQuantity{ "Quantity" };
}

void CraftingJobRequest::WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    // Check if the xmlNode is valid.
    if (xmlNode == nullptr)
    {
        return;
    }

    JobRequest::WriteAttributes(xmlNode, xmlParent);

    // Create a new <Request> element for this submission
    xmlNode->SetAttribute(xmlRequest::pszXMLItemDesc, m_strItemDesc.c_str());
    xmlNode->SetAttribute(xmlRequest::pszXMLItemQuantity, std::to_string(m_ulQuantity).c_str());
    xmlParent->InsertEndChild(xmlNode);
}

void CraftingJobRequest::ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    // Check if the xmlNode is valid.
    if (xmlNode == nullptr)
    {
        return;
    }

    JobRequest::ReadAttributes(xmlNode, xmlParent);

    // Read the attributes and set the member variables
    const char* itemDesc = xmlNode->Attribute(xmlRequest::pszXMLItemDesc);
    if (itemDesc)
    {
        m_strItemDesc = itemDesc;
    }

    const char* itemQuantity = xmlNode->Attribute(xmlRequest::pszXMLItemQuantity);
    if (itemQuantity)
    {
        m_ulQuantity = std::stoull(itemQuantity);  // Assuming quantity is a positive integer
    }
}

std::string CraftingJobRequest::PrintJobDetails() const
{
    std::string str = JobRequest::PrintJobDetails();
    std::stringstream ss;
    ss << "Item Description: " << m_strItemDesc << std::endl;
    ss << "Item Quantity: " << m_ulQuantity << std::endl;
    return str + ss.str();
}