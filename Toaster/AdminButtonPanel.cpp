#include "AdminButtonPanel.h"

#include "AdminConfigDialog.h"
#include "BotUtility.h"
#include "CommandContext.h"
#include "JobQueue.h"
#include "JobRequest.h"
#include "GuildSettings.h"
#include "PermissionsMgr.h"
// d++
#include <dpp/guild.h>
// fmt
#include <fmt/format.h>
// std library
#include <memory>
#include <unordered_map>

AdminQueueButtonPanel::AdminQueueButtonPanel(
	const std::string& OwnerName,
	const dpp::snowflake& userID)
	: dpp::message(), m_userID{ userID }
{

	m_btnAssignWorker.set_type(dpp::cot_button)
		.set_label("Assign Worker")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_assignworkers:{}", OwnerName, m_userID));

	m_btnAddWorker.set_type(dpp::cot_button)
		.set_label("Add Worker")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_addworker:{}", OwnerName, m_userID));

	m_btnRemoveWorker.set_type(dpp::cot_button)
		.set_label("Remove Worker")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_removeworker:{}", OwnerName, m_userID));

	m_row.add_component(m_btnAssignWorker)
		.add_component(m_btnAddWorker)
		.add_component(m_btnRemoveWorker);

	m_btnShowWorkers.set_type(dpp::cot_button)
		.set_label("Show Workers")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_showworkers:{}", OwnerName, m_userID));

	m_btnArchiveJobs.set_type(dpp::cot_button)
		.set_label("Archive Completed")
		.set_style(dpp::cos_danger)
		.set_id(fmt::format("{}_archivecomplete:{}", OwnerName, m_userID));

	m_row2.add_component(m_btnShowWorkers)
		.add_component(m_btnArchiveJobs);

	add_component(m_row);
	add_component(m_row2);
}

void AdminQueueButtonPanel::AddEmbed(const std::string& header, const std::string& description)
{
	dpp::embed embed;
	embed.set_title(fmt::format("{}", header))
		.set_description(description)
		.set_color(0x3498db);

	add_embed(embed);
}

void AdminQueueButtonPanel::ShowWorkersButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	//event.reply(dpp::message("Functionality coming soon...").set_flags(dpp::m_ephemeral));
	dpp::guild* guild = utils::FindGuildByID(ctx.cluster, event.command.guild_id);
	if (!guild) return;

	const auto pManager = PermissionsMgr::GetInstance();
	const auto& members = guild->members;

	// Role display names in enum order
	constexpr std::array<const char*, 8> roleNames{
		"General Worker Role",
		"Item Crafter",
		"Base Builder",
		"Component Dealer",
		"Resource Gatherer",
		"Refinery Worker",
		"Hazardous Materials Collector",
		"Manager"
	};

	fmt::memory_buffer buffer;
	std::size_t count = 0;
	for (const auto& role : ctx.guild->roles)
	{
		fmt::format_to(std::back_inserter(buffer), "### {}\n", roleNames[count]);
		for (const auto& member : members)
		{
			if (role.has_value() && pManager->HasRole(member.second, role))
				fmt::format_to(std::back_inserter(buffer), "{}\n", utils::FindPreferredNameByID(ctx.cluster, member.first, guild->id));
		}

		++count;
	}

	dpp::message msg;
	const std::string response = fmt::to_string(buffer);
	if (response.size() < 5500)
	{
		dpp::embed embed;
		embed.set_title("Assigned Worker Roles")
			.set_description(fmt::to_string(buffer))
			.set_color(0x3498db);

		msg.add_embed(embed);
	}
	else
	{
		msg.add_file("Worker_List.txt", response);
	}

	event.reply(msg.set_flags(dpp::m_ephemeral));
}

void AdminQueueButtonPanel::AssignWorkerButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	event.reply(dpp::message("Functionality coming soon...").set_flags(dpp::m_ephemeral));
}

void AdminQueueButtonPanel::AddWorkerButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	event.reply(dpp::message("Functionality coming soon...").set_flags(dpp::m_ephemeral));
}

void AdminQueueButtonPanel::RemoveWorkerButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	event.reply(dpp::message("Functionality coming soon...").set_flags(dpp::m_ephemeral));
}

void AdminQueueButtonPanel::ArchiceCompletedButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	event.reply(dpp::message("Functionality coming soon...").set_flags(dpp::m_ephemeral));
}

AdminBotButtonPanel::AdminBotButtonPanel(
	const std::string& OwnerName,
	const dpp::snowflake& userID)
	: dpp::message(), m_userID{ userID }
{
	// Row 1
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

	m_row.add_component(m_btnChangeRole)
		.add_component(m_btnChangeChannel)
		.add_component(m_btnPingRules);

	// Row 2
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

	// Row 3
	m_btnSendLogs.set_type(dpp::cot_button)
		.set_label("Send Logs")
		.set_style(dpp::cos_primary)
		.set_id(fmt::format("{}_sendlogs:{}", OwnerName, m_userID));

	m_btnSendQueue.set_type(dpp::cot_button)
		.set_label("Send Queue")
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
	add_component(m_row2);
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
		[](const std::shared_ptr<const JobRequest> job) -> bool { return true; });

	utils::RemoveChar(queue, '*');

	event.reply(
		dpp::message("Functionality coming soon...")
		.set_flags(dpp::m_ephemeral)
		.add_file(fmt::format("queue_{}.txt", event.command.guild_id), queue)
	);
}

void AdminBotButtonPanel::DeleteDataButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
	event.reply(dpp::message("Functionality coming soon...").set_flags(dpp::m_ephemeral));
}