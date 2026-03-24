#include "Commands.h"

#include "AdminConfigDialog.h"
#include "GuildSettings.h"
#include "PermissionsMgr.h"
#include "BotUtility.h"
// fmt
#include <fmt/format.h>
// std library
#include <string>
#include <algorithm>
#include <cctype>

namespace
{
    std::string CreateReply(CommandContext& ctx)
    {
        // Role display names in enum order
        constexpr std::array<const char*, 8> roleNames{
            "General",
            "Crafter",
            "Builder",
            "Component",
            "Gatherer",
            "Refiner",
            "Hazmat",
            "Manager"
        };

        std::string roleList;
        for (std::size_t i = 0; i < ctx.guild->roles.size(); ++i)
        {
            roleList += fmt::format(
                "**{}:** {}\n",
                roleNames[i],
                ctx.guild->roles[i].has_value()
                ? fmt::format("<@&{}>", ctx.guild->roles[i].value())
                : "*Not Set*"
            );
        }

        std::string response = fmt::format(
            "**Current Role Configuration**\n"
            "{}",
            roleList
        );

        return response;
    }
}

void AdminResetRolesCommand::ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Type));

    const auto pManager = PermissionsMgr::GetInstance();
    if (!pManager->CanAccessAdminPanel(event, author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) && !ctx.debug)
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("USER '{}' was DENIED access to use '{}' command", author.global_name, event.command.get_command_name()));
        return;
    }

    if (strCmdID == Option_GeneralRole)
    {
        ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Ping)] = std::nullopt;
    }
    else if (strCmdID == Option_CraftingRole)
    {
        ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Crafter)] = std::nullopt;
    }
    else if (strCmdID == Option_BuildingRole)
    {
        ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Builder)] = std::nullopt;
    }
    else if (strCmdID == Option_CompDealerRole)
    {
        ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Comp)] = std::nullopt;
    }
    else if (strCmdID == Option_ResourceRole)
    {
        ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Gatherer)] = std::nullopt;
    }
    else if (strCmdID == Option_RefiningRole)
    {
        ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Refiner)] = std::nullopt;
    }
    else if (strCmdID == Option_HazmatRole)
    {
        ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Hazmat)] = std::nullopt;
    }
    else if (strCmdID == Option_ManagerRole)
    {
        ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Manager)] = std::nullopt;
    }
    else
    {
        event.reply(dpp::message("Unknown option.").set_flags(dpp::m_ephemeral));
        return;
    }

    ctx.guild->SaveGuildSettings(ctx.repo);

    event.reply(dpp::message(CreateReply(ctx)).set_flags(dpp::m_ephemeral));
}