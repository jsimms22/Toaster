#include "BotUtility.h"
#include "Commands.h"
#include "JobQueue.h"
// fmt
#include <fmt/format.h>

void MyRequestsCommand::ExecuteCommand(CommandContext& ctx,  const dpp::slashcommand_t& event)
{
}

void MyRequestsCommand::ExecuteInteraction(CommandContext& ctx,  const dpp::interaction_create_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Type));

    const std::size_t filterType = utils::CmdStringToJobType(strCmdID);
    const std::string header = "Your Requests:";
    if (ctx.queue->GetQueueSize() == 0)
    {
        dpp::embed embed;
        embed.set_title(header)
            .set_description("Request queue is currently empty.")
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

        return;
    }

    const std::string result = ctx.queue->PrintQueueByUser(ctx.cluster, author.id, filterType);
    dpp::embed embed;
    embed.set_title(header)
        .set_description(!result.empty() ? result : "You have no requests of this type in queue.")
        .set_color(0x3498db);

    event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
}

void MyRequestsCommand::ExecuteFormSubmit(CommandContext& ctx,  const dpp::form_submit_t& event)
{
}