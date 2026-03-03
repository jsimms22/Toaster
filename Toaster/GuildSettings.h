#pragma once
// d++
#include <dpp/snowflake.h>
// tinyxml2
#include "tinyxml2.h"
// std library
#include <array>
#include <cstdlib>
#include <chrono>
#include <optional>

struct CommandContext;

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
	std::optional<dpp::snowflake> idNewJobChannel;
	std::optional<dpp::snowflake> idUpdateJobChannel;
	std::optional<dpp::snowflake> idDeleteJobChannel;
	std::optional<dpp::snowflake> idCompleteJobChannel;

	// Slow Down
	std::chrono::seconds announcement_cooldown{ 0 };

	// Ping Rules
	bool bPingOnNew			{ true };
	bool bPingOnUpdate		{ false };
	bool bPingOnDelete		{ false };
	bool bPingOnComplete	{ false };

	// Roles - Order must correlate with enum order
	std::optional<dpp::snowflake> idPingRole;		///< Set a general ping role
	std::optional<dpp::snowflake> idCraftingRole;	///< Set role id for crafters
	std::optional<dpp::snowflake> idBuidlingRole;	///< Set role id for base builders
	std::optional<dpp::snowflake> idComponentRole;	///< Set role id for component dealers
	std::optional<dpp::snowflake> idResourceRole;	///< Set role id for resource gatherers
	std::optional<dpp::snowflake> idRefiningRole;	///< Set role id for refinery workers
	std::optional<dpp::snowflake> idHazardMatRole;	///< Set role id for hazardous material collectors
	std::optional<dpp::snowflake> idManagerRole;	///< Set role id for people managing the system

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
	bool HasAnnouncementChannel() const 
	{ 
		return (idNewJobChannel.has_value() || 
				idUpdateJobChannel.has_value() || 
				idCompleteJobChannel.has_value() ||
				idDeleteJobChannel.has_value()); 
	};

	bool HasPingRole() const { return roles[0].has_value(); }

	static std::optional<GuildSettings::Roles> JobTypeToRole(const std::size_t type);
	static std::optional<std::size_t> RoleToJobType(const GuildSettings::Roles type);

	static void AnnounceOnNew(CommandContext& ctx, const std::size_t jobType, const std::string& jobDetails);
	static void AnnounceOnUpdate(CommandContext& ctx, const std::size_t jobType, const std::string& jobDetails);
	static void AnnounceOnDelete(CommandContext& ctx, const std::size_t jobType, const std::string& jobDetails);
	static void AnnounceOnComplete(CommandContext& ctx, const std::size_t jobType, const std::string& jobDetails);

	void WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent) const;
	void ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent);

	void SaveGuildSettings(const dpp::snowflake guildID);
};

