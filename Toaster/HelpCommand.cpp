#include "Commands.h"
// fmt
#include <fmt/format.h>
// std library
#include <string>

//---------------------------------------------------------------------------------------------------------------------
/// \brief Builds formatted help text listing all registered bot commands.
///
/// Iterates through the global Toaster::BotCommands list and generates a formatted string containing each slash 
/// command name and description. The output is intended for ephemeral display in Discord.
///
/// \return A formatted markdown string containing available commands.
//---------------------------------------------------------------------------------------------------------------------
std::string BuildHelpText()
{
    std::ostringstream oss;
    oss << "**Available Commands**\n\n";

    for (const auto* cmd : Toaster::BotCommands)
    {
        if (!cmd) continue;

        oss << "/"
            << cmd->name                         // slash command name
            << " - "
            << cmd->description                  // slash command description
            << "\n";
    }

    return oss.str();
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Executes the Help slash command.
///
/// Validates the command name and replies with an ephemeral message containing a dynamically generated list of 
/// available bot commands.
///
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Slash command invocation event.
//---------------------------------------------------------------------------------------------------------------------
void HelpCommand::ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event)
{
    if (event.command.get_command_name() == this->name)
    {
        event.reply(dpp::message(BuildHelpText()).set_flags(dpp::m_ephemeral));
    }
}