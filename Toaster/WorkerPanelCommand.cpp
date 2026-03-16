//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief Implementation of the WorkerPanelCommand.
///
/// Contains the implementation of WorkerPanelCommand declared in Commands.h.
//---------------------------------------------------------------------------------------------------------------------
#include "Commands.h"
#include "JobQueue.h"
#include "RequestDlg.h"
#include "BotUtility.h"
#include "PaginationPanel.h"
#include "WorkerPanel.h"
#include "PermissionsMgr.h"
// fmt
#include <fmt/format.h>

//---------------------------------------------------------------------------------------------------------------------
/// \brief Event handler for create interaction processes.
///
/// Displays either all assignments for a worker or the overview of the top-priority assignment.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
//---------------------------------------------------------------------------------------------------------------------
void WorkerPanelCommand::ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Type));

    // Check Permissions
    const auto pManager = PermissionsMgr::GetInstance();
    if (!(pManager->IsWorker(author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) ||
        pManager->IsGuildAdmin(author.id, event) ||
        pManager->IsBotOwner(author.id)) && !ctx.debug)
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("USER '{}' was DENIED access to use '{}' command", author.id, event.command.get_command_name()));
        return;
    }

    if (strCmdID == Option_AllAssignments)
    {
        // Return early if queue is empty
        if (ctx.queue->GetQueueSize() == 0)
        {
            dpp::embed embed;
            embed.set_title("Your Assignments")
                .set_description("No requests or jobs currently assigned to you.")
                .set_color(0x3498db);

            event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

            return;
        }

        // Acknowledge immediately to avoid timing out
        event.reply(dpp::message("...this could take a second. Please hold.").set_flags(dpp::m_ephemeral));

        // Construct the actual info panel
        const std::size_t page = 0;
        const bool bShowComplete = true;

        auto compare = [worker = author.id](const std::shared_ptr<const JobRequest> job) -> bool { return job->IsWorker(worker); };
        const std::string result = ctx.queue->PrintPagedQueue(ctx.cluster, event.command.guild_id, page, bShowComplete, compare);
        const std::size_t size = ctx.queue->GetQueueSize(bShowComplete, compare);
        const std::string header = "Your Assignments";

        PaginationPanel panel(ctx, Option_AllAssignments, 0, page, size, JobQueue::JOBS_PER_DETAIL_PAGE, bShowComplete, author.id);
        panel.AddEmbed(header, size != 0 ? result : "No requests or jobs currently assigned to you.");

        // Edit the original message
        event.edit_original_response(panel.set_flags(dpp::m_ephemeral));
    }
    else if (strCmdID == Option_Overview)
    {
        // Acknowledge immediately to avoid timing out
        event.reply(dpp::message("...this could take a second. Please hold.").set_flags(dpp::m_ephemeral));

        // Edit the original message
        event.edit_original_response(SendPanel(ctx, event, author.id, 0 /* page */).set_flags(dpp::m_ephemeral));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Event handler for button interactions.
///
/// Handles all button clicks in the worker panel, including paging through assignments, completing jobs, adding 
/// notes, unassigning, and deleting.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
//---------------------------------------------------------------------------------------------------------------------
void WorkerPanelCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
    const std::string id = event.custom_id;
    auto parts = utils::Split(id, ':');
    if (parts[0] == fmt::format("{}_pageshow", Option_AllAssignments))
    {
        parts[4] = parts[4] == "true" ? "false" : "true";
    }

    if (parts[0] == fmt::format("{}_pagenext", Option_AllAssignments) ||
        parts[0] == fmt::format("{}_pageprev", Option_AllAssignments) ||
        parts[0] == fmt::format("{}_pageshow", Option_AllAssignments))
    {
        // Decompose custom id for state information
        const std::size_t type = std::stoul(parts[1]);
        std::size_t page = parts[2] != std::to_string(std::numeric_limits<std::size_t>::max()) ? std::stoul(parts[2]) : 0;
        const dpp::snowflake user = parts[3];
        const bool bShowComplete = parts[4] == "true" ? true : false;

        // Construct the actual info panel
        auto compare = [worker = user](const std::shared_ptr<const JobRequest> job) -> bool { return job->IsWorker(worker); };
        const std::size_t size = ctx.queue->GetQueueSize(bShowComplete, compare);

        if (size != 0)
        {
            const std::size_t lastPage = (size - 1) / JobQueue::JOBS_PER_DETAIL_PAGE;
            if (page > lastPage)
                page = lastPage;
        }
        else
        {
            page = 0;
        }

        const std::string result = ctx.queue->PrintPagedQueue(ctx.cluster, event.command.guild_id, page, bShowComplete, compare);
        const std::string header = "Your Assignments";

        PaginationPanel panel(ctx, Option_AllAssignments, 0, page, size, JobQueue::JOBS_PER_DETAIL_PAGE, bShowComplete, user);
        panel.AddEmbed(header, size != 0 ? result : "No requests or jobs currently assigned to you.");

        // Edit the original message
        event.reply(dpp::ir_update_message, panel.set_flags(dpp::m_ephemeral));
        return;
    }

    if (id.starts_with(fmt::format("{}_complete:", Option_Overview)))
    {
        WorkerPanel::CompleteButton(id, ctx, event);

        auto parts = utils::Split(id, ':');
        const dpp::snowflake user = parts[1];

        // Edit the original message
        event.reply(dpp::ir_update_message, SendPanel(ctx, event, user, 0).set_flags(dpp::m_ephemeral));

        return;
    }
    else if (id.starts_with(fmt::format("{}_edit:", Option_Overview)))
    {
        GeneralUserPanel::EditButton(id, ctx, event);
        return;
    }
    else if (id.starts_with(fmt::format("{}_note:", Option_Overview)))
    {
        GeneralUserPanel::NoteButton(id, ctx, event);
        return;
    }
    else if (id.starts_with(fmt::format("{}_unassign:", Option_Overview)))
    {
        WorkerPanel::UnassignButton(id, ctx, event);

        auto parts = utils::Split(id, ':');
        const dpp::snowflake user = parts[1];
        
        // Edit the original message
        event.reply(dpp::ir_update_message, SendPanel(ctx, event, user, 0).set_flags(dpp::m_ephemeral));

        return;
    }
    else if (id.starts_with(fmt::format("{}_delete:", Option_Overview)))
    {
        GeneralUserPanel::DeleteButton(id, ctx, event);
        return;
    }
    else if (id.starts_with(fmt::format("{}_allnotes:", Option_Overview)))
    {
        GeneralUserPanel::ShowNotesButton(id, ctx, event);
        return;
    }
    /*-------- Page (Queue) buttons --------*/
    else if (id.starts_with(fmt::format("{}_usernext:", Option_Overview)) ||
             id.starts_with(fmt::format("{}_userprev:", Option_Overview)))
    {
        // Deconstruct custom id for state information
        const auto parts = utils::Split(id, ':');
        const dpp::snowflake user = parts[1];
        const std::size_t page = parts[2] != std::to_string(std::numeric_limits<std::size_t>::max()) ? std::stoul(parts[2]) : 0;

        event.reply(dpp::ir_update_message, SendPanel(ctx, event, user, page).set_flags(dpp::m_ephemeral));
        return;
    }
}

dpp::message WorkerPanelCommand::SendPanel(
    CommandContext& ctx, 
    const dpp::interaction_create_t& event, 
    const dpp::snowflake& user,
    const std::size_t page) const
{
    // Return early if queue is empty
    const std::string header = "Worker Summary Report";
    if (ctx.queue->GetQueueSize() == 0)
    {
        dpp::embed embed;
        embed.set_title(header)
            .set_description("No requests or jobs in queue.")
            .set_color(0x3498db);

        return dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral);
    }

    // Construct the actual info panel
    const std::string summary = ctx.queue->PrintQueueWorkerSummary(ctx.cluster, user);
    const dpp::user author = event.command.get_issuing_user();

    // Retrieve the job
    const auto compare = [user](const std::shared_ptr<const JobRequest> job) -> bool { return job->IsWorker(user) && job->GetStatus() < JobRequest::status::complete; };
    const auto job = ctx.queue->FirstAssignment(compare, page);
    const std::size_t size = ctx.queue->GetQueueSize(true, compare);
    if (!job)
    {
        dpp::embed overview;
        overview.set_title(header)
                .set_description(!summary.empty() ? summary : "No job data in the active queue.")
                .set_color(0x3498db);
        dpp::embed top;
        top.set_title("Assigned Job (Page 1 of 1)")
            .set_description("No active requests or jobs currently assigned to you.")
            .set_color(0x3498db);

        return dpp::message().add_embed(overview).add_embed(top).set_flags(dpp::m_ephemeral);
    }

    const std::string priority = job->PrintJobDetails(ctx.cluster, event.command.guild_id);
    const std::string strJobID = ToString(job->GetID());

    WorkerPanel panel(Option_Overview, user, strJobID, job->IsWorker(user));
    panel.AddPageRow(page, size);
    panel.AddEmbed(header, summary);
    panel.AddEmbed(fmt::format("Assigned Job (Page {} of {})", page + 1, size), priority);

    return panel;
}