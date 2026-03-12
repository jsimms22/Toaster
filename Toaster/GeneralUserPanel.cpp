#include "GeneralUserPanel.h"

#include "BotUtility.h"
#include "CommandContext.h"
#include "GuildSettings.h"
#include "PermissionsMgr.h"
#include "JobRequest.h"
#include "JobQueue.h"
#include "RequestDlg.h"
#include "NoteDialog.h"
// fmt
#include <fmt/format.h>

GeneralUserPanel::GeneralUserPanel(
	const std::string& OwnerName,
	const dpp::snowflake& userID,
	const std::string& jobID)
	: dpp::message(), m_userID{ userID }, m_jobID{ jobID }
{
	m_btnEdit.set_type(dpp::cot_button)
		.set_label("Edit")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_edit:{}:{}", OwnerName, m_userID, m_jobID));

	m_btnNote.set_type(dpp::cot_button)
		.set_label("Add Note")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_note:{}:{}", OwnerName, m_userID, m_jobID));

	m_btnDelete.set_type(dpp::cot_button)
		.set_label("Delete")
		.set_style(dpp::cos_danger)
		.set_id(fmt::format("{}_delete:{}:{}", OwnerName, m_userID, m_jobID));

	m_btnShowNotes.set_type(dpp::cot_button)
		.set_label("Show Job Notes")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_allnotes:{}:{}", OwnerName, m_userID, m_jobID));
}

void GeneralUserPanel::AddEmbed(const std::string& header, const std::string& description)
{
	dpp::embed embed;
	embed.set_title(fmt::format("{}", header))
		.set_description(description)
		.set_color(0x3498db);

	add_embed(embed);
}


//---------------------------------------------------------------------------------------------------------------------
/// \brief 
///
/// \param[in] id        Button id string
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Modal form submission event.
//---------------------------------------------------------------------------------------------------------------------
void GeneralUserPanel::EditButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	auto parts = utils::Split(id, ':');
	const dpp::snowflake user = parts[1];
	const std::string rID = parts[2];

	const auto job = ctx.queue->GetJobByID(rID);
	if (!job)
	{
		event.reply(dpp::message("Could not find the job by its ID. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
		return;
	}

	const auto pManager = PermissionsMgr::GetInstance();
	if (pManager->CanEditJob(event, user, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) || ctx.debug)
	{
		EditRequestDlg modal(job);
		event.dialog(modal);
	}
	else
	{
		event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
		ctx.cluster.log(dpp::ll_debug, fmt::format("USER '{}' was DENIED access to use '{}' button", user, parts[0]));
		return;
	}
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief 
///
/// \param[in] id        Button id string
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Modal form submission event.
//---------------------------------------------------------------------------------------------------------------------
void GeneralUserPanel::NoteButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	auto parts = utils::Split(id, ':');
	const dpp::snowflake user = parts[1];
	const std::string rID = parts[2];

	const auto job = ctx.queue->GetJobByID(rID);
	if (!job)
	{
		event.reply(dpp::message("Could not find the job by its ID. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
		return;
	}

	const auto pManager = PermissionsMgr::GetInstance();
	if (pManager->CanAddNote(event, user, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) || ctx.debug)
	{
		NoteDialog modal(ctx, job);
		event.dialog(modal);
	}
	else
	{
		event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
		ctx.cluster.log(dpp::ll_debug, fmt::format("USER '{}' was DENIED access to use '{}' button", user, parts[0]));
		return;
	}
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief 
///
/// \param[in] id        Button id string
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Modal form submission event.
//---------------------------------------------------------------------------------------------------------------------
void GeneralUserPanel::DeleteButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	auto parts = utils::Split(id, ':');
	const dpp::snowflake user = parts[1];
	const std::string rID = parts[2];

	const auto job = ctx.queue->GetJobByID(rID);
	if (!job)
	{
		event.reply(dpp::message("Could not find the job by its ID. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
		return;
	}

	const auto pManager = PermissionsMgr::GetInstance();
	if (pManager->CanDeleteJob(event, user, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) || ctx.debug)
	{
		DeleteRequestDlg modal(job, job->PrintJobDetails(ctx.cluster, event.command.guild_id));
		event.dialog(modal);
	}
	else
	{
		event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
		ctx.cluster.log(dpp::ll_debug, fmt::format("USER '{}' was DENIED access to use '{}' button", user, parts[0]));
		return;
	}
}

void GeneralUserPanel::ShowNotesButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	auto parts = utils::Split(id, ':');
	const dpp::snowflake user = parts[1];
	const std::string rID = parts[2];

	const auto job = ctx.queue->GetJobByID(rID);
	if (!job)
	{
		event.reply(dpp::message("Could not find the job by its ID. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
		return;
	}

	const auto pManager = PermissionsMgr::GetInstance();
	// Check if the user has permission to view notes for the job
	if (pManager->CanAddNote(event, user, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) || ctx.debug)
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
		ctx.cluster.log(dpp::ll_debug, fmt::format("USER '{}' was DENIED access to view notes for job '{}'", user, rID));
		return;
	}
}