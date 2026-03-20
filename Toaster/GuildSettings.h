#pragma once
#include "Resource.h"
// d++
#include <dpp/snowflake.h>
// tinyxml2
#include "tinyxml2.h"
// bsoncxx
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/types.hpp>
// std library
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <optional>

class IJobRepo;
struct CommandContext;

class GuildSettings : public std::enable_shared_from_this<GuildSettings>
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

	// Role display names in enum order
	static const std::array<const char*, 8> RoleNames;

	GuildSettings(const dpp::snowflake& guildID)
		: m_idGuild{ guildID }
	{
	}

	~GuildSettings() = default;

	// Channel Announcements
	std::optional<dpp::snowflake> idNewJobChannel;
	std::optional<dpp::snowflake> idUpdateJobChannel;
	std::optional<dpp::snowflake> idDeleteJobChannel;
	std::optional<dpp::snowflake> idCompleteJobChannel;

	// Slow Down
	std::chrono::seconds announcement_cooldown{ 0 };

	// Open request cap
	std::uint32_t requestLimitPerUser = 15;

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

	// increment by +1 for number of job types
	std::array<std::atomic<std::int32_t>, JOB_PLACEHOLDER> g_counter;

	// archival age
	std::chrono::hours archival_age{ 48 };
	std::chrono::hours stalled_age{ 96 };

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

	// Serialization + Deserialization: XML
	void WriteAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent) const;
	void ReadAttributes(tinyxml2::XMLElement* xmlNode, tinyxml2::XMLElement* xmlParent);

	// Serialization + Deserialization: BSON
	virtual bsoncxx::builder::basic::document WriteAttributesBSON() const;
	virtual void ReadAttributesBSON(const bsoncxx::document::view& doc);

	void SaveGuildSettings(std::shared_ptr<IJobRepo> repo);

	dpp::snowflake GetGuildID() const { return m_idGuild; }

private:
	dpp::snowflake m_idGuild;
};

