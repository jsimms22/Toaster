#pragma once
#include "JobRequest.h"

class CraftingJobRequest : public JobRequest
{
public:
    CraftingJobRequest() = default;
    ~CraftingJobRequest() = default;

    const std::string& GetItemDesc() const { return m_strItemDesc; }
    void SetItemDesc(const std::string& itemDesc) { m_strItemDesc = itemDesc; }
    std::size_t GetQuantity() const { return m_ulQuantity; }
    void SetQuantity(std::size_t quantity) { m_ulQuantity = quantity; }


    virtual std::size_t JobType() const override { return JOB_TYPE_CRAFTING; }
    virtual std::string JobTypeToString() const override { return "Crafting"; }
    virtual bool SupportsType(const std::size_t type) const override { return (type == JobType() || type == JobRequest::JobType()); }
    virtual void WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent) override;
    virtual void ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent) override;
    virtual std::string PrintJobDetails() const override;

private:
    std::string m_strItemDesc;      // Description of the item being crafted
    std::size_t m_ulQuantity = 0;   // Quantity of items to craft
};

