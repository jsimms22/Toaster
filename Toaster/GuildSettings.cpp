#include "GuildSettings.h"

#include "Resource.h"


//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
const GuildSettings::Roles GuildSettings::JobTypeToRole(const std::size_t type) const
{
    switch (type)
    {
    case JOB_TYPE_CRAFTING: { return GuildSettings::Roles::Crafter; }
    case JOB_TYPE_BUILDING: { return GuildSettings::Roles::Builder; }
    case JOB_TYPE_RESOURCE: { return GuildSettings::Roles::Gatherer; }
    case JOB_TYPE_REFINERY: { return GuildSettings::Roles::Refiner; }
    case JOB_TYPE_COMPONENT: { return GuildSettings::Roles::Comp; }
    case JOB_TYPE_HAZARD: { return GuildSettings::Roles::Hazmat; }
    default: { return GuildSettings::Roles::Manager; }
    }
}