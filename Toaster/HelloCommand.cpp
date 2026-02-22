#include "Commands.h"
// fmt
#include <fmt/format.h>

void HelloCommand::ExecuteCommand(CommandContext& ctx,  const dpp::slashcommand_t& event)
{
    if (event.command.get_command_name() == this->name)
    {
        const dpp::user author = event.command.get_issuing_user();
        // Reply to the user, but only let them see the response. 
        event.reply(dpp::message("Hello! How are you today?").set_flags(dpp::m_ephemeral));
    }
}

void HelloCommand::ExecuteInteraction(CommandContext& ctx,  const dpp::interaction_create_t& event)
{
}

void HelloCommand::ExecuteFormSubmit(CommandContext& ctx,  const dpp::form_submit_t& event)
{
}