#pragma once
#include "JobRequest.h"

class ResourceJobRequest : public JobRequest
{
public:
    // Enum for Job status
    enum state
    {
        UnrefinedMineable = 1,
        RefinedMineable,
        UnrefinedSalvage,
        RefinedSalvage,
        UnrefinedHarvest,
        RefinedHarvest

    };
    ResourceJobRequest() = default;
    ~ResourceJobRequest() = default;

    static std::string StateToString(ResourceJobRequest::state s);
    static ResourceJobRequest::state StringToState(const std::string& str);

    const ResourceJobRequest::state GetResourceState() const { return m_eResourceState; }
    void SetResourceState(const ResourceJobRequest::state eState) { m_eResourceState = eState; }

    const std::string& GetResourcelist() const { return m_strResourceList; }
    void SetResourcelist(const std::string& list) { m_strResourceList = list; }

    std::string GetQualityThres() const { return m_strQuality; }
    void SetQualityThres(const std::string quality) { m_strQuality = quality; }

    virtual std::size_t JobType() const override { return JOB_TYPE_RESOURCE; }
    virtual std::string JobTypeToString() const override { return "Resource"; }
    virtual bool SupportsType(const std::size_t type) const override { return (type == JobType() || type == JobRequest::JobType()); }
    virtual void WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent) override;
    virtual void ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent) override;
    virtual std::string PrintJobDetails(dpp::cluster& cluster, const dpp::snowflake& idGuild) const override;

private:
    ResourceJobRequest::state m_eResourceState = ResourceJobRequest::state::UnrefinedMineable;
    std::string m_strResourceList = "n/a";  // Description of the item being crafted
    std::string m_strQuality = "any";       // Quality the item to craft
};

