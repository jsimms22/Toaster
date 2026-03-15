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
	void AddPageRow(const std::size_t page, const std::size_t size);
	void AddCustomButton(const dpp::component btn, const std::size_t row);

	static void EditButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void NoteButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void DeleteButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);
	static void ShowNotesButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event);

protected:
	dpp::component m_pagerow;
	dpp::component m_row;
	dpp::component m_row2;

	dpp::component m_btnEdit;
	dpp::component m_btnNote;
	dpp::component m_btnDelete;
	dpp::component m_btnShowNotes;
	// page row
	dpp::component m_btnNext;
	dpp::component m_btnPrev;

	dpp::snowflake m_userID;
	std::string m_jobID;
	std::string m_ownerName;
};
