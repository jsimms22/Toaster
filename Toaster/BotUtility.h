#pragma once
#include "Resource.h"

#include <string>
// Microsoft
#include <guiddef.h>

namespace utils
{
	std::string LoadBotToken(const std::string& filename);
	int PriorityToString(const std::string& priority_str);
	std::string GuidToString(const GUID& guid);
	const GUID StringToGuid(const std::string& guidStr);
	std::string GuidToStringNoBrackets(const GUID& guid);
	const GUID CreateGUID();
	const std::size_t GetEpochTimestamp();
}