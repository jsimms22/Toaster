#include "GuildSettings.h"

#include "CommandContext.h"
#include "GlobalButtonPanel.h"
#include "JobRequest.h"
#include "JobQueue.h"
#include "IJobRepo.h"
#include "Resource.h"
// d++
#include <dpp/message.h>
// fmt
#include <fmt/format.h>
// mongo
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/concatenate.hpp>
// std library
#include <string>

// ================================
// Helper Utilities
// ================================
namespace
{
    void WriteOptionalId(tinyxml2::XMLElement* xmlNode, const char* name, const std::optional<dpp::snowflake>& value)
    {
        if (value.has_value()) { xmlNode->SetAttribute(name, value.value()); }
        else { xmlNode->SetAttribute(name, 0ULL); }
    }

    std::optional<dpp::snowflake> ReadOptionalId(tinyxml2::XMLElement* xmlNode, const char* name)
    {
        const std::size_t attr = xmlNode->Unsigned64Attribute(name, 0);
        if (!attr) { return std::nullopt; }
        return attr;
    }

    std::optional<dpp::snowflake> ReadOptionalId(bsoncxx::document::element elem)
    {
        const std::size_t attr = static_cast<std::size_t>(elem.get_int64().value);
        if (!attr) { return std::nullopt; }
        return attr;
    }
}

