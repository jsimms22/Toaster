//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "Resource.h"
// d++
#include <dpp/cluster.h>
#include <dpp/snowflake.h>
#include <dpp/user.h>
// std library
#include <cstdlib>
#include <cstdint>
#include <string>

class GuildSettings;
class JobRequest;

namespace utils
{
	static constexpr std::size_t PAGE_SIZE{ 1500 };

	std::string LoadSecret(const std::string& filename, const std::string& find);
	int PriorityToString(const std::string& priority_str);
	const std::size_t GetEpochTimestamp();
	const std::size_t CmdStringToJobType(const std::string& cmd);
	const std::string JobTypeToString(const std::size_t& type);
	void NotifyIssuerMsg(dpp::cluster& cluster, const dpp::snowflake& idUser, const dpp::event_dispatch_t& event, const std::string& msg);
	void NotifyIssuerMsgWithEmbed(
		dpp::cluster& cluster,
		const dpp::event_dispatch_t& event,
		const dpp::snowflake& idUser,
		const std::string& msgContent,
		const std::string& embedHeader,
		const std::string& embedContent);
	dpp::user FindUserByID(dpp::cluster& cluster, const dpp::snowflake& id);
	dpp::guild* FindGuildByID(dpp::cluster& cluster, const dpp::snowflake& id);
	void FindGuildCallback(dpp::cluster& cluster, const dpp::snowflake& id, std::function<void(dpp::guild*)> callback);
	std::vector<std::pair<dpp::snowflake, std::string>> BuildWorkerList(dpp::guild* guild, const std::shared_ptr<const JobRequest>& job, const std::shared_ptr<const GuildSettings>& settings);
	const std::string FindPreferredNameByID(dpp::cluster& cluster, const dpp::snowflake& idUser, const dpp::snowflake& idGuild);
	std::vector<std::string> Split(const std::string& input, char delimiter);
	void RemoveChar(std::string& str, const char sym);
	void FilterWhiteSpace(std::string& str);
	void FilterUserString(std::string& str);
	void FilterCharacters(std::string& str);
	void RemoveHiddenLinks(std::string& str);
	void RemoveUnsafeProtocols(std::string& str);
}