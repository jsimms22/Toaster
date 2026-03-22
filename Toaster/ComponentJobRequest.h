//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "JobRequest.h"

//---------------------------------------------------------------------------------------------------------------------
/// \class ComponentJobRequest
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class ComponentJobRequest : public JobRequest
{
public:
    explicit ComponentJobRequest(const dpp::snowflake& guildID, const dpp::snowflake& customerID) 
        : JobRequest(guildID, customerID) {}
    virtual ~ComponentJobRequest() = default;

    const std::string& GetComponentList() const { return m_strComponentList; }
    void SetComponentList(const std::string& list) { m_strComponentList = list; }

    virtual std::uint64_t JobType() const override { return JOB_TYPE_COMPONENT; }
    virtual std::string JobTypeToString() const override { return "Component"; }
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
    std::string m_strComponentList = "n/a";      // Description of the item being crafted
};