const std::array<const char*, 8> GuildSettings::RoleNames
{
    "General Worker",
    "Item Crafter",
    "Base Builder",
    "Component Dealer",
    "Resource Gatherer",
    "Refinery Worker",
    "Hazardous Materials Collector",
    "Manager"
};

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
std::optional<GuildSettings::Roles> GuildSettings::JobTypeToRole(const std::size_t type)
{
    switch (type)
    {
        case JOB_TYPE_CRAFTING: { return GuildSettings::Roles::Crafter; }
        case JOB_TYPE_BUILDING: { return GuildSettings::Roles::Builder; }
        case JOB_TYPE_RESOURCE: { return GuildSettings::Roles::Gatherer; }
        case JOB_TYPE_REFINERY: { return GuildSettings::Roles::Refiner; }
        case JOB_TYPE_COMPONENT: { return GuildSettings::Roles::Comp; }
        case JOB_TYPE_HAZARD: { return GuildSettings::Roles::Hazmat; }
        default: { return std::nullopt; }
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
std::optional<std::size_t> GuildSettings::RoleToJobType(const GuildSettings::Roles role)
{
    switch (static_cast<std::size_t>(role))
    {
        case static_cast<std::size_t>(Roles::Crafter):  { return JOB_TYPE_CRAFTING; }
        case static_cast<std::size_t>(Roles::Builder):  { return JOB_TYPE_BUILDING; }
        case static_cast<std::size_t>(Roles::Gatherer): { return JOB_TYPE_RESOURCE; }
        case static_cast<std::size_t>(Roles::Refiner):  { return JOB_TYPE_REFINERY; }
        case static_cast<std::size_t>(Roles::Comp):     { return JOB_TYPE_COMPONENT; }
        case static_cast<std::size_t>(Roles::Hazmat):   { return JOB_TYPE_HAZARD; }
        default: { return std::nullopt; }
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void GuildSettings::AnnounceOnNew(CommandContext& ctx, const std::shared_ptr<const JobRequest> job, const bool bReopened)
{
    if (!job)
    {
        return;
    }

    std::string roleMention;
    if (ctx.guild->bPingOnNew)
    {
        if (const auto roleOpt = GuildSettings::JobTypeToRole(job->JobType()); roleOpt.has_value())
        {
            const auto roleEnum = *roleOpt;
            const std::optional<dpp::snowflake> roleSnowflake = ctx.guild->roles[static_cast<size_t>(roleEnum)];

            if (roleSnowflake.has_value())
            {
                roleMention = "<@&" + std::to_string(*roleSnowflake) + ">";
            }
        }

        if (const auto roleOpt = ctx.guild->roles[static_cast<size_t>(GuildSettings::Roles::Ping)]; roleOpt.has_value())
        {
            roleMention += (!roleMention.empty() ? "\n" : "");
            roleMention += "<@&" + std::to_string(roleOpt.value()) + ">";
        }
    }

    if (ctx.guild->idNewJobChannel.has_value() && job)
    {
        GlobalButtonPanel panel{ std::to_string(job->GetID().value), bReopened };
        panel.AddEmbed(bReopened ? "Reopened Job Request" : "New Job Request", job->PrintJobDetails(ctx.cluster, ctx.guild->m_idGuild));
        panel.set_content(roleMention);
        panel.set_channel_id(ctx.guild->idNewJobChannel.value_or(0));

        ctx.cluster.message_create(panel);
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void GuildSettings::AnnounceOnUpdate(CommandContext& ctx, const std::shared_ptr<const JobRequest> job)
{
    if (!job)
    {
        return;
    }

    std::string roleMention;
    if (ctx.guild->bPingOnUpdate)
    {
        if (const auto roleOpt = GuildSettings::JobTypeToRole(job->JobType()); roleOpt.has_value())
        {
            const auto roleEnum = *roleOpt;
            const std::optional<dpp::snowflake> roleSnowflake = ctx.guild->roles[static_cast<size_t>(roleEnum)];

            if (roleSnowflake.has_value())
            {
                roleMention = "<@&" + std::to_string(*roleSnowflake) + ">";
            }
        }

        if (const auto roleOpt = ctx.guild->roles[static_cast<size_t>(GuildSettings::Roles::Ping)]; roleOpt.has_value())
        {
            roleMention += (!roleMention.empty() ? "\n" : "");
            roleMention += "<@&" + std::to_string(roleOpt.value()) + ">";
        }
    }
    
    if (ctx.guild->idUpdateJobChannel.has_value())
    {
        dpp::embed announce;
        announce.set_title("Job Request Edited")
            .set_description(job->PrintJobDetails(ctx.cluster, ctx.guild->m_idGuild))
            .set_color(0x3498db);

        ctx.cluster.message_create(dpp::message(ctx.guild->idUpdateJobChannel.value_or(0), roleMention).add_embed(announce));
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void GuildSettings::AnnounceOnDelete(CommandContext& ctx, const std::size_t jobType, const std::string& jobDetails)
{
    std::string roleMention;
    if (ctx.guild->bPingOnDelete)
    {
        if (const auto roleOpt = GuildSettings::JobTypeToRole(jobType); roleOpt.has_value())
        {
            const auto roleEnum = *roleOpt;
            const std::optional<dpp::snowflake> roleSnowflake = ctx.guild->roles[static_cast<size_t>(roleEnum)];

            if (roleSnowflake.has_value())
            {
                roleMention = "<@&" + std::to_string(*roleSnowflake) + ">";
            }
        }

        if (const auto roleOpt = ctx.guild->roles[static_cast<size_t>(GuildSettings::Roles::Ping)]; roleOpt.has_value())
        {
            roleMention += (!roleMention.empty() ? "\n" : "");
            roleMention += "<@&" + std::to_string(roleOpt.value()) + ">";
        }
    }

    if (ctx.guild->idDeleteJobChannel.has_value())
    {
        dpp::embed announce;
        announce.set_title("Job Request Deleted")
            .set_description(jobDetails)
            .set_color(0x3498db);

        ctx.cluster.message_create(dpp::message(ctx.guild->idDeleteJobChannel.value_or(0), roleMention).add_embed(announce));
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void GuildSettings::AnnounceOnComplete(CommandContext& ctx, const std::shared_ptr<const JobRequest> job)
{
    if (!job) 
    {
        return;
    }

    std::string roleMention;
    if (ctx.guild->bPingOnComplete)
    {
        if (const auto roleOpt = GuildSettings::JobTypeToRole(job->JobType()); roleOpt.has_value())
        {
            const auto roleEnum = *roleOpt;
            const std::optional<dpp::snowflake> roleSnowflake = ctx.guild->roles[static_cast<size_t>(roleEnum)];

            if (roleSnowflake.has_value())
            {
                roleMention = "<@&" + std::to_string(roleSnowflake.value()) + ">";
            }
        }

        if (const auto roleOpt = ctx.guild->roles[static_cast<size_t>(GuildSettings::Roles::Ping)]; roleOpt.has_value())
        {
            roleMention += (!roleMention.empty() ? "\n" : "");
            roleMention += "<@&" + std::to_string(roleOpt.value()) + ">";
        }
    }

    if (ctx.guild->idCompleteJobChannel.has_value())
    {
        dpp::embed announce;
        announce.set_title("Job Request Completed")
            .set_description(job->PrintJobDetails(ctx.cluster, ctx.guild->m_idGuild))
            .set_color(0x3498db);

        ctx.cluster.message_create(dpp::message(ctx.guild->idCompleteJobChannel.value_or(0), roleMention).add_embed(announce));
    }
}

void GuildSettings::WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent) const
{
    if (!xmlNode || !xmlParent)
        return;

    // Channels
    WriteOptionalId(xmlNode, "NewJobChannel", idNewJobChannel);
    WriteOptionalId(xmlNode, "UpdateJobChannel", idUpdateJobChannel);
    WriteOptionalId(xmlNode, "DeleteJobChannel", idDeleteJobChannel);
    WriteOptionalId(xmlNode, "CompleteJobChannel", idCompleteJobChannel);

    // Cooldown
    xmlNode->SetAttribute("Cooldown", static_cast<int>(announcement_cooldown.count()));

    // Ping Rules
    xmlNode->SetAttribute("PingOnNew", bPingOnNew);
    xmlNode->SetAttribute("PingOnUpdate", bPingOnUpdate);
    xmlNode->SetAttribute("PingOnDelete", bPingOnDelete);
    xmlNode->SetAttribute("PingOnComplete", bPingOnComplete);

    // Roles
    WriteOptionalId(xmlNode, "PingRole", roles[static_cast<std::size_t>(Roles::Ping)]);
    WriteOptionalId(xmlNode, "CraftRole", roles[static_cast<std::size_t>(Roles::Crafter)]);
    WriteOptionalId(xmlNode, "BuildRole", roles[static_cast<std::size_t>(Roles::Builder)]);
    WriteOptionalId(xmlNode, "ComponentRole", roles[static_cast<std::size_t>(Roles::Comp)]);
    WriteOptionalId(xmlNode, "ResourceRole", roles[static_cast<std::size_t>(Roles::Refiner)]);
    WriteOptionalId(xmlNode, "RefineRole", roles[static_cast<std::size_t>(Roles::Refiner)]);
    WriteOptionalId(xmlNode, "HazmatRole", roles[static_cast<std::size_t>(Roles::Hazmat)]);
    WriteOptionalId(xmlNode, "ManagerRole", roles[static_cast<std::size_t>(Roles::Manager)]);

    xmlParent->InsertEndChild(xmlNode);
}

void GuildSettings::ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent)
{
    if (!xmlNode)
        return;

    // Channels
    idNewJobChannel = ReadOptionalId(xmlNode, "NewJobChannel");
    idUpdateJobChannel = ReadOptionalId(xmlNode, "UpdateJobChannel");
    idDeleteJobChannel = ReadOptionalId(xmlNode, "DeleteJobChannel");
    idCompleteJobChannel = ReadOptionalId(xmlNode, "CompleteJobChannel");

    // Cooldown
    announcement_cooldown = static_cast<std::chrono::seconds>(xmlNode->UnsignedAttribute("Cooldown", 0));

    // Ping Rules
    bPingOnNew = xmlNode->BoolAttribute("PingOnNew", true);
    bPingOnUpdate = xmlNode->BoolAttribute("PingOnUpdate", false);
    bPingOnDelete = xmlNode->BoolAttribute("PingOnDelete", false);
    bPingOnComplete = xmlNode->BoolAttribute("PingOnComplete", false);

    // Roles
    roles[static_cast<std::size_t>(Roles::Ping)] = ReadOptionalId(xmlNode, "PingRole");
    roles[static_cast<std::size_t>(Roles::Crafter)] = ReadOptionalId(xmlNode, "CraftRole");
    roles[static_cast<std::size_t>(Roles::Builder)] = ReadOptionalId(xmlNode, "BuildRole");
    roles[static_cast<std::size_t>(Roles::Comp)] = ReadOptionalId(xmlNode, "ComponentRole");
    roles[static_cast<std::size_t>(Roles::Gatherer)] = ReadOptionalId(xmlNode, "ResourceRole");
    roles[static_cast<std::size_t>(Roles::Refiner)] = ReadOptionalId(xmlNode, "RefineRole");
    roles[static_cast<std::size_t>(Roles::Hazmat)] = ReadOptionalId(xmlNode, "HazmatRole");
    roles[static_cast<std::size_t>(Roles::Manager)] = ReadOptionalId(xmlNode, "ManagerRole");
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
bsoncxx::builder::basic::document GuildSettings::WriteAttributesBSON() const
{
    using namespace bsoncxx::builder::basic;

    document doc{};

    doc.append(
        kvp("_id", static_cast<std::int64_t>(m_idGuild)),
        // Channel IDs
        kvp("NewJobChannel", static_cast<std::int64_t>(idNewJobChannel.value_or(0))),
        kvp("UpdateJobChannel", static_cast<std::int64_t>(idUpdateJobChannel.value_or(0))),
        kvp("DeleteJobChannel", static_cast<std::int64_t>(idDeleteJobChannel.value_or(0))),
        kvp("CompleteJobChannel", static_cast<std::int64_t>(idCompleteJobChannel.value_or(0))),
        // Cooldown
        kvp("Cooldown", static_cast<std::int64_t>(announcement_cooldown.count())),
        // Max Requests Per User
        kvp("MaxRequestsPerUser", static_cast<std::int32_t>(requestLimitPerUser)),
        // Archival Threshold Age
        kvp("ArchiveThreshold", static_cast<std::int64_t>(archival_age.count())),
        // Stalled Threshold Age
        kvp("StalledThreshold", static_cast<std::int64_t>(stalled_age.count())),
        // New Job Counter
        kvp("JobCounter", [&]() {
            bsoncxx::builder::basic::array a;
            for (const auto& c : g_counter)
                a.append(c.load());
            return a;
            }()),
        // Ping Rules
        kvp("PingOnNew", bPingOnNew),
        kvp("PingOnUpdate", bPingOnUpdate),
        kvp("PingOnDelete", bPingOnDelete),
        kvp("PingOnComplete", bPingOnComplete),
        // Roles
        kvp("PingRole", static_cast<std::int64_t>(roles[static_cast<std::size_t>(Roles::Ping)].value_or(0))),
        kvp("CraftRole", static_cast<std::int64_t>(roles[static_cast<std::size_t>(Roles::Crafter)].value_or(0))),
        kvp("BuildRole", static_cast<std::int64_t>(roles[static_cast<std::size_t>(Roles::Builder)].value_or(0))),
        kvp("ComponentRole", static_cast<std::int64_t>(roles[static_cast<std::size_t>(Roles::Comp)].value_or(0))),
        kvp("ResourceRole", static_cast<std::int64_t>(roles[static_cast<std::size_t>(Roles::Gatherer)].value_or(0))),
        kvp("RefineRole", static_cast<std::int64_t>(roles[static_cast<std::size_t>(Roles::Refiner)].value_or(0))),
        kvp("HazmatRole", static_cast<std::int64_t>(roles[static_cast<std::size_t>(Roles::Hazmat)].value_or(0))),
        kvp("ManagerRole", static_cast<std::int64_t>(roles[static_cast<std::size_t>(Roles::Manager)].value_or(0)))
    );

    return doc;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void GuildSettings::ReadAttributesBSON(const bsoncxx::document::view& doc)
{
    // _id
    if (auto elem = doc["_id"]; elem && elem.type() == bsoncxx::type::k_int64)
        m_idGuild = static_cast<std::uint64_t>(elem.get_int64().value);

    // Channel IDs
    if (auto elem = doc["NewJobChannel"]; elem && elem.type() != bsoncxx::type::k_null)
        idNewJobChannel = ReadOptionalId(elem);

    if (auto elem = doc["UpdateJobChannel"]; elem && elem.type() != bsoncxx::type::k_null)
        idUpdateJobChannel = ReadOptionalId(elem);

    if (auto elem = doc["DeleteJobChannel"]; elem && elem.type() != bsoncxx::type::k_null)
        idDeleteJobChannel = ReadOptionalId(elem);

    if (auto elem = doc["CompleteJobChannel"]; elem && elem.type() != bsoncxx::type::k_null)
        idCompleteJobChannel = ReadOptionalId(elem);

    // Cooldown
    if (auto elem = doc["Cooldown"]; elem && elem.type() == bsoncxx::type::k_int64)
        announcement_cooldown = std::chrono::seconds(elem.get_int64().value);

    // Max Requests Per User
    if (auto elem = doc["MaxRequestsPerUser"]; elem && elem.type() == bsoncxx::type::k_int32)
        requestLimitPerUser = static_cast<std::uint32_t>(elem.get_int32().value);

    // Archival Threshold Age
    if (auto elem = doc["ArchiveThreshold"]; elem && elem.type() == bsoncxx::type::k_int64)
        archival_age = std::chrono::hours(elem.get_int64().value);

    // Stalled Threshold Age
    if (auto elem = doc["StalledThreshold"]; elem && elem.type() == bsoncxx::type::k_int64)
        stalled_age = std::chrono::hours(elem.get_int64().value);

    // New Job Counter
    if (auto elem = doc["JobCounter"]; elem && elem.type() == bsoncxx::type::k_array)
    {
        auto array = elem.get_array().value;
        size_t index = 0;

        for (auto&& val : array)
        {
            if (index >= g_counter.size()) break;

            if (val.type() == bsoncxx::type::k_int32)
                g_counter[index].store(val.get_int32().value);
            else
                g_counter[index].store(0);

            ++index;
        }
    }

    // Ping Rules
    if (auto elem = doc["PingOnNew"]; elem && elem.type() == bsoncxx::type::k_bool)
        bPingOnNew = elem.get_bool().value;

    if (auto elem = doc["PingOnUpdate"]; elem && elem.type() == bsoncxx::type::k_bool)
        bPingOnUpdate = elem.get_bool().value;

    if (auto elem = doc["PingOnDelete"]; elem && elem.type() == bsoncxx::type::k_bool)
        bPingOnDelete = elem.get_bool().value;

    if (auto elem = doc["PingOnComplete"]; elem && elem.type() == bsoncxx::type::k_bool)
        bPingOnComplete = elem.get_bool().value;

    // Roles
    if (auto elem = doc["PingRole"]; elem && elem.type() != bsoncxx::type::k_null)
        roles[static_cast<std::size_t>(Roles::Ping)] = ReadOptionalId(elem);

    if (auto elem = doc["CraftRole"]; elem && elem.type() != bsoncxx::type::k_null)
        roles[static_cast<std::size_t>(Roles::Crafter)] = ReadOptionalId(elem);

    if (auto elem = doc["BuildRole"]; elem && elem.type() != bsoncxx::type::k_null)
        roles[static_cast<std::size_t>(Roles::Builder)] = ReadOptionalId(elem);

    if (auto elem = doc["ComponentRole"]; elem && elem.type() != bsoncxx::type::k_null)
        roles[static_cast<std::size_t>(Roles::Comp)] = ReadOptionalId(elem);

    if (auto elem = doc["ResourceRole"]; elem && elem.type() != bsoncxx::type::k_null)
        roles[static_cast<std::size_t>(Roles::Gatherer)] = ReadOptionalId(elem);

    if (auto elem = doc["RefineRole"]; elem && elem.type() != bsoncxx::type::k_null)
        roles[static_cast<std::size_t>(Roles::Refiner)] = ReadOptionalId(elem);

    if (auto elem = doc["HazmatRole"]; elem && elem.type() != bsoncxx::type::k_null)
        roles[static_cast<std::size_t>(Roles::Hazmat)] = ReadOptionalId(elem);

    if (auto elem = doc["ManagerRole"]; elem && elem.type() != bsoncxx::type::k_null)
        roles[static_cast<std::size_t>(Roles::Manager)] = ReadOptionalId(elem);
}

void GuildSettings::SaveGuildSettings(std::shared_ptr<IJobRepo> repo)
{
    auto copy = shared_from_this();
    repo->UpdateGuild(m_idGuild, copy);
}