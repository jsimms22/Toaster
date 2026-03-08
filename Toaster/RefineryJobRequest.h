//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "JobRequest.h"

//---------------------------------------------------------------------------------------------------------------------
/// \class RefineryJobRequest
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class RefineryJobRequest : public JobRequest
{
public:
    // Enum for Job status
    enum state
    {
        RefinedMineable = 1,
        RefinedSalvage,
        RefinedHarvest

    };

    explicit RefineryJobRequest(const dpp::snowflake& guildID) : JobRequest(guildID) {}
    virtual ~RefineryJobRequest() = default;

    static std::string StateToString(const RefineryJobRequest::state s);
    static RefineryJobRequest::state StringToState(const std::string& str);

    const RefineryJobRequest::state GetResourceState() const { return m_eResourceState; }
    void SetResourceState(const RefineryJobRequest::state eState) { m_eResourceState = eState; }

    const std::string& GetResourcelist() const { return m_strResourceList; }
    void SetResourcelist(const std::string& list) { m_strResourceList = list; }

    const std::string& GetRefinery() const { return m_strRefinery; }
    void SetRefinery(const std::string& location) { m_strRefinery = location; }

    virtual std::size_t JobType() const override { return JOB_TYPE_REFINERY; }
    virtual std::string JobTypeToString() const override { return "Refining"; }
    virtual bool SupportsType(const std::size_t type) const override { return (type == JobType() || type == JobRequest::JobType()); }

    // Serialization + Deserialization: XML
    virtual void WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent, tinyxml2::XMLDocument* doc) const override;
    virtual void ReadAttributes(tinyxml2::XMLElement* xmlNode) override;
    virtual void WriteChildren(tinyxml2::XMLElement* xmlParent, tinyxml2::XMLDocument* doc) const override { JobRequest::WriteChildren(xmlParent, doc); }
    virtual void ReadChildren(tinyxml2::XMLElement* xmlNode) override { JobRequest::ReadChildren(xmlNode); }

    // Serialization + Deserialization: BSON
    virtual bsoncxx::builder::basic::document WriteAttributesBSON() const override;
    virtual void ReadAttributesBSON(const bsoncxx::document::view& doc) override;

    virtual std::string PrintJobDetails(dpp::cluster& cluster, const dpp::snowflake& idGuild) const override;

private:
    RefineryJobRequest::state m_eResourceState = RefineryJobRequest::state::RefinedMineable;
    std::string m_strResourceList = "n/a";  // Description of the items to be refined
    std::string m_strRefinery = "any";      // Where the items are to be refined
};

