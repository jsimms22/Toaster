#include "Commands.h"

#include "BotUtility.h"
// fmt
#include <fmt/format.h>

void AdminBotConfigCommand::ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Type));

    const auto pManager = PermissionsMgr::GetInstance();
    if (!(pManager->IsGuildAdmin(author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id)) ||
          pManager->IsBotOwner(author.id)) && !ctx.debug)
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("USER '{}' was DENIED access to use '{}' command", author.id, event.command.get_command_name()));
        return;
    }
}

void AdminBotConfigCommand::ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) 
{
}

void AdminBotConfigCommand::ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) 
{
}

void AdminBotConfigCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) 
{
}