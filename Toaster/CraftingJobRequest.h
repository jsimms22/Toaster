//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "JobRequest.h"

//---------------------------------------------------------------------------------------------------------------------
/// \class CraftingJobRequest
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class CraftingJobRequest : public JobRequest
{
public:
    explicit CraftingJobRequest(const dpp::snowflake& guildID, const dpp::snowflake& customerID) 
        : JobRequest(guildID, customerID) {}
    virtual ~CraftingJobRequest() = default;

    const std::string& GetItemDesc() const { return m_strItemDesc; }
    void SetItemDesc(const std::string& itemDesc) { m_strItemDesc = itemDesc; }

    std::string GetQuantity() const { return m_strQuantity; }
    void SetQuantity(const std::string quantity) { m_strQuantity = quantity; }

    std::string GetQualityThres() const { return m_strQuality; }
    void SetQualityThres(const std::string quality) { m_strQuality = quality; }

    virtual std::uint64_t JobType() const override { return JOB_TYPE_CRAFTING; }
    virtual std::string JobTypeToString() const override { return "Crafting"; }
    virtual bool SupportsType(const std::uint64_t type) const override { return (type == JobType() || type == JobRequest::JobType()); }

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
    std::string m_strItemDesc = "n/a";  // Description of the item being crafted
    std::string m_strQuantity = "0";    // Quantity of item to craft
    std::string m_strQuality = "any";   // Quality the item to craft
};

