#include "Commands.h"

#include "BotUtility.h"
#include "GuildSettings.h"
#include "PermissionsMgr.h"
// fmt
#include <fmt/format.h>
// std library
#include <string>

void AdminResetChannelsCommand::ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event)
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

    if (strCmdID == Option_NewRequest)
    {
        ctx.guild->idNewJobChannel = std::nullopt;
    }
    else if (strCmdID == Option_EditRequest)
    {
        ctx.guild->idUpdateJobChannel = std::nullopt;
    }
    else if (strCmdID == Option_DeleteRequest)
    {
        ctx.guild->idDeleteJobChannel = std::nullopt;
    }
    else if (strCmdID == Option_CompleteRequest)
    {
        ctx.guild->idCompleteJobChannel = std::nullopt;
    }
    else
    {
        event.reply(dpp::message("Unknown option.").set_flags(dpp::m_ephemeral));
        return;
    }

    ctx.guild->SaveGuildSettings(ctx.repo);

    auto FormatChannel = [](const std::optional<dpp::snowflake>& id)
        {
            return id.has_value()
                ? fmt::format("<#{}>", *id)
                : std::string("*Not Set*");
        };

    const std::string settings = fmt::format(
        "**New Job:** {}\n"
        "**Update Job:** {}\n"
        "**Delete Job:** {}\n"
        "**Complete Job:** {}\n\n",
        FormatChannel(ctx.guild->idNewJobChannel),
        FormatChannel(ctx.guild->idUpdateJobChannel),
        FormatChannel(ctx.guild->idDeleteJobChannel),
        FormatChannel(ctx.guild->idCompleteJobChannel));

    dpp::embed embed;
    embed.set_title("Channel Announcement Settings")
        .set_description(settings)
        .set_color(0x3498db);

    event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
}