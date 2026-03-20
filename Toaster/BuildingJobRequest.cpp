#include "BuildingJobRequest.h"
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
    constexpr const char* pszBuildDesignation{ "build_designation" };
    constexpr const char* pszBuildRequirements{ "build_requirements" };
    constexpr const char* pszBuildZone{ "build_zone" };
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void BuildingJobRequest::WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent, tinyxml2::XMLDocument* doc) const
{
    if (!xmlNode || !xmlParent || !doc)
    {
        return;
    }

    JobRequest::WriteAttributes(xmlNode, xmlParent ,doc);

    xmlNode->SetAttribute(serial::pszBuildDesignation, m_strBldgDesignation.c_str());
    xmlNode->SetAttribute(serial::pszBuildRequirements, m_strBldgRequires.c_str());
    xmlNode->SetAttribute(serial::pszBuildZone, m_strBldgZone.c_str());
    xmlParent->InsertEndChild(xmlNode);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void BuildingJobRequest::ReadAttributes(tinyxml2::XMLElement* xmlNode)
{
    if (!xmlNode)
    {
        return;
    }

    JobRequest::ReadAttributes(xmlNode);

    const char* pszDesignation = xmlNode->Attribute(serial::pszBuildDesignation);
    if (pszDesignation)
    {
        m_strBldgDesignation = pszDesignation;
    }

    const char* pszRequires = xmlNode->Attribute(serial::pszBuildRequirements);
    if (pszRequires)
    {
        m_strBldgRequires = pszRequires;
    }

    const char* pszBuildZone = xmlNode->Attribute(serial::pszBuildZone);
    if (pszBuildZone)
    {
        m_strBldgZone = pszBuildZone;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
bsoncxx::builder::basic::document BuildingJobRequest::WriteAttributesBSON() const
{
    using namespace bsoncxx::builder::basic;
    auto doc = JobRequest::WriteAttributesBSON(); // base class attributes

    // add derived attributes
    doc.append(
        kvp(std::string{ serial::pszBuildDesignation }, m_strBldgDesignation),
        kvp(std::string{ serial::pszBuildRequirements }, m_strBldgRequires),
        kvp(std::string{ serial::pszBuildZone }, m_strBldgZone)
    );

    return doc;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void BuildingJobRequest::ReadAttributesBSON(const bsoncxx::document::view& doc)
{
    // Let parent read base fields AND notes
    JobRequest::ReadAttributesBSON(doc);

    // Read only derived fields
    if (auto elem = doc[serial::pszBuildDesignation]; elem && elem.type() == bsoncxx::type::k_string)
    {
        m_strBldgDesignation = std::string{ elem.get_string().value };
    }

    if (auto elem = doc[serial::pszBuildRequirements]; elem && elem.type() == bsoncxx::type::k_string)
    {
        m_strBldgRequires = std::string{ elem.get_string().value };
    }

    if (auto elem = doc[serial::pszBuildZone]; elem && elem.type() == bsoncxx::type::k_string)
    {
        m_strBldgZone = std::string{ elem.get_string().value };
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::string BuildingJobRequest::PrintJobDetails(dpp::cluster& cluster, const dpp::snowflake& idGuild) const
{
    std::string base = JobRequest::PrintJobDetails(cluster, idGuild);

    return fmt::format(
        "{}"
        "Building Designation: {}\n"
        "Building Zone: {}\n"
        "Building Requirements: {}",
        base,
        m_strBldgDesignation,
        m_strBldgZone,
        m_strBldgRequires);
}