#pragma once
// d++
#include <dpp/snowflake.h>
// std library
#include <array>
#include <cstdlib>
#include <chrono>
#include <optional>

class GuildSettings
{
public:
	enum class Roles : std::size_t
	{
		Ping = 0,
		Crafter = 1,
		Builder = 2,
		Comp = 3,
		Gatherer = 4,
		Refiner = 5,
		Hazmat = 6,
		Manager = 7
	};

	GuildSettings() = default;
	~GuildSettings() = default;

	// Channel Announcements
	dpp::snowflake idNewJobChannel = 1473505000239136911;
	dpp::snowflake idUpdateJobChannel = 1473505000239136911;
	dpp::snowflake idCompleteJobChannel = 1473505000239136911;

	// Slow Down
	std::chrono::seconds announcement_cooldown{ 0 };

	// Ping Rules
	bool bPingOnNew{ true };
	bool bPingOnUpdates{ false };
	bool bPingOnCompleted{ false };

	// Roles - Order must correlate with enum order
	std::optional<dpp::snowflake> idPingRole;								///< Ping users with this role
	std::optional<dpp::snowflake> idCraftingRole	= 1474121108432093234;	///< Set role id for crafters
	std::optional<dpp::snowflake> idBuidlingRole	= 1477553823735091230;	///< Set role id for base builders
	std::optional<dpp::snowflake> idComponentRole	= 1477553859562962944;	///< Set role id for component dealers
	std::optional<dpp::snowflake> idResourceRole	= 1477553919902093323;	///< Set role id for resource gatherers
	std::optional<dpp::snowflake> idRefiningRole	= 1477553919902093323;	///< Set role id for refinery workers
	std::optional<dpp::snowflake> idHazardMatRole	= 1477554129403379816;	///< Set role id for hazardous material collectors
	std::optional<dpp::snowflake> idManagerRole		= 1477554190803931248;	///< Set role id for people managing the system
	std::array<std::optional<dpp::snowflake>, 8> roles
	{
		idPingRole,
		idCraftingRole,
		idBuidlingRole,
		idComponentRole,
		idResourceRole,
		idRefiningRole,
		idHazardMatRole,
		idManagerRole
	};

	// Utility
	bool HasAnnouncementChannel() const { return (idNewJobChannel || idUpdateJobChannel || idCompleteJobChannel); };
	bool HasPingRole() const { return idPingRole.has_value(); }
	const GuildSettings::Roles JobTypeToRole(const std::size_t type) const;
};

