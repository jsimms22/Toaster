#include "Commands.h"

#include "AdminConfigDialog.h"
#include "BotUtility.h"
// fmt
#include <fmt/format.h>
#include <string>

void AdminConfigChannelsCommand::ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();

    const auto pManager = PermissionsMgr::GetInstance();
    if (!pManager->CanAccessAdminPanel(event, author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) && !ctx.debug)
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("USER '{}' was DENIED access to use '{}' command", author.id, event.command.get_command_name()));
        return;
    }

    AdminConfigDialog modal(this->name, ctx);
    event.dialog(modal);
}

void AdminConfigChannelsCommand::ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event)
{
}

void AdminConfigChannelsCommand::ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event)
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

        if (!strParam1.empty())
        {
            utils::FilterWhiteSpace(strParam1);
            ctx.guild.idNewJobChannel = dpp::snowflake(strParam1);
        }

        if (!strParam2.empty())
        {
            utils::FilterWhiteSpace(strParam2);
            ctx.guild.idUpdateJobChannel = dpp::snowflake(strParam2);
        }

        if (!strParam3.empty())
        {
            utils::FilterWhiteSpace(strParam3);
            ctx.guild.idDeleteJobChannel = dpp::snowflake(strParam3);
        }

        if (!strParam4.empty())
        {
            utils::FilterWhiteSpace(strParam4);
            ctx.guild.idCompleteJobChannel = dpp::snowflake(strParam4);
        }

        ctx.guild.SaveGuildSettings(event.command.guild_id);

        auto FormatChannel = [](const std::optional<dpp::snowflake>& id)
            {
                return id.has_value()
                    ? fmt::format("<#{}>", *id)
                    : std::string("*Not Set*");
            };


        const std::string settings = fmt::format("**New Job:** {}\n"
                                                "**Update Job:** {}\n"
                                                "**Delete Job:** {}\n"
                                                "**Complete Job:** {}\n\n",
                                                FormatChannel(ctx.guild.idNewJobChannel),
                                                FormatChannel(ctx.guild.idUpdateJobChannel),
                                                FormatChannel(ctx.guild.idDeleteJobChannel),
                                                FormatChannel(ctx.guild.idCompleteJobChannel));

        dpp::embed embed;
        embed.set_title("Channel Announcement Settings")
            .set_description(settings)
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
    }
}

void AdminConfigChannelsCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
}