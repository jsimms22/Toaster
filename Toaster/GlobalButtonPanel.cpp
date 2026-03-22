#include "GlobalButtonPanel.h"

#include "BotUtility.h"
#include "CommandContext.h"
#include "JobRequest.h"
#include "GuildSettings.h"
#include "PermissionsMgr.h"
#include "JobQueue.h"

#include "NoteDialog.h"
// fmt
#include <fmt/format.h>

GlobalButtonPanel::GlobalButtonPanel(const std::string& rID, const bool bReopened)
	: dpp::message(), m_rID{ rID }
{
	m_btnGlobalAssign.set_type(dpp::cot_button)
		.set_label("Assign Me")
		.set_style(dpp::cos_success)
		.set_id(fmt::format("global_assign:{}:{}", rID, bReopened));

	m_btnGlobalUnassign.set_type(dpp::cot_button)
		.set_label("Unassign Me")
		.set_style(dpp::cos_danger)
		.set_id(fmt::format("global_unassign:{}:{}", rID, bReopened));

	m_btnGlobalAddNote.set_type(dpp::cot_button)
		.set_label("Add Note")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("global_addnotes:{}:{}", rID, bReopened));

	m_btnGlobalShowNotes.set_type(dpp::cot_button)
		.set_label("Show Notes")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("global_shownotes:{}:{}", rID, bReopened));

	m_row.add_component(m_btnGlobalAssign)
		 .add_component(m_btnGlobalUnassign)
		 .add_component(m_btnGlobalAddNote)
		 .add_component(m_btnGlobalShowNotes);

    add_component(m_row);
}

void GlobalButtonPanel::AddEmbed(const std::string& header, const std::string& description)
{
    dpp::embed embed;
    embed.set_title(fmt::format("{}", header))
        .set_description(description)
        .set_color(0x3498db);

    add_embed(embed);
}

bool GlobalButtonPanel::AssignButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
    auto parts = utils::Split(id, ':');
    const std::string rID = parts[1];
    const auto job = ctx.queue->GetJobByID(rID);
    if (!job)
    {
        event.reply(dpp::ir_update_message, dpp::message("This job was not found in the queue. It may have been deleted or archived."));
        ctx.cluster.log(dpp::ll_warning, fmt::format("Global assign was pressed for {}", rID));
        return false;
    }

    const auto pManager = PermissionsMgr::GetInstance();
    const dpp::user user = event.command.get_issuing_user();
    if (!pManager || (!pManager->CanAssignJob(event, user.id, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) && !ctx.debug))
    {
        // Permission denied
        event.reply(dpp::message(fmt::format("You do not have permissions to modify {}.", ToString(job->GetID()))).set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("{} attempted to modify job {} with global assign button.", user.global_name, ToString(job->GetID())));
        return false;
    }

    if (!job->IsWorker(user.id))
    {
        ctx.queue->RequestModify(job->GetID(),
            [id = user.id](std::shared_ptr<JobRequest> job)
            {
                job->AddWorkerID(id);
                if (job->GetStatus() < JobRequest::status::complete)
                    job->SetStatus(JobRequest::status::assigned);
            });

        const dpp::snowflake customer = job->GetCustomerID();
        if ((job->IsCustomerSubscribed() && customer != user.id) || ctx.debug)
        {
            utils::NotifyIssuerMsg(ctx.cluster, job->GetCustomerID(), event,
                fmt::format("{} has been assigned to your request {}.", fmt::format("<@{}>", user.id), rID));
        }
    }

    return true;
}

bool GlobalButtonPanel::UnassignButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
    auto parts = utils::Split(id, ':');
    const std::string rID = parts[1];
    const auto job = ctx.queue->GetJobByID(rID);
    if (!job)
    {
        event.reply(dpp::ir_update_message, dpp::message("This job was not found in the queue. It may have been deleted or archived."));
        ctx.cluster.log(dpp::ll_warning, fmt::format("Global assign was pressed for {}", rID));
        return false;
    }

    const auto pManager = PermissionsMgr::GetInstance();
    const dpp::user user = event.command.get_issuing_user();
    if (!pManager || (!pManager->CanAssignJob(event, user.id, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) && !ctx.debug))
    {
        // Permission denied
        event.reply(dpp::message(fmt::format("You do not have permissions to modify {}.", ToString(job->GetID()))).set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("{} attempted to modify job {} with global assign button.", user.global_name, ToString(job->GetID())));
        return false;
    }

    if (job->IsWorker(user.id))
    {
        ctx.queue->RequestModify(job->GetID(),
            [id = user.id](std::shared_ptr<JobRequest> job)
            {
                job->RemoveWorkerID(id);
                if (job->GetStatus() != JobRequest::status::complete && job->GetWorkerIDs().empty())
                    job->SetStatus(JobRequest::status::open);
            });

        const dpp::snowflake customer = job->GetCustomerID();
        if ((job->IsCustomerSubscribed() && customer != user.id) || ctx.debug)
        {
            utils::NotifyIssuerMsg(ctx.cluster, job->GetCustomerID(), event,
                fmt::format("{} has been unassigned from your request {}.", fmt::format("<@{}>", user.id), rID));
        }
    }

    return true;
}

void GlobalButtonPanel::AddNoteButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
    auto parts = utils::Split(id, ':');
    const std::string rID = parts[1];

    const auto job = ctx.queue->GetJobByID(rID);
    if (!job)
    {
        event.reply(dpp::message("Could not find the job by its ID. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
        return;
    }

    const auto pManager = PermissionsMgr::GetInstance();
    if (pManager->CanAddNote(event, event.command.get_issuing_user().id, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) || ctx.debug)
    {
        NoteDialog modal(ctx, job);
        event.dialog(modal);
        return;
    }
    else
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_debug, fmt::format("USER '{}' was DENIED access to use '{}' button", event.command.get_issuing_user().id, parts[0]));
        return;
    }
}

void GlobalButtonPanel::ShowNotesButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
    auto parts = utils::Split(id, ':');
    const std::string rID = parts[1];

    const auto job = ctx.queue->GetJobByID(rID);
    if (!job)
    {
        event.reply(dpp::message("Could not find the job by its ID. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
        return;
    }

    const auto pManager = PermissionsMgr::GetInstance();
    // Check if the user has permission to view notes for the job
    if (pManager->CanAddNote(event, event.command.get_issuing_user().id, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) || ctx.debug)
    {
        // Format the note history
        const std::string noteHistory = job->PrintNoteHistory(ctx.cluster);

        // Check if there are any notes to show
        if (noteHistory.empty())
        {
            event.reply(dpp::message("There are no notes for this job.").set_flags(dpp::m_ephemeral));
        }
        else
        {
            // Send the note history in an embed message
            dpp::embed embed;
            embed.set_title(fmt::format("Notes for Job: {}", rID))
                .set_description(noteHistory)
                .set_color(0x3498db); // You can adjust this color as needed

            event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        }
    }
    else
    {
        // If user doesn't have permission to view notes
        event.reply(dpp::message("You do not have sufficient permissions to view the notes for this job.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_debug, fmt::format("USER '{}' was DENIED access to view notes for job '{}'", event.command.get_issuing_user().id, rID));
        return;
    }
}