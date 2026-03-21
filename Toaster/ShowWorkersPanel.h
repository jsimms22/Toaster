#pragma once
// d++
#include <dpp/dispatcher.h>
#include <dpp/message.h>
#include <dpp/snowflake.h>
// std library
#include <memory>
#include <string>

struct CommandContext;
class GuildSettings;

class ShowWorkersPanel : public dpp::message
{
public:
	ShowWorkersPanel(
		const std::string& OwnerName, 
		const std::shared_ptr<GuildSettings>& settings,
		const std::optional<dpp::snowflake> roleID);

	virtual ~ShowWorkersPanel() = default;

	void AddEmbed(const std::string& header, const std::string& description);

protected:
	dpp::component m_row;
	dpp::component m_row2;

	// Row 1
	dpp::component m_btnGeneral;
	dpp::component m_btnCrafters;
	dpp::component m_btnBuilders;
	dpp::component m_btnCompSuppliers;

	// Row 2
	dpp::component m_btnResource;
	dpp::component m_btnRefinery;
	dpp::component m_btnHazmat;
	dpp::component m_btnManager;

	std::string m_ownerName;
};

