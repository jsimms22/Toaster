#pragma once
#include "Resource.h"
// d++
#include <dpp/snowflake.h>
#include <dpp/cluster.h>
// microsoft
#include <guiddef.h>
// std library
#include <cstdlib>
#include <string>

namespace utils
{
	std::string LoadSecret(const std::string& filename, const std::string& find);
	int PriorityToString(const std::string& priority_str);
	std::string GuidToString(const GUID& guid);
	const GUID StringToGuid(const std::string& guidStr);
	std::string GuidToStringNoBrackets(const GUID& guid);
	const GUID CreateGUID();
	const std::size_t GetEpochTimestamp();
	const std::size_t CmdStringToJobType(const std::string& cmd);
	void NotifyIssuerMsg(dpp::cluster& cluster, const dpp::snowflake& idUser, const dpp::event_dispatch_t& event, const std::string& msg);
}