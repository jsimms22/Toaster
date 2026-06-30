//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief Implementation of the ShowRequestCommand.
///
/// Contains the implementation of ShowRequestCommand declared in Commands.h.
//---------------------------------------------------------------------------------------------------------------------
#include "Commands.h"
#include "JobQueue.h"
#include "JobRequest.h"
#include "BotUtility.h"
#include "RequestDlg.h"
#include "PermissionsMgr.h"
#include "WorkerPanel.h"
#include "CustomerPanel.h"
#include "PaginationPanel.h"
// fmt
#include <fmt/format.h>

//---------------------------------------------------------------------------------------------------------------------
/// \brief Event handler for create interaction processes.
///
/// Handles the command by displaying the details of a specific job request. Verifies that the user has permission 
/// to view the request.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
//---------------------------------------------------------------------------------------------------------------------
void ShowRequestCommand::ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    std::string rID = std::get<std::string>(event.get_parameter(Parameter_Id));
    utils::FilterWhiteSpace(rID);
    const auto job = ctx.queue->GetJobByID(rID);
    if (!job)
    {
        event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    const auto pManager = PermissionsMgr::GetInstance();
    if (!(pManager->IsRequestOwner(author.id, job) ||
          pManager->IsRequestWorker(author.id, job) ||
          pManager->IsWorker(author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) ||
          pManager->IsManager(author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) ||
          pManager->IsGuildAdmin(author.id, event) ||
          pManager->IsBotOwner(author.id)) && !ctx.debug)
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("USER '{}' was DENIED access to use '{}' command", author.id, event.command.get_command_name()));
        return;
    }

    // Acknowledge immediately to avoid timing out
    event.reply(dpp::message("...this could take a second. Please hold.").set_flags(dpp::m_ephemeral));

    // Edit the original message
    event.edit_original_response(SendPanel(ctx, event, job, author.id).set_flags(dpp::m_ephemeral));
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Event handler for button interactions.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
//---------------------------------------------------------------------------------------------------------------------
void ShowRequestCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
    const std::string id = event.custom_id;

    if (id.starts_with(fmt::format("{}_refresh:", this->name)))
    {
        auto parts = utils::Split(id, ':');
        dpp::snowflake user = parts[1];
        const std::string rID = parts[2];
        const auto job = ctx.queue->GetJobByID(rID);

        if (!job)
        {
            event.reply(dpp::message("Could not find the job by its ID. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
            return;
        }

        // Edit the original message
        event.reply(dpp::ir_update_message, SendPanel(ctx, event, job, user).set_flags(dpp::m_ephemeral));
        return;
    }
    else if (id.starts_with(fmt::format("{}_complete:", this->name)))
    {
        WorkerPanel::CompleteButton(id, ctx, event);

        auto parts = utils::Split(id, ':');
        dpp::snowflake user = parts[1];
        const std::string rID = parts[2];
        const auto job = ctx.queue->GetJobByID(rID);

        if (!job)
        {
            event.reply(dpp::message("Could not find the job by its ID. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
            return;
        }

        // Edit the original message
        event.reply(dpp::ir_update_message, SendPanel(ctx, event, job, user).set_flags(dpp::m_ephemeral));
        return;
    }
    else if (id.starts_with(fmt::format("{}_edit:", this->name)))
    {
        GeneralUserPanel::EditButton(id, ctx, event);
        return;
    }
    else if (id.starts_with(fmt::format("{}_note:", this->name)))
    {
        GeneralUserPanel::NoteButton(id, ctx, event);
        return;
    }
    else if (id.starts_with(fmt::format("{}_unassign:", this->name)))
    {
        WorkerPanel::UnassignButton(id, ctx, event);

        auto parts = utils::Split(id, ':');
        dpp::snowflake user = parts[1];
        const std::string rID = parts[2];
        const auto job = ctx.queue->GetJobByID(rID);

        if (!job)
        {
            event.reply(dpp::message("Could not find the job by its ID. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
            return;
        }

        // Edit the original message
        event.reply(dpp::ir_update_message, SendPanel(ctx, event, job, user).set_flags(dpp::m_ephemeral));
        return;
    }
    else if (id.starts_with(fmt::format("{}_subscribe:", this->name)))
    {
        CustomerPanel::SubscribeButton(id, ctx, event);

        auto parts = utils::Split(id, ':');
        dpp::snowflake user = parts[1];
        const std::string rID = parts[2];
        const auto job = ctx.queue->GetJobByID(rID);

        if (!job)
        {
            event.reply(dpp::message("Could not find the job by its ID. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
            return;
        }

        // Edit the original message
        event.reply(dpp::ir_update_message, SendPanel(ctx, event, job, user).set_flags(dpp::m_ephemeral));
        return;
    }
    else if (id.starts_with(fmt::format("{}_delete:", this->name)))
    {
        GeneralUserPanel::DeleteButton(id, ctx, event);
        return;
    }
    else if (id.starts_with(fmt::format("{}_allnotes:", this->name)))
    {
        GeneralUserPanel::ShowNotesButton(id, ctx, event);
        return;
    }

    if (id.starts_with("globalpanel_pagenext:") ||
        id.starts_with("globalpanel_pageprev:") ||
        id.starts_with("generaluser_pagenext:") ||
        id.starts_with("generaluser_pageprev:"))
    {
        auto parts = utils::Split(id, ':');
        const std::string rID = parts[1];
        std::size_t page = parts[2] != std::to_string(std::numeric_limits<std::size_t>::max()) ? std::stoul(parts[2]) : 0;

        const auto job = ctx.queue->GetJobByID(rID);
        if (!job)
        {
            event.reply(dpp::message("Could not find the job by its ID. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
            return;
        }

        const auto numNotes = job->NoteHistorySize();

        // Check if there are any notes to show
        if (numNotes == 0)
        {
            event.reply(dpp::message("There are no notes for this job.").set_flags(dpp::m_ephemeral));
        }
        else
        {
            // Format the note history
            const std::string noteHistory = job->PrintNoteHistory(ctx.cluster, page);

            const std::string header = fmt::format("Notes for Job: {}", rID);
            PaginationPanel panel(ctx, "generaluser", rID, page, numNotes, JobRequest::NOTES_PER_PAGE);
            panel.AddEmbed(header, numNotes != 0 ? noteHistory : "There are no notes for this job.");

            event.reply(dpp::ir_update_message, panel.set_flags(dpp::m_ephemeral));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
dpp::message ShowRequestCommand::SendPanel(
    CommandContext& ctx, 
    const dpp::interaction_create_t& event, 
    const std::shared_ptr<const JobRequest>& job,
    const dpp::snowflake& user) const
{
    if (user == job->GetCustomerID() && !job->IsWorker(user))
    {
        CustomerPanel panel(this->name, user, ToString(job->GetID()), job->IsCustomerSubscribed());
        panel.AddEmbed("Here is the Request", job->PrintJobDetails(ctx.cluster, event.command.guild_id));
        return panel;
    }
    else
    {
        WorkerPanel panel(this->name, user, ToString(job->GetID()), user);
        panel.AddEmbed("Here is the Request", job->PrintJobDetails(ctx.cluster, event.command.guild_id));
        return panel;
    }
}