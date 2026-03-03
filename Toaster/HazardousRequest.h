//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "JobRequest.h"

//---------------------------------------------------------------------------------------------------------------------
/// \class HazardousRequest
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class HazardousRequest : public JobRequest
{
public:
    // Enum for Job status
    enum ThreatLevel
    {
        Permissive = 1,
        Minimal,
        Uncertain,
        Hostile
    };

    HazardousRequest() = default;
    virtual ~HazardousRequest() = default;

    static std::string ThreatToString(const ThreatLevel t);
    static ThreatLevel StringToThreat(const std::string& str);

    void SetItemList(const std::string& items) { m_strHazItemList = items; }
    std::string GetItemList() const { return m_strHazItemList; }

    void SetItemLocation(const std::string& zone) { m_strHazItemZone = zone; }
    std::string GetItemLocation() const { return m_strHazItemZone; }

    void SetThreatLevel(const ThreatLevel threat) { m_threat = threat; }
    ThreatLevel GetThreatLevel() const { return m_threat; }

    virtual std::size_t JobType() const override { return JOB_TYPE_HAZARD; }
    virtual std::string JobTypeToString() const override { return "Hazardous"; }
    virtual bool SupportsType(const std::size_t type) const override { return (type == JobType() || type == JobRequest::JobType()); }
    virtual void WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent, tinyxml2::XMLDocument* doc) const override;
    virtual void ReadAttributes(tinyxml2::XMLElement* xmlNode) override;
    virtual void WriteChildren(tinyxml2::XMLElement* xmlParent, tinyxml2::XMLDocument* doc) const override { JobRequest::WriteChildren(xmlParent, doc); }
    virtual void ReadChildren(tinyxml2::XMLElement* xmlNode) override { JobRequest::ReadChildren(xmlNode); }
    virtual std::string PrintJobDetails(dpp::cluster& cluster, const dpp::snowflake& idGuild) const override;

private:
    ThreatLevel m_threat = ThreatLevel::Uncertain;  // Expected threat level of the area
    std::string m_strHazItemZone = "n/a";           // Location or zone for the items to be found
    std::string m_strHazItemList = "n/a";           // Hazardous item list
};

