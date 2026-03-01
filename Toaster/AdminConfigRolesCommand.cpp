#include "Commands.h"

#include "BotUtility.h"
// fmt
#include <fmt/format.h>

void AdminConfigRolesCommand::ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Type));
    std::string strRoleID = std::get<std::string>(event.get_parameter(Parameter_Id));

    const auto pManager = PermissionsMgr::GetInstance();
    if (!pManager->CanAccessAdminPanel(event, author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) && !ctx.debug)
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("USER '{}' was DENIED access to use '{}' command", author.id, event.command.get_command_name()));
        return;
    }

    utils::FilterWhiteSpace(strRoleID);
    dpp::snowflake selectedRole = std::stoull(strRoleID);

    auto SetRole = [&ctx, &event, &selectedRole](GuildSettings::Roles roleEnum, const std::string& label)
        {
            ctx.guild.roles[static_cast<std::size_t>(roleEnum)] = selectedRole;

            // Role display names in enum order
            constexpr std::array<const char*, 8> roleNames{
                "Ping",
                "Crafter",
                "Builder",
                "Component",
                "Gatherer",
                "Refiner",
                "Hazmat",
                "Manager"
            };

            std::string roleList;
            for (std::size_t i = 0; i < ctx.guild.roles.size(); ++i)
            {
                roleList += fmt::format(
                    "**{}:** {}\n",
                    roleNames[i],
                    ctx.guild.roles[i].has_value()
                    ? fmt::format("<@&{}>", ctx.guild.roles[i].value())
                    : "*Not Set*"
                );
            }

            std::string response = fmt::format(
                "** {} role updated** \n"
                "Set to: <@&{}>\n\n"
                "**Current Role Configuration**\n"
                "{}",
                label,
                selectedRole,
                roleList
            );

            event.reply(dpp::message(response).set_flags(dpp::m_ephemeral));
        };

    if (strCmdID == Option_CraftingRole)
    {
        SetRole(GuildSettings::Roles::Crafter, "Item Crafter");
    }
    else if (strCmdID == Option_BuildingRole)
    {
        SetRole(GuildSettings::Roles::Builder, "Base Builder");
    }
    else if (strCmdID == Option_CompDealerRole)
    {
        SetRole(GuildSettings::Roles::Comp, "Component Dealer");
    }
    else if (strCmdID == Option_ResourceRole)
    {
        SetRole(GuildSettings::Roles::Gatherer, "Resource Gatherer");
    }
    else if (strCmdID == Option_RefiningRole)
    {
        SetRole(GuildSettings::Roles::Refiner, "Refinery Worker");
    }
    else if (strCmdID == Option_HazmatRole)
    {
        SetRole(GuildSettings::Roles::Hazmat, "Hazardous Materials Collector");
    }
    else if (strCmdID == Option_ManagerRole)
    {
        SetRole(GuildSettings::Roles::Manager, "Manager");
    }
    else
    {
        event.reply(dpp::message("Unknown role option.").set_flags(dpp::m_ephemeral));
        return;
    }

    ctx.guild.SaveGuildSettings(event.command.guild_id);
}