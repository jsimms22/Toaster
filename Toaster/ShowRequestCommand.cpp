#include "Commands.h"
#include "JobQueue.h"
#include "BotUtility.h"
// fmt
#include <fmt/format.h>

void ShowRequestCommand::ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event)
{
}

void ShowRequestCommand::ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();

    std::string strJobID = std::get<std::string>(event.get_parameter(Parameter_Id));
    utils::FilterWhiteSpace(strJobID);

    const std::shared_ptr<JobRequest> job = ctx.queue->GetJobByGUID(strJobID);
    if (!job)
    {
        event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
        return;
    }

    if (!(ctx.manager->IsRequestOwner(author.id, job) ||
          ctx.manager->IsRequestWorker(author.id, job) ||
          ctx.manager->IsActiveWorker(author.id, ctx.workers) ||
          ctx.manager->IsGuildAdmin(author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id)) ||
          ctx.manager->IsBotOwner(author.id)))
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("USER '{}' was DENIED access to use '{}' command", author.id, event.command.get_command_name()));
        return;
    }

    dpp::embed embed;
    embed.set_title("Here is the request:")
        .set_description(job->PrintJobDetails(ctx.cluster, event.command.guild_id))
        .set_color(0x3498db);

    event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
}

void ShowRequestCommand::ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event)
{
}