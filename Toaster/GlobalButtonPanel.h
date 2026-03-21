#pragma once
// d++
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/snowflake.h>
// std library
#include <string>

struct CommandContext;

class GlobalButtonPanel :public dpp::message
{
public:
	GlobalButtonPanel(const std::string& rID, const bool bReopened);

	virtual ~GlobalButtonPanel() = default;

	void AddEmbed(const std::string& header, const std::string& description);

	static bool AssignButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static bool UnassignButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void AddNoteButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void ShowNotesButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);

protected:
	dpp::component m_row;

	dpp::component m_btnGlobalAssign;
	dpp::component m_btnGlobalUnassign;
	dpp::component m_btnGlobalAddNote;
	dpp::component m_btnGlobalShowNotes;

	std::string m_rID;
};
