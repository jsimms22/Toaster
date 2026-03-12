//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief Implementation of the ShowQueueSummaryCommand.
///
/// Contains the implementation of ShowQueueSummaryCommand declared in Commands.h.
//---------------------------------------------------------------------------------------------------------------------
#include "Commands.h"

#include "BotUtility.h"
#include "JobRequest.h"
#include "JobQueue.h"
#include "PaginationPanel.h"
#include "PermissionsMgr.h"
// fmt
#include <fmt/format.h>
// std library
#include <algorithm>

//---------------------------------------------------------------------------------------------------------------------
/// \brief Event handler for create interaction processes.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
//---------------------------------------------------------------------------------------------------------------------
void ShowQueueSummaryCommand::ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    const auto pManager = PermissionsMgr::GetInstance();
    if (!(pManager->IsWorker(author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) ||
          pManager->IsManager(author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) ||
          pManager->IsGuildAdmin(author.id, event) ||
          pManager->IsBotOwner(author.id)) && !ctx.debug)
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("USER '{}' was DENIED access to use '{}' command", author.id, event.command.get_command_name()));
        return;
    }

    const std::string header = "Queue Summary:";
    if (ctx.queue->GetQueueSize() == 0)
    {
        dpp::embed embed;
        embed.set_title(header)
            .set_description("Request queue is currently empty.")
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

        return;
    }

    const std::string result = ctx.queue->PrintQueueSummary(ctx.cluster);
    dpp::embed embed;
    embed.set_title(header)
        .set_description(result)
        .set_color(0x3498db);

    dpp::message msg;
    msg.add_embed(embed).set_flags(dpp::m_ephemeral);
    if (!result.empty())
    {
        dpp::component button1 = dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Show Stalled Jobs")
            .set_style(dpp::cos_primary)
            .set_id(Button_Stalled);

        dpp::component button2 = dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Show Unassigned Jobs")
            .set_style(dpp::cos_primary)
            .set_id(Button_Unassigned);

        dpp::component row = dpp::component()
            .set_type(dpp::cot_action_row)
            .add_component(button1)
            .add_component(button2);

        msg.add_component(row);
    }

    event.reply(msg);
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Event handler for button interactions.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
//---------------------------------------------------------------------------------------------------------------------
void ShowQueueSummaryCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
    if (!ctx.queue)
    {
        return;
    }

    if (event.custom_id == Button_Stalled)
    {
        // Return early if queue is empty
        if (ctx.queue->GetQueueSize() == 0)
        {
            dpp::embed embed;
            embed.set_title("Stalled Jobs Report:")
                .set_description("Request queue is currently empty.")
                .set_color(0x3498db);

            event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

            return;
        }

        // Acknowledge immediately to avoid timing out
        event.reply(dpp::message("...this could take a second. Please hold.").set_flags(dpp::m_ephemeral));

        // Construct the actual info panel
        const std::size_t page = 0;
        const std::string result = ctx.queue->PrintPagedQueue(ctx.cluster, 
                                                              event.command.guild_id, 
                                                              page,
                                                              [](const std::shared_ptr<const JobRequest> job) -> bool 
                                                              { return job->GetStatus() == JobRequest::status::stalled; });
        const std::size_t size = ctx.queue->GetQueueSize([](const std::shared_ptr<const JobRequest> job) -> bool 
                                                        { return job->GetStatus() == JobRequest::status::stalled; });
        const std::string header = "Stalled Job Report";

        PaginationPanel panel(ctx, Button_Stalled, static_cast<int>(JobRequest::status::stalled), page, size, JobQueue::JOBS_PER_DETAIL_PAGE);
        panel.AddEmbed(header, size != 0 ? result : "No stalled jobs in queue.");

        // Edit the original message
        event.edit_original_response(panel.set_flags(dpp::m_ephemeral));
        return;
    }
    else if (event.custom_id == Button_Unassigned)
    {
        // Return early if queue is empty
        if (ctx.queue->GetQueueSize() == 0)
        {
            dpp::embed embed;
            embed.set_title("Unassigned Jobs Report:")
                .set_description("Request queue is currently empty.")
                .set_color(0x3498db);

            event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

            return;
        }

        // Acknowledge immediately to avoid timing out
        event.reply(dpp::message("...this could take a second. Please hold.").set_flags(dpp::m_ephemeral));

        // Construct the actual info panel
        const std::size_t page = 0;
        const std::string result = ctx.queue->PrintPagedQueue(ctx.cluster,
                                                              event.command.guild_id,
                                                              page,
                                                              [](const std::shared_ptr<const JobRequest> job) -> bool
                                                              { return job->GetWorkerIDs().empty() && job->GetStatus() != JobRequest::status::complete; });
        const std::size_t size = ctx.queue->GetQueueSize([](const std::shared_ptr<const JobRequest> job) -> bool
                                                         { return job->GetWorkerIDs().empty() && job->GetStatus() != JobRequest::status::complete; });
        const std::string header = "Unassigned Job Report";

        PaginationPanel panel(ctx, Button_Unassigned, 0, page, size, JobQueue::JOBS_PER_DETAIL_PAGE);
        panel.AddEmbed(header, size != 0 ? result : "No unassigned jobs in queue.");

        // Edit the original message
        event.edit_original_response(panel.set_flags(dpp::m_ephemeral));
        return;
    }

    const std::string id = event.custom_id; // "stalled:JobRequest::status:page"

    if (id.starts_with(fmt::format("{}:", Button_Stalled)))
    {
        // Deconstruct custom id for state information
        auto parts = utils::Split(id, ':');
        const JobRequest::status type = static_cast<JobRequest::status>(std::stoi(parts[1]));
        if (type != JobRequest::status::stalled) { return; }
        std::size_t page = parts[2] != std::to_string(std::numeric_limits<std::size_t>::max()) ? std::stoul(parts[2]) : 0;

        // Construct the actual info panel
        const std::string result = ctx.queue->PrintPagedQueue(ctx.cluster,
                                                              event.command.guild_id,
                                                              page,
                                                              [](const std::shared_ptr<const JobRequest> job) -> bool
                                                              { return job->GetStatus() == JobRequest::status::stalled; });
        const std::size_t size = ctx.queue->GetQueueSize([](const std::shared_ptr<const JobRequest> job) -> bool 
                                                        { return job->GetStatus() == JobRequest::status::stalled; });
        const std::string header = "Stalled Job Report";

        PaginationPanel panel(ctx, Button_Stalled, static_cast<int>(JobRequest::status::stalled), page, size, JobQueue::JOBS_PER_DETAIL_PAGE);
        panel.AddEmbed(header, size != 0 ? result : "No stalled jobs in queue.");

        // Edit the original message
        event.reply(dpp::ir_update_message, panel.set_flags(dpp::m_ephemeral));
        return;
    }
    else if (id.starts_with(fmt::format("{}:", Button_Unassigned)))
    {
        // Deconstruct custom id for state information
        auto parts = utils::Split(id, ':');
        const dpp::snowflake worker = 0;
        if (worker != 0) { return; }
        std::size_t page = parts[2] != std::to_string(std::numeric_limits<std::size_t>::max()) ? std::stoul(parts[2]) : 0;

        // Construct the actual info panel
        const std::string result = ctx.queue->PrintPagedQueue(ctx.cluster,
                                                              event.command.guild_id,
                                                              page,
                                                              [](const std::shared_ptr<const JobRequest> job) -> bool
                                                              { return job->GetWorkerIDs().empty(); });
        const std::size_t size = ctx.queue->GetQueueSize([](const std::shared_ptr<const JobRequest> job) -> bool
                                                         { return job->GetWorkerIDs().empty() && job->GetStatus() != JobRequest::status::complete; });
        const std::string header = "Unassigned Job Report";

        PaginationPanel panel(ctx, Button_Unassigned, 0, page, size, JobQueue::JOBS_PER_DETAIL_PAGE);
        panel.AddEmbed(header, size != 0 ? result : "No unassigned jobs in queue.");

        // Edit the original message
        event.reply(dpp::ir_update_message, panel.set_flags(dpp::m_ephemeral));
        return;
    }
}