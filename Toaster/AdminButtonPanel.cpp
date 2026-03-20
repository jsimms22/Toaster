#include "AdminButtonPanel.h"

#include "AdminConfigDialog.h"
#include "BotUtility.h"
#include "CommandContext.h"
#include "JobQueue.h"
#include "JobRequest.h"
#include "GuildSettings.h"
#include "PermissionsMgr.h"
#include "RequestDlg.h"
#include "Resource.h"
#include "ShowWorkersPanel.h"
#include "WorkerPanel.h"
// d++
#include <dpp/guild.h>
// fmt
#include <fmt/format.h>
// std library
#include <memory>
#include <unordered_map>

AdminQueueButtonPanel::AdminQueueButtonPanel(
	const std::string& OwnerName,
	const dpp::snowflake& userID,
	const std::string& rID)
	: dpp::message(), m_userID{ userID }, m_ownerName{OwnerName}
{
	// row 2
	m_btnRefreshPanel.set_type(dpp::cot_button)
		.set_label("Refresh Panel")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_queuerefresh:{}:{}", m_ownerName, m_userID, rID));

	m_btnAssignWorkers.set_type(dpp::cot_button)
		.set_label("Assign Workers")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_assignworkers:{}:{}", m_ownerName, m_userID, rID));

	m_btnMarkComplete.set_type(dpp::cot_button)
		.set_label("Mark Complete")
		.set_style(dpp::cos_success)
		.set_id(fmt::format("{}_admincomplete:{}:{}", m_ownerName, m_userID, rID));

	m_btnMarkOnHold.set_type(dpp::cot_button)
		.set_label("Mark On Hold")
		.set_style(dpp::cos_danger)
		.set_id(fmt::format("{}_adminhold:{}:{}", m_ownerName, m_userID, rID));

	m_row2.add_component(m_btnRefreshPanel)
		.add_component(m_btnAssignWorkers)
		.add_component(m_btnMarkComplete)
		.add_component(m_btnMarkOnHold);

	// row 3
	m_btnShowWorkers.set_type(dpp::cot_button)
		.set_label("Show Workers")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_showworkers:{}", m_ownerName, m_userID, rID));

	m_btnDownloadWorkers.set_type(dpp::cot_button)
		.set_label("Download Workers")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_downloadworkers:{}", m_ownerName, m_userID, rID));

	m_btnDownloadArchive.set_type(dpp::cot_button)
		.set_label("Download Archive")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_downloadarchive:{}", m_ownerName, m_userID));

	m_btnReopenArchived.set_type(dpp::cot_button)
		.set_label("Reopen Archived Job")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_reopenarchive:{}", m_ownerName, m_userID));

	m_row3.add_component(m_btnShowWorkers)
		.add_component(m_btnDownloadWorkers)
		.add_component(m_btnDownloadArchive)
		.add_component(m_btnReopenArchived);

	add_component(m_row2);
	add_component(m_row3);
}

void AdminQueueButtonPanel::AddEmbed(const std::string& header, const std::string& description)
{
	dpp::embed embed;
	embed.set_title(fmt::format("{}", header))
		.set_description(description)
		.set_color(0x3498db);

	add_embed(embed);
}

void AdminQueueButtonPanel::AddPageRow(const std::size_t page, const std::size_t size)
{
	const auto lastPage = size <= 1 ? 0 : (size - 1) / 1;

	const std::size_t prevPage = page > 0 ? page - 1 : 0;
	const std::size_t nextPage = page < lastPage ? page + 1 : lastPage;

	if (prevPage != nextPage)
	{
		m_btnPrev.set_type(dpp::cot_button)
			.set_label("Prev")
			.set_style(dpp::cos_primary)
			.set_id(fmt::format("{}_adminnext:{}", m_ownerName, prevPage));

		m_btnNext.set_type(dpp::cot_button)
			.set_label("Next")
			.set_style(dpp::cos_primary)
			.set_id(fmt::format("{}_adminprev:{}", m_ownerName, nextPage));

		m_pagerow.add_component(m_btnPrev)
				 .add_component(m_btnNext);

		add_component(m_pagerow);
	}
}

void AdminQueueButtonPanel::ShowWorkersButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event, const bool bSendFile)
{
	dpp::guild* guild = utils::FindGuildByID(ctx.cluster, event.command.guild_id);
	if (!guild) return;

	const auto pManager = PermissionsMgr::GetInstance();
	const auto& members = guild->members;
	const auto role = ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Ping)];

	fmt::memory_buffer buffer;
	fmt::format_to(std::back_inserter(buffer), "### {}\n", GuildSettings::RoleNames[static_cast<std::size_t>(GuildSettings::Roles::Ping)]);
	for (const auto& member : members)
	{
		if (role.has_value() && pManager->HasRole(member.second, role))
		{
			std::string label = fmt::format("<@{}>", member.first);
			fmt::format_to(std::back_inserter(buffer), "- {}\n", label);
		}
	}

	ShowWorkersPanel workerlist{ Command_ShowWorkers, ctx.guild };
	workerlist.AddEmbed("Assigned Worker Roles", fmt::to_string(buffer));

	event.reply(workerlist.set_flags(dpp::m_ephemeral));
}

void AdminQueueButtonPanel::DownloadWorkersButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	dpp::guild* guild = utils::FindGuildByID(ctx.cluster, event.command.guild_id);
	if (!guild) return;

	const auto pManager = PermissionsMgr::GetInstance();
	const auto& members = guild->members;

	fmt::memory_buffer buffer;
	std::size_t count = 0;
	for (const auto& role : ctx.guild->roles)
	{
		fmt::format_to(std::back_inserter(buffer), "### {}\n", GuildSettings::RoleNames[count]);
		for (const auto& member : members)
		{
			if (role.has_value() && pManager->HasRole(member.second, role))
			{
				fmt::format_to(std::back_inserter(buffer), "- {} | {}\n", member.first, utils::FindPreferredNameByID(ctx.cluster, member.first, guild->id));
			}
		}

		++count;
	}

	dpp::message msg;
	const std::string response = fmt::to_string(buffer);
	msg.add_file("Worker_List.txt", response);

	event.reply(msg.set_flags(dpp::m_ephemeral));
}

void AdminQueueButtonPanel::AssignWorkersButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	auto parts = utils::Split(id, ':');
	const dpp::snowflake worker = parts[1];
	const std::string rID = parts[2];

	const auto job = ctx.queue->GetJobByID(rID);
	if (!job)
	{
		event.reply(dpp::message("Could not find the job by its ID. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
		return;
	}

	AssignRequestDlg modal(job);
	event.dialog(modal);
	return;
}

void AdminQueueButtonPanel::MarkCompleteButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	WorkerPanel::CompleteButton(id, ctx, event);
}

void AdminQueueButtonPanel::MarkOnHoldButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	auto parts = utils::Split(id, ':');
	const dpp::snowflake worker = parts[1];
	const std::string rID = parts[2];

	const auto job = ctx.queue->GetJobByID(rID);
	if (!job)
	{
		event.reply(dpp::message("Could not find the job by its ID. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
		return;
	}

	const auto pManager = PermissionsMgr::GetInstance();
	if ((pManager->CanAssignJob(event, worker, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) &&
		job->GetStatus() < JobRequest::status::complete) || ctx.debug)
	{
		ctx.queue->RequestModify(job->GetID(), [](std::shared_ptr<JobRequest> job)
			{
				job->SetStatus(JobRequest::status::hold);
			});

		const dpp::snowflake customer = job->GetCustomerID();
		if ((job->IsCustomerSubscribed() && customer != worker) || ctx.debug)
		{
			utils::NotifyIssuerMsg(ctx.cluster, job->GetCustomerID(), event,
				fmt::format("Request {} has been placed on hold by {}.", rID, event.command.get_issuing_user().global_name));
		}

		ctx.cluster.log(dpp::ll_info, fmt::format("Request {} has been placed on hold by {}.", ToString(job->GetID()), event.command.get_issuing_user().global_name));
	}
	else
	{
		event.reply(dpp::message("Could not perform this action.").set_flags(dpp::m_ephemeral));
	}
}

void AdminQueueButtonPanel::RefreshButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	/* do nothing */
}

void AdminQueueButtonPanel::DownloadArchiveButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	std::string queue = ctx.queue->PrintArchive(ctx.cluster,
		event.command.guild_id,
		[](const std::shared_ptr<const JobRequest> job) -> bool { return true; });

	utils::RemoveChar(queue, '*');

	event.reply(
		dpp::message()
		.set_flags(dpp::m_ephemeral)
		.add_file(fmt::format("archive_{}.txt", event.command.guild_id), queue)
	);
}

void AdminQueueButtonPanel::ReopenArchiveButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	event.reply(dpp::message("Functionality coming soon...").set_flags(dpp::m_ephemeral));
}

AdminBotButtonPanel::AdminBotButtonPanel(
	const std::string& OwnerName,
	const dpp::snowflake& userID)
	: dpp::message(), m_userID{ userID }
{
	// Row 1
	m_btnRefreshPanel.set_type(dpp::cot_button)
		.set_label("Refresh Panel")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_botrefresh:{}", OwnerName, m_userID));

	m_btnChangeRole.set_type(dpp::cot_button)
		.set_label("Change Role")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_changerole:{}", OwnerName, m_userID));

	m_btnChangeChannel.set_type(dpp::cot_button)
		.set_label("Channel Rules")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_channelrules:{}", OwnerName, m_userID));

	m_btnPingRules.set_type(dpp::cot_button)
		.set_label("Ping Rules")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_pingrules:{}", OwnerName, m_userID));

	m_row.add_component(m_btnRefreshPanel)
		.add_component(m_btnChangeRole)
		.add_component(m_btnChangeChannel)
		.add_component(m_btnPingRules);

	// Row 2
	/*
	m_btnShowBans.set_type(dpp::cot_button)
		.set_label("Show Ban List")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_banlist:{}", OwnerName, m_userID));

	m_btnAddBan.set_type(dpp::cot_button)
		.set_label("Add Ban")
		.set_style(dpp::cos_danger)
		.set_id(fmt::format("{}_addban:{}", OwnerName, m_userID));

	m_btnRemoveBan.set_type(dpp::cot_button)
		.set_label("Remove Ban")
		.set_style(dpp::cos_success)
		.set_id(fmt::format("{}_removeban:{}", OwnerName, m_userID));

	m_row2.add_component(m_btnShowBans)
		.add_component(m_btnAddBan)
		.add_component(m_btnRemoveBan);
	*/

	// Row 3
	m_btnSendLogs.set_type(dpp::cot_button)
		.set_label("Download Logs")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_sendlogs:{}", OwnerName, m_userID));

	m_btnSendQueue.set_type(dpp::cot_button)
		.set_label("Download Queue")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_sendqueue:{}", OwnerName, m_userID));

	m_btnDeleteData.set_type(dpp::cot_button)
		.set_label("Delete Guild Data")
		.set_style(dpp::cos_danger)
		.set_id(fmt::format("{}_deleteguild:{}", OwnerName, m_userID));

	m_row3.add_component(m_btnSendLogs)
		.add_component(m_btnSendQueue)
		.add_component(m_btnDeleteData);

	add_component(m_row);
	//add_component(m_row2);
	add_component(m_row3);
}

void AdminBotButtonPanel::AddEmbed(const std::string& header, const std::string& description)
{
	dpp::embed embed;
	embed.set_title(fmt::format("{}", header))
		.set_description(description)
		.set_color(0x3498db);

	add_embed(embed);
}

// Row 1
void AdminBotButtonPanel::ChangeRoleButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	AdminConfigDialog adminDlg(Command_ConfigRoles, ctx);
	event.dialog(adminDlg);
}

void AdminBotButtonPanel::ChangeChannelButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	AdminConfigDialog adminDlg(Command_ConfigChannels, ctx);
	event.dialog(adminDlg);
}

void AdminBotButtonPanel::PingRulesButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	AdminConfigDialog adminDlg(Command_ConfigPing, ctx);
	event.dialog(adminDlg);
}

// Row 2
void AdminBotButtonPanel::ShowBansButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	event.reply(dpp::message("Functionality coming soon...").set_flags(dpp::m_ephemeral));
}

void AdminBotButtonPanel::AddBanButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	event.reply(dpp::message("Functionality coming soon...").set_flags(dpp::m_ephemeral));
}

void AdminBotButtonPanel::RemoveBanButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	event.reply(dpp::message("Functionality coming soon...").set_flags(dpp::m_ephemeral));
}

// Row 3
void AdminBotButtonPanel::SendLogsButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	event.reply(dpp::message("Functionality coming soon...").set_flags(dpp::m_ephemeral).add_file("testLog.txt", "Imagine a forest. There might be a bear. But definitely some logs."));
}

void AdminBotButtonPanel::SendQueueButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	std::string queue = ctx.queue->PrintQueue(ctx.cluster,
		event.command.guild_id,
		true,
		[](const std::shared_ptr<const JobRequest> job) -> bool { return true; });

	utils::RemoveChar(queue, '*');

	event.reply(
		dpp::message()
		.set_flags(dpp::m_ephemeral)
		.add_file(fmt::format("queue_{}.txt", event.command.guild_id), queue)
	);
}

void AdminBotButtonPanel::DeleteDataButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	event.reply(dpp::message("Functionality coming soon...").set_flags(dpp::m_ephemeral));
}