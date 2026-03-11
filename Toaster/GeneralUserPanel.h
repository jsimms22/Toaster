#pragma once
// d++
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/snowflake.h>
// std library
#include <string>

struct CommandContext;

class GeneralUserPanel : public dpp::message
{
public:
	GeneralUserPanel(
		const std::string& OwnerName,
		const dpp::snowflake& userID,
		const std::string& jobID);

	virtual ~GeneralUserPanel() = default;

	void AddEmbed(const std::string& header, const std::string& description);

	static void EditButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void NoteButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void DeleteButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void ShowNotesButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);

protected:
	dpp::component m_row;
	dpp::component m_row2;
	dpp::component m_btnEdit;
	dpp::component m_btnNote;
	dpp::component m_btnDelete;
	dpp::component m_btnShowNotes;

	dpp::snowflake m_userID;
	std::string m_jobID;
};
