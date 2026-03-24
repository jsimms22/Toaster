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
    std::string CreateReply(CommandContext& ctx, const std::optional<dpp::snowflake>& selectedRole)
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

void AdminConfigRolesCommand::ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Type));
    std::string strRoleID = std::get<std::string>(event.get_parameter(Parameter_Role));

    const auto pManager = PermissionsMgr::GetInstance();
    if (!pManager->CanAccessAdminPanel(event, author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) && !ctx.debug)
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("USER '{}' was DENIED access to use '{}' command", author.global_name, event.command.get_command_name()));
        return;
    }

    auto is_all_numbers = [](const std::string& s) -> bool {
        return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) {
            return std::isdigit(c);
            });
        };

    utils::FilterWhiteSpace(strRoleID);
    utils::FilterCharacters(strRoleID);

    if (!is_all_numbers(strRoleID) && !strRoleID.empty())
    {
        event.reply(dpp::message("Invalid input for the role id. Must be all numeric characters or a role ping.").set_flags(dpp::m_ephemeral));
        return;
    }

    bool bRoleExists = false;
    dpp::snowflake newID{ strRoleID };
    dpp::guild* guild = utils::FindGuildByID(ctx.cluster, event.command.guild_id);
    for (const auto& roleID : guild->roles)
    {
        if (newID == roleID)
        {
            bRoleExists = true;
            break;
        }
    }

    const std::optional<dpp::snowflake> selectedRole = !bRoleExists ? std::nullopt : std::optional<dpp::snowflake>(strRoleID);

    auto SetRole = [&ctx, &event, &selectedRole](GuildSettings::Roles roleEnum, const std::string& label)
        {
            ctx.guild->roles[static_cast<std::size_t>(roleEnum)] = selectedRole;
            event.reply(dpp::message(CreateReply(ctx, selectedRole)).set_flags(dpp::m_ephemeral));
        };

    if (strCmdID == Option_GeneralRole)
    {
        SetRole(GuildSettings::Roles::Ping, "General Worker");
    }
    else if (strCmdID == Option_CraftingRole)
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
        event.reply(dpp::message("Unknown option.").set_flags(dpp::m_ephemeral));
        return;
    }

    ctx.guild->SaveGuildSettings(ctx.repo);
}

void AdminConfigRolesCommand::ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event)
{
    const std::string id = event.custom_id;
    if (!ctx.queue || !id.starts_with(fmt::format("{}:", AdminConfigDialog::modalID)))
    {
        return;
    }

    auto parts = utils::Split(id, ':');
    const std::string strDialogOwner = parts[1];

    if (strDialogOwner == this->name)
    {
        const dpp::user author = event.command.get_issuing_user();
        // These parameters are input that depend on the dialog form's order as defined by the type of the job
        std::string strParam1 = event.components.size() > 0 ? std::get<std::string>(event.components[0].value) : "";
        std::string strParam2 = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";

        auto is_all_numbers = [](const std::string& s) -> bool {
            return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) {
                return std::isdigit(c);
                });
            };

        utils::FilterWhiteSpace(strParam1);
        utils::FilterWhiteSpace(strParam2);

        if (!is_all_numbers(strParam2) && !strParam2.empty())
        {
            event.reply(dpp::message("Invalid input for the role id. Must be all numeric characters.").set_flags(dpp::m_ephemeral));
            return;
        }

        bool bRoleExists = false;
        dpp::snowflake newID{ strParam2 };
        dpp::guild* guild = utils::FindGuildByID(ctx.cluster, event.command.guild_id);
        for (const auto& roleID : guild->roles)
        {
            if (newID == roleID)
            {
                bRoleExists = true;
                break;
            }
        }

        if (!strParam1.empty() && !strParam2.empty() && bRoleExists)
        {
            try
            {
                ctx.guild->roles[std::stoull(strParam1)] = newID;
            }
            catch (const std::exception)
            {
                event.reply(dpp::message("Failed to set new role id.").set_flags(dpp::m_ephemeral));
                return;
            }
        }
        else if (!strParam1.empty() || !bRoleExists)
        {
            try
            {
                ctx.guild->roles[std::stoull(strParam1)] = std::nullopt;
            }
            catch (const std::exception)
            {
                event.reply(dpp::message("Failed to set new role id.").set_flags(dpp::m_ephemeral));
                return;
            }
        }

        ctx.guild->SaveGuildSettings(ctx.repo);

        event.reply(dpp::message(CreateReply(ctx, dpp::snowflake(strParam2))).set_flags(dpp::m_ephemeral));
    }
}