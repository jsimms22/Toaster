#include "ShowWorkersPanel.h"

#include "BotUtility.h"
#include "CommandContext.h"
#include "GuildSettings.h"
// fmt
#include <fmt/format.h>
// std library
#include <optional>

ShowWorkersPanel::ShowWorkersPanel(
	const std::string& OwnerName, 
	const std::shared_ptr<GuildSettings>& settings,
	const std::optional<dpp::snowflake> roleID)
	: dpp::message(), m_ownerName{ OwnerName }
{
	// row 1
	const auto generalID = settings->roles[static_cast<std::size_t>(GuildSettings::Roles::Ping)];
	const auto crafterID = settings->roles[static_cast<std::size_t>(GuildSettings::Roles::Crafter)];
	const auto builderID = settings->roles[static_cast<std::size_t>(GuildSettings::Roles::Builder)];
	const auto compID = settings->roles[static_cast<std::size_t>(GuildSettings::Roles::Comp)];

	if (generalID.has_value())
	{
		m_btnGeneral.set_type(dpp::cot_button)
			.set_label("General Workers")
			.set_style(roleID.has_value() && roleID == generalID ? dpp::cos_primary : dpp::cos_secondary)
			.set_id(fmt::format("{}_showgeneralworkers:{}:{}", m_ownerName, generalID.value(), static_cast<std::size_t>(GuildSettings::Roles::Ping)));
		m_row.add_component(m_btnGeneral);
	}

	if (crafterID.has_value())
	{
		m_btnCrafters.set_type(dpp::cot_button)
			.set_label("Item Crafters")
			.set_style(roleID.has_value() && roleID == crafterID ? dpp::cos_primary : dpp::cos_secondary)
			.set_id(fmt::format("{}_showcrafters:{}:{}", m_ownerName, crafterID.value(), static_cast<std::size_t>(GuildSettings::Roles::Crafter)));
		m_row.add_component(m_btnCrafters);
	}

	if (builderID.has_value())
	{
		m_btnBuilders.set_type(dpp::cot_button)
			.set_label("Base Builders")
			.set_style(roleID.has_value() && roleID == builderID ? dpp::cos_primary : dpp::cos_secondary)
			.set_id(fmt::format("{}_showbuilders:{}:{}", m_ownerName, builderID.value(), static_cast<std::size_t>(GuildSettings::Roles::Builder)));
		m_row.add_component(m_btnBuilders);
	}

	if (compID.has_value())
	{
		m_btnCompSuppliers.set_type(dpp::cot_button)
			.set_label("Component Suppliers")
			.set_style(roleID.has_value() && roleID == compID ? dpp::cos_primary : dpp::cos_secondary)
			.set_id(fmt::format("{}_showcompsupply:{}:{}", m_ownerName, compID.value(), static_cast<std::size_t>(GuildSettings::Roles::Comp)));
		m_row.add_component(m_btnCompSuppliers);
	}

	// row 2
	const auto resourceID = settings->roles[static_cast<std::size_t>(GuildSettings::Roles::Gatherer)];
	const auto refinerID = settings->roles[static_cast<std::size_t>(GuildSettings::Roles::Refiner)];
	const auto hazmatID = settings->roles[static_cast<std::size_t>(GuildSettings::Roles::Hazmat)];
	const auto managerID = settings->roles[static_cast<std::size_t>(GuildSettings::Roles::Manager)];

	if (resourceID.has_value())
	{
		m_btnResource.set_type(dpp::cot_button)
			.set_label("Resource Gatherers")
			.set_style(roleID.has_value() && roleID == resourceID ? dpp::cos_primary : dpp::cos_secondary)
			.set_id(fmt::format("{}_showgatherers:{}:{}", m_ownerName, resourceID.value(), static_cast<std::size_t>(GuildSettings::Roles::Gatherer)));
		m_row2.add_component(m_btnResource);
	}

	if (refinerID.has_value())
	{
		m_btnRefinery.set_type(dpp::cot_button)
			.set_label("Refinery Workers")
			.set_style(roleID.has_value() && roleID == refinerID ? dpp::cos_primary : dpp::cos_secondary)
			.set_id(fmt::format("{}_showrefiners:{}:{}", m_ownerName, refinerID.value(), static_cast<std::size_t>(GuildSettings::Roles::Refiner)));
		m_row2.add_component(m_btnRefinery);
	}

	if (hazmatID.has_value())
	{
		m_btnHazmat.set_type(dpp::cot_button)
			.set_label("Hazard Materials")
			.set_style(roleID.has_value() && roleID == hazmatID ? dpp::cos_primary : dpp::cos_secondary)
			.set_id(fmt::format("{}_showhazmatworkers:{}:{}", m_ownerName, hazmatID.value(), static_cast<std::size_t>(GuildSettings::Roles::Hazmat)));
		m_row2.add_component(m_btnHazmat);
	}

	if (managerID.has_value())
	{
		m_btnHazmat.set_type(dpp::cot_button)
			.set_label("Managers")
			.set_style(roleID.has_value() && roleID == managerID ? dpp::cos_primary : dpp::cos_secondary)
			.set_id(fmt::format("{}_showmanagers:{}:{}", m_ownerName, managerID.value(), static_cast<std::size_t>(GuildSettings::Roles::Manager)));
		m_row2.add_component(m_btnHazmat);
	}

	add_component(m_row);
	add_component(m_row2);
}

void ShowWorkersPanel::AddEmbed(const std::string& header, const std::string& description)
{
	dpp::embed embed;
	embed.set_title(fmt::format("{}", header))
		.set_description(description)
		.set_color(0x3498db);

	add_embed(embed);
}