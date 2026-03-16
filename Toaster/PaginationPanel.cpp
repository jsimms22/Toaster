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
	const bool bShowComplete,
	const dpp::snowflake& user)
	: dpp::message(), m_type{ type }, m_page{ page }, m_size{ size }, m_bShowComplete{ bShowComplete }
{
	m_lastPage = m_size <= 1 ? 0 : (m_size - 1) / itemsPerPage;

	const std::size_t prevPage = m_page > 0 ? m_page - 1 : 0;
	const std::size_t nextPage = m_page < m_lastPage ? m_page + 1 : m_lastPage;

	if (prevPage != nextPage)
	{
		m_btnPrev.set_type(dpp::cot_button)
			.set_label("Prev")
			.set_style(dpp::cos_primary)
			.set_id(fmt::format("{}_pagenext:{}:{}:{}:{}", OwnerName, m_type, prevPage, user, m_bShowComplete));

		m_btnNext.set_type(dpp::cot_button)
			.set_label("Next")
			.set_style(dpp::cos_primary)
			.set_id(fmt::format("{}_pageprev:{}:{}:{}:{}", OwnerName, m_type, nextPage, user, m_bShowComplete));

		m_btnShowComplete.set_type(dpp::cot_button)
			.set_label(m_bShowComplete ? "Hide Completed" : "Show Completed")
			.set_style(m_bShowComplete ? dpp::cos_secondary :dpp::cos_primary)
			.set_id(fmt::format("{}_pageshow:{}:{}:{}:{}", OwnerName, m_type, m_page, user, m_bShowComplete));


		m_row.add_component(m_btnPrev)
			.add_component(m_btnNext);

		if (OwnerName != Button_Unassigned && OwnerName != Button_Stalled)
		{
			m_row.add_component(m_btnShowComplete);
		}

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
