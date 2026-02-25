#pragma once
#include "JobRequest.h"

class ComponentJobRequest : public JobRequest
{
public:
    ComponentJobRequest() = default;
    ~ComponentJobRequest() = default;

    const std::string& GetComponentList() const { return m_strComponentList; }
    void SetComponentList(const std::string& list) { m_strComponentList = list; }

    virtual std::size_t JobType() const override { return JOB_TYPE_COMPONENT; }
    virtual std::string JobTypeToString() const override { return "Component"; }
    virtual bool SupportsType(const std::size_t type) const override { return (type == JobType() || type == JobRequest::JobType()); }
    virtual void WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent) override;
    virtual void ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent) override;
    virtual std::string PrintJobDetails(dpp::cluster& cluster, const dpp::snowflake& idGuild) const override;

private:
    std::string m_strComponentList = "n/a";      // Description of the item being crafted
};

