#pragma once
#include "JobRequest.h"

class BuildingJobRequest : public JobRequest
{
public:
    BuildingJobRequest() = default;
    ~BuildingJobRequest() = default;

    const std::string& GetBuildDesign() const { return m_strBldgDesignation; }
    void SetBuildDesign(const std::string& designation) { m_strBldgDesignation = designation; }

    const std::string& GetBuildRequirments() const { return m_strBldgRequires; }
    void SetBuildRequirments(const std::string& requirements) { m_strBldgRequires = requirements; }

    const std::string& GetBuildZone() const { return m_strBldgZone; }
    void SetBuildZone(const std::string& zone) { m_strBldgZone = zone; }

    virtual std::size_t JobType() const override { return JOB_TYPE_BUILDING; }
    virtual std::string JobTypeToString() const override { return "Building"; }
    virtual bool SupportsType(const std::size_t type) const override { return (type == JobType() || type == JobRequest::JobType()); }
    virtual void WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent) override;
    virtual void ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent) override;
    virtual std::string PrintJobDetails(dpp::cluster* cluster) const override;

private:
    std::string m_strBldgDesignation = "n/a";   // Purpose/Type of building
    std::string m_strBldgRequires = "n/a";      // Requirements
    std::string m_strBldgZone = "n/a";      // Location or zone to place the building
};

