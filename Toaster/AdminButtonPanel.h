#pragma once
// d++
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/snowflake.h>
// std library
#include <string>

struct CommandContext;

class AdminQueueButtonPanel :public dpp::message
{
public:
	AdminQueueButtonPanel(
		const std::string& OwnerName,
		const dpp::snowflake& userID);

	virtual ~AdminQueueButtonPanel() = default;

	void AddEmbed(const std::string& header, const std::string& description);

	static void ShowWorkersButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void AssignWorkerButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void AddWorkerButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void RemoveWorkerButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void ArchiceCompletedButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);

protected:
	dpp::component m_row;
	dpp::component m_row2;

	// Row 1
	dpp::component m_btnAssignWorker;
	dpp::component m_btnAddWorker;
	dpp::component m_btnRemoveWorker;

	// Row 2
	dpp::component m_btnShowWorkers;
	dpp::component m_btnArchiveJobs;

	dpp::snowflake m_userID;
};

class AdminBotButtonPanel :public dpp::message
{
public:
	AdminBotButtonPanel(
		const std::string& OwnerName,
		const dpp::snowflake& userID);

	virtual ~AdminBotButtonPanel() = default;

	void AddEmbed(const std::string& header, const std::string& description);

	// Row 1
	static void ChangeRoleButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void ChangeChannelButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void PingRulesButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);

	// Row 2
	static void ShowBansButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void AddBanButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void RemoveBanButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	
	// Row 3
	static void SendLogsButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void SendQueueButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void DeleteDataButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);

protected:
	dpp::component m_row;
	dpp::component m_row2;
	dpp::component m_row3;

	// Row 1
	dpp::component m_btnChangeRole;
	dpp::component m_btnChangeChannel;
	dpp::component m_btnPingRules;

	// Row 2
	dpp::component m_btnShowBans;
	dpp::component m_btnAddBan;
	dpp::component m_btnRemoveBan;

	// Row 3
	dpp::component m_btnSendLogs;
	dpp::component m_btnSendQueue;
	dpp::component m_btnDeleteData;

	dpp::snowflake m_userID;
};