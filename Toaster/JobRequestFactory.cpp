#include "JobRequestFactory.h"
#include "CraftingJobRequest.h"
#include "BuildingJobRequest.h"
#include "ComponentJobRequest.h"
#include "ResourceJobRequest.h"
#include "RefineryJobRequest.h"
#include "HazardousRequest.h"

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<JobRequest> JobRequestFactory::Create(const std::size_t type, const dpp::snowflake& guildID)
{
    switch (type)
    {
    case JOB_TYPE_CRAFTING:
        return std::make_shared<CraftingJobRequest>(guildID, 0);

    case JOB_TYPE_BUILDING:
        return std::make_shared<BuildingJobRequest>(guildID, 0);

    case JOB_TYPE_COMPONENT:
        return std::make_shared<ComponentJobRequest>(guildID, 0);

    case JOB_TYPE_RESOURCE:
        return std::make_shared<ResourceJobRequest>(guildID, 0);

    case JOB_TYPE_REFINERY:
        return std::make_shared<RefineryJobRequest>(guildID, 0);

    case JOB_TYPE_HAZARD:
        return std::make_shared<HazardousRequest>(guildID, 0);

    default:
        return std::make_shared<JobRequest>(guildID, 0);
    }
}