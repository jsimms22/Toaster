#include "PaginationPanel.h"

#include "CommandContext.h"
// fmt
#include <fmt/format.h>

PaginationPanel::PaginationPanel(
	CommandContext& ctx,
	const std::string& OwnerName,
	const std::size_t type,
	const std::size_t page,
	const std::size_t size,
	const std::size_t itemsPerPage,
	const dpp::snowflake& user)
	: dpp::message(), m_type{ type }, m_page{ page }, m_size{ size }
{
	m_lastPage = m_size <= 1 ? 0 : (m_size - 1) / itemsPerPage;

	const std::size_t prevPage = m_page > 0 ? m_page - 1 : 0;
	const std::size_t nextPage = m_page < m_lastPage ? m_page + 1 : m_lastPage;

	if (prevPage != nextPage)
	{
		m_btnPrev.set_type(dpp::cot_button)
			.set_label("Prev")
			.set_style(dpp::cos_primary)
			.set_id(fmt::format("{}:{}:{}:{}", OwnerName, m_type, prevPage, user));

		m_btnNext.set_type(dpp::cot_button)
			.set_label("Next")
			.set_style(dpp::cos_primary)
			.set_id(fmt::format("{}:{}:{}:{}", OwnerName, m_type, nextPage, user));

		m_row.add_component(m_btnPrev)
			.add_component(m_btnNext);

		add_component(m_row);
	}
}

void PaginationPanel::AddEmbed(const std::string& header, const std::string& description)
{
	dpp::embed embed;
	embed.set_title(fmt::format("{} (Page {} of {})", header, m_page + 1, m_lastPage + 1))
		.set_description(description)
		.set_color(0x3498db);

	add_embed(embed);
}

