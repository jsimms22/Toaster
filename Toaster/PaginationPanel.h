#pragma once
#include "Commands.h"
#include "CommandContext.h"
// d++
#include <dpp/message.h>
#include <dpp/snowflake.h>
// std library
#include <string>

struct CommandContext;

class PaginationPanel : public dpp::message
{
public:
	PaginationPanel(
		CommandContext& ctx,
		const std::string& OwnerName,
		const std::size_t type,
		const std::size_t page,
		const std::size_t size,
		const std::size_t itemsPerPage,
		const dpp::snowflake& user = 0);

	virtual ~PaginationPanel() = default;

	void AddEmbed(const std::string& header, const std::string& description);

private:
	dpp::component m_row;
	dpp::component m_btnNext;
	dpp::component m_btnPrev;

	std::size_t m_page;
	std::size_t m_lastPage;
	std::size_t m_type;
	std::size_t m_size;
};

