#pragma once
#include "Resource.h"
// d++
#include <dpp/cluster.h>
#include <dpp/snowflake.h>
#include <dpp/user.h>
// microsoft
#include <guiddef.h>
// std library
#include <cstdlib>
#include <cstdint>
#include <string>

namespace utils
{
	static constexpr std::size_t PAGE_SIZE{ 1500 };

	std::string LoadSecret(const std::string& filename, const std::string& find);
	int PriorityToString(const std::string& priority_str);
	std::string GuidToString(const GUID& guid);
	const GUID StringToGuid(const std::string& guidStr);
	std::string GuidToStringNoBrackets(const GUID& guid);
	const GUID CreateGUID();
	const std::size_t GetEpochTimestamp();
	const std::size_t CmdStringToJobType(const std::string& cmd);
	const std::string JobTypeToString(const std::size_t& type);
	void NotifyIssuerMsg(dpp::cluster& cluster, const dpp::snowflake& idUser, const dpp::event_dispatch_t& event, const std::string& msg);
	dpp::user FindUserByID(dpp::cluster& cluster, const dpp::snowflake& id);
	dpp::guild* FindGuildByID(dpp::cluster& cluster, const dpp::snowflake& id);
	std::vector<std::string> SplitIntoPages(const std::string& input, size_t max_len = 1500);
	std::vector<std::string> Split(const std::string& input, char delimiter);
}