#pragma once
#include "JobRequest.h"

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
    RefineryJobRequest() = default;
    ~RefineryJobRequest() = default;

    static std::string StateToString(RefineryJobRequest::state s);
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
    virtual void WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent) override;
    virtual void ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent) override;
    virtual std::string PrintJobDetails(dpp::cluster& cluster) const override;

private:
    RefineryJobRequest::state m_eResourceState = RefineryJobRequest::state::RefinedMineable;
    std::string m_strResourceList = "n/a";  // Description of the items to be refined
    std::string m_strRefinery = "n/a";  // Description of the items to be refined
};

