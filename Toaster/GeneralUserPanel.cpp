#include "GeneralUserPanel.h"

#include "BotUtility.h"
#include "CommandContext.h"
#include "JobRequest.h"
#include "JobQueue.h"
#include "RequestDlg.h"
// fmt
#include <fmt/format.h>


GeneralUserPanel::GeneralUserPanel(
	CommandContext& ctx,
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
	const std::string guid = parts[2];

	auto job = ctx.queue->GetJobByGUID(guid);
	if (!job)
	{
		event.reply(dpp::message("Could not find the job by its ID. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
		return;
	}

	const auto pManager = PermissionsMgr::GetInstance();
	if (pManager->CanEditJob(user, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) || ctx.debug)
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
	const std::string guid = parts[2];

	auto job = ctx.queue->GetJobByGUID(guid);
	if (!job)
	{
		event.reply(dpp::message("Could not find the job by its ID. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
		return;
	}

	const auto pManager = PermissionsMgr::GetInstance();
	if (pManager->CanAddNote(user, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) || ctx.debug)
	{
		/* todo add note functionality to job class */
		event.reply(dpp::message("Functionality currently not supported.").set_flags(dpp::m_ephemeral));
		return;

		// todo note modal dlg
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
	const std::string guid = parts[2];

	auto job = ctx.queue->GetJobByGUID(guid);
	if (!job)
	{
		event.reply(dpp::message("Could not find the job by its ID. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
		return;
	}

	const auto pManager = PermissionsMgr::GetInstance();
	if (pManager->CanDeleteJob(user, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) || ctx.debug)
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