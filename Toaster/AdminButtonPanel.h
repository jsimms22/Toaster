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
		const dpp::snowflake& userID,
		const std::string& rID);

	virtual ~AdminQueueButtonPanel() = default;

	void AddEmbed(const std::string& header, const std::string& description);
	void AddPageRow(const std::size_t page, const std::size_t size);

	static void AssignWorkersButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void RefreshButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void MarkCompleteButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void MarkOnHoldButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void AdminDelete(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);

	static void ShowWorkersButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event, const bool bSendFile = false);
	static void DownloadWorkersButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void DownloadArchiveButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void ReopenArchiveButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);

protected:
	dpp::component m_pagerow;
	dpp::component m_row2;
	dpp::component m_row3;
	// page row
	dpp::component m_btnNext;
	dpp::component m_btnPrev;

	// Row 2
	dpp::component m_btnRefreshPanel;
	dpp::component m_btnAssignWorkers;
	dpp::component m_btnMarkComplete;
	dpp::component m_btnMarkOnHold;
	dpp::component m_btnAdminDelete;

	// Row 3
	dpp::component m_btnShowWorkers;
	dpp::component m_btnDownloadWorkers;
	dpp::component m_btnDownloadArchive;
	dpp::component m_btnReopenArchived;

	dpp::snowflake m_userID;
	std::string m_ownerName;
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
	static void ChangeCooldown(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void ChangeMaxOpenRequests(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void ChangeArchiveThreshold(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void ChangeStallThreshold(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	
	// Row 3
	static void SendLogsButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void SendQueueButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void DeleteDataButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);

protected:
	dpp::component m_row;
	dpp::component m_row2;
	dpp::component m_row3;

	// Row 1
	dpp::component m_btnRefreshPanel;
	dpp::component m_btnChangeRole;
	dpp::component m_btnChangeChannel;
	dpp::component m_btnPingRules;

	// Row 2
	dpp::component m_btnChangeCooldown;
	dpp::component m_btnChangeMaxRequests;
	dpp::component m_btnChangeAutomatedArchive;
	dpp::component m_btnChangeAutomatedStall;

	// Row 3
	dpp::component m_btnSendLogs;
	dpp::component m_btnSendQueue;
	dpp::component m_btnDeleteData;

	dpp::snowflake m_userID;
};