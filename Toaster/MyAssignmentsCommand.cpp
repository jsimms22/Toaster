#include "Commands.h"
#include "JobQueue.h"

void MyAssignmentsCommand::ExecuteCommand(CommandContext& ctx,  const dpp::slashcommand_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    const std::string header = event.command.get_command_name() == Command_MyTopAssignment ? "Top Assigment (by priority):" : "Your Assignments:";

    if (ctx.queue->GetQueueSize() == 0)
    {
        dpp::embed embed;
        embed.set_title(header)
            .set_description("No requests or jobs currently assigned to you.")
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

        return;
    }

    const std::string result = ctx.queue->PrintQueueByWorker(ctx.cluster, author.id);
    dpp::embed embed;
    embed.set_title(header)
        .set_description(!result.empty() ? result : "No requests or jobs currently assigned to you.")
        .set_color(0x3498db);

    event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
}

void MyAssignmentsCommand::ExecuteInteraction(CommandContext& ctx,  const dpp::interaction_create_t& event)
{
}

void MyAssignmentsCommand::ExecuteFormSubmit(CommandContext& ctx,  const dpp::form_submit_t& event)
{
}