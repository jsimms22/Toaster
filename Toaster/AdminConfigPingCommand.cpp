#include "Commands.h"

#include "AdminConfigDialog.h"
#include "GuildSettings.h"
#include "PermissionsMgr.h"
#include "BotUtility.h"
// fmt
#include <fmt/format.h>

void AdminConfigPingCommand::ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Type));
    const bool bPingRuleSetting = std::get<bool>(event.get_parameter(Parameter_Bool));

    const auto pManager = PermissionsMgr::GetInstance();
    if (!pManager->CanAccessAdminPanel(event, author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) && !ctx.debug)
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("USER '{}' was DENIED access to use '{}' command", author.id, event.command.get_command_name()));
        return;
    }

    if (strCmdID == Option_NewRequest)
    {
        ctx.guild->bPingOnNew = bPingRuleSetting;
    }
    else if (strCmdID == Option_EditRequest)
    {
        ctx.guild->bPingOnUpdate = bPingRuleSetting;
    }
    else if (strCmdID == Option_DeleteRequest)
    {
        ctx.guild->bPingOnDelete = bPingRuleSetting;
    }
    else if (strCmdID == Option_CompleteRequest)
    {
        ctx.guild->bPingOnComplete = bPingRuleSetting;
    }
    else
    {
        event.reply(dpp::message("Unknown option.").set_flags(dpp::m_ephemeral));
        return;
    }

    ctx.guild->SaveGuildSettings(ctx.repo);

    auto FormatRole = [](const std::optional<dpp::snowflake>& id)
        {
            return id.has_value()
                ? fmt::format("<@&{}>", *id)
                : std::string("*Not Set*");
        };

    const std::string settings = fmt::format(
        "**On New:** {}\n"
        "**On Update:** {}\n"
        "**On Delete:** {}\n"
        "**On Complete:** {}\n",
        ctx.guild->bPingOnNew ? "Enabled" : "Disabled",
        ctx.guild->bPingOnUpdate ? "Enabled" : "Disabled",
        ctx.guild->bPingOnDelete ? "Enabled" : "Disabled",
        ctx.guild->bPingOnComplete ? "Enabled" : "Disabled");

    dpp::embed embed;
    embed.set_title("Ping Rule Settings")
        .set_description(settings)
        .set_color(0x3498db);

    event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
}

void AdminConfigPingCommand::ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event)
{
}

void AdminConfigPingCommand::ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event)
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
        std::string strParam3 = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";
        std::string strParam4 = event.components.size() > 3 ? std::get<std::string>(event.components[3].value) : "";
        std::string strParam5 = event.components.size() > 4 ? std::get<std::string>(event.components[4].value) : "";

        auto is_all_numbers = [](const std::string& s) -> bool {
            return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) {
                return std::isdigit(c);
                });
            };

        utils::FilterWhiteSpace(strParam1);

        if (!strParam1.empty() && !is_all_numbers(strParam1))
        {
            event.reply(dpp::message("Invalid input for the role id. Must be all numeric characters.").set_flags(dpp::m_ephemeral));
            return;
        }

        bool bRoleExists = false;
        dpp::snowflake newID{ strParam1 };
        dpp::guild* guild = utils::FindGuildByID(ctx.cluster, event.command.guild_id);
        for (const auto& roleID : guild->roles)
        {
            if (newID == roleID)
            {
                bRoleExists = true;
                break;
            }
        }

        if (!strParam1.empty() && bRoleExists)
        {
            ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Ping)] = newID;
        }
        else
        {
            ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Ping)] = std::nullopt;
        }

        if (!strParam2.empty())
        {
            ctx.guild->bPingOnNew = strParam2 == "1" ? true : false;
        }

        if (!strParam3.empty())
        {
            ctx.guild->bPingOnUpdate = strParam3 == "1" ? true : false;
        }

        if (!strParam4.empty())
        {
            ctx.guild->bPingOnDelete = strParam4 == "1" ? true : false;
        }

        if (!strParam5.empty())
        {
            ctx.guild->bPingOnComplete = strParam5 == "1" ? true : false;
        }

        ctx.guild->SaveGuildSettings(ctx.repo);

        auto FormatRole = [](const std::optional<dpp::snowflake>& id)
            {
                return id.has_value()
                    ? fmt::format("<@&{}>", *id)
                    : std::string("*Not Set*");
            };

        const std::string settings = fmt::format(
            "**General Ping Role:** {}\n"
            "**On New:** {}\n"
            "**On Update:** {}\n"
            "**On Delete:** {}\n"
            "**On Complete:** {}\n\n",
            FormatRole(ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Ping)]),
            ctx.guild->bPingOnNew ? "Enabled" : "Disabled",
            ctx.guild->bPingOnUpdate ? "Enabled" : "Disabled",
            ctx.guild->bPingOnDelete ? "Enabled" : "Disabled",
            ctx.guild->bPingOnComplete ? "Enabled" : "Disabled");

        dpp::embed embed;
        embed.set_title("Ping Rule Settings")
            .set_description(settings)
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
    }
}

void AdminConfigPingCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
}