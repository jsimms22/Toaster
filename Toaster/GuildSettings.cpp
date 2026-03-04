#include "GuildSettings.h"

#include "CommandContext.h"
#include "Resource.h"
// d++
#include <dpp/message.h>
// fmt
#include <fmt/format.h>

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
}

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
void GuildSettings::AnnounceOnNew(CommandContext& ctx, const std::size_t jobType, const std::string& jobDetails)
{
    std::string roleMention;
    if (ctx.guild.bPingOnNew)
    {
        if (const auto roleOpt = GuildSettings::JobTypeToRole(jobType); roleOpt.has_value())
        {
            const auto roleEnum = *roleOpt;
            const auto roleSnowflake = ctx.guild.roles[static_cast<size_t>(roleEnum)];

            if (roleSnowflake.has_value())
            {
                roleMention = "<@&" + std::to_string(*roleSnowflake) + ">";
            }
        }

        if (const auto roleOpt = ctx.guild.roles[static_cast<size_t>(GuildSettings::Roles::Ping)]; roleOpt.has_value())
        {
            roleMention += (!roleMention.empty() ? "\n" : "");
            roleMention += "<@&" + std::to_string(roleOpt.value()) + ">";
        }
    }

    if (ctx.guild.idNewJobChannel.has_value())
    {
        dpp::embed announce;
        announce.set_title("New Job Request")
            .set_description(jobDetails)
            .set_color(0x3498db);

        auto ExtractJobID = [](const std::string& text) -> std::optional<std::string>
            {
                constexpr std::string_view key = "**ID**: ";

                const std::size_t pos = text.find(key);
                if (pos == std::string::npos)
                    return std::nullopt;

                const std::size_t start = pos + key.size();
                const std::size_t end = text.find('\n', start);

                if (end == std::string::npos)
                    return std::nullopt;

                return text.substr(start, end - start);
            };

        dpp::component row;
        row.add_component(dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Assign Me")
            .set_style(dpp::cos_success)
            .set_id(fmt::format("global_assign:{}:{}", jobType, ExtractJobID(jobDetails).value_or("unknown"))));

        ctx.cluster.message_create(dpp::message(ctx.guild.idNewJobChannel.value_or(0), roleMention).add_embed(announce).add_component(row));
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void GuildSettings::AnnounceOnUpdate(CommandContext& ctx, const std::size_t jobType, const std::string& jobDetails)
{
    std::string roleMention;
    if (ctx.guild.bPingOnUpdate)
    {
        if (const auto roleOpt = GuildSettings::JobTypeToRole(jobType); roleOpt.has_value())
        {
            const auto roleEnum = *roleOpt;
            const auto roleSnowflake = ctx.guild.roles[static_cast<size_t>(roleEnum)];

            if (roleSnowflake.has_value())
            {
                roleMention = "<@&" + std::to_string(*roleSnowflake) + ">";
            }
        }

        if (const auto roleOpt = ctx.guild.roles[static_cast<size_t>(GuildSettings::Roles::Ping)]; roleOpt.has_value())
        {
            roleMention += (!roleMention.empty() ? "\n" : "");
            roleMention += "<@&" + std::to_string(roleOpt.value()) + ">";
        }
    }
    
    if (ctx.guild.idUpdateJobChannel.has_value())
    {
        dpp::embed announce;
        announce.set_title("Job Request Edited")
            .set_description(jobDetails)
            .set_color(0x3498db);

        ctx.cluster.message_create(dpp::message(ctx.guild.idUpdateJobChannel.value_or(0), roleMention).add_embed(announce));
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void GuildSettings::AnnounceOnDelete(CommandContext& ctx, const std::size_t jobType, const std::string& jobDetails)
{
    std::string roleMention;
    if (ctx.guild.bPingOnDelete)
    {
        if (const auto roleOpt = GuildSettings::JobTypeToRole(jobType); roleOpt.has_value())
        {
            const auto roleEnum = *roleOpt;
            const auto roleSnowflake = ctx.guild.roles[static_cast<size_t>(roleEnum)];

            if (roleSnowflake.has_value())
            {
                roleMention = "<@&" + std::to_string(*roleSnowflake) + ">";
            }
        }

        if (const auto roleOpt = ctx.guild.roles[static_cast<size_t>(GuildSettings::Roles::Ping)]; roleOpt.has_value())
        {
            roleMention += (!roleMention.empty() ? "\n" : "");
            roleMention += "<@&" + std::to_string(roleOpt.value()) + ">";
        }
    }

    if (ctx.guild.idDeleteJobChannel.has_value())
    {
        dpp::embed announce;
        announce.set_title("Job Request Deleted")
            .set_description(jobDetails)
            .set_color(0x3498db);

        ctx.cluster.message_create(dpp::message(ctx.guild.idDeleteJobChannel.value_or(0), roleMention).add_embed(announce));
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void GuildSettings::AnnounceOnComplete(CommandContext& ctx, const std::size_t jobType, const std::string& jobDetails)
{
    std::string roleMention;
    if (ctx.guild.bPingOnComplete)
    {
        if (const auto roleOpt = GuildSettings::JobTypeToRole(jobType); roleOpt.has_value())
        {
            const auto roleEnum = *roleOpt;
            const auto roleSnowflake = ctx.guild.roles[static_cast<size_t>(roleEnum)];

            if (roleSnowflake.has_value())
            {
                roleMention = "<@&" + std::to_string(roleSnowflake.value()) + ">";
            }
        }

        if (const auto roleOpt = ctx.guild.roles[static_cast<size_t>(GuildSettings::Roles::Ping)]; roleOpt.has_value())
        {
            roleMention += (!roleMention.empty() ? "\n" : "");
            roleMention += "<@&" + std::to_string(roleOpt.value()) + ">";
        }
    }

    if (ctx.guild.idCompleteJobChannel.has_value())
    {
        dpp::embed announce;
        announce.set_title("Job Request Completed")
            .set_description(jobDetails)
            .set_color(0x3498db);

        ctx.cluster.message_create(dpp::message(ctx.guild.idCompleteJobChannel.value_or(0), roleMention).add_embed(announce));
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

void GuildSettings::SaveGuildSettings(const dpp::snowflake guildID)
{
    tinyxml2::XMLDocument doc;
    doc.LoadFile("../guilds.xml");

    // Get the root element (<GuidlList>)
    tinyxml2::XMLElement* root = doc.RootElement();
    if (!root)
    {
        root = doc.NewElement("GuidlList");
        doc.InsertEndChild(root);
    }

    tinyxml2::XMLElement* guildNode = nullptr;

    // Search for existing Guild node with matching ID
    for (auto* xmlNode = root->FirstChildElement("Guild");
        xmlNode != nullptr;
        xmlNode = xmlNode->NextSiblingElement("Guild"))
    {
        const dpp::snowflake id = xmlNode->Unsigned64Attribute("ID", 0);
        if (id == guildID)
        {
            guildNode = xmlNode;
            break;
        }
    }

    // If not found, create new node
    if (!guildNode)
    {
        guildNode = doc.NewElement("Guild");
        guildNode->SetAttribute("ID", guildID);
        root->InsertEndChild(guildNode);
    }

    // Re-set ID (since we cleared attributes)
    guildNode->SetAttribute("ID", std::to_string(guildID).c_str());

    // Write settings
    WriteAttributes(guildNode, root);

    // Save the updated XML to a file
    doc.SaveFile("../guilds.xml");
}