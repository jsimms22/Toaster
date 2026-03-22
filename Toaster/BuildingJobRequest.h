//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "JobRequest.h"

//---------------------------------------------------------------------------------------------------------------------
/// \class BuildingJobRequest
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class BuildingJobRequest : public JobRequest
{
public:
    explicit BuildingJobRequest(const dpp::snowflake& guildID, const dpp::snowflake& customerID) 
        : JobRequest(guildID, customerID) {}
    virtual ~BuildingJobRequest() = default;

    const std::string& GetBuildDesign() const { return m_strBldgDesignation; }
    void SetBuildDesign(const std::string& designation) { m_strBldgDesignation = designation; }

    const std::string& GetBuildRequirments() const { return m_strBldgRequires; }
    void SetBuildRequirments(const std::string& requirements) { m_strBldgRequires = requirements; }

    const std::string& GetBuildZone() const { return m_strBldgZone; }
    void SetBuildZone(const std::string& zone) { m_strBldgZone = zone; }

    virtual std::uint64_t JobType() const override { return JOB_TYPE_BUILDING; }
    virtual std::string JobTypeToString() const override { return "Building"; }
    virtual bool SupportsType(const std::uint64_t type) const override { return (type == JobType() || type == JobRequest::JobType()); }

    // Serialization + Deserialization: XML
    virtual void WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent, tinyxml2::XMLDocument* doc) const override;
    virtual void ReadAttributes(tinyxml2::XMLElement* xmlNode) override;
    virtual void WriteChildren(tinyxml2::XMLElement* xmlParent, tinyxml2::XMLDocument* doc) const override { JobRequest::WriteChildren(xmlParent, doc); }
    virtual void ReadChildren(tinyxml2::XMLElement* xmlNode) override { JobRequest::ReadChildren(xmlNode); }

    // Serialization + Deserialization: BSON
    virtual bsoncxx::builder::basic::document WriteAttributesBSON() const override;
    virtual void ReadAttributesBSON(const bsoncxx::document::view& doc) override;
    
    virtual std::string PrintJobDetails(dpp::cluster& cluster, const dpp::snowflake& idGuild, const bool bPrintNames = false) const override;

private:
    std::string m_strBldgDesignation = "n/a";   // Purpose/Type of building
    std::string m_strBldgRequires = "n/a";      // Requirements
    std::string m_strBldgZone = "n/a";      // Location or zone to place the building
};

