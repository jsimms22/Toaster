#pragma once
#include "Common.h"

#include <string>

namespace utils
{
	std::string LoadBotToken(const std::string& filename);
	int PriorityToString(const std::string& priority_str);
	std::string GuidToString(const GUID& guid);
	GUID StringToGuid(const std::string& guidStr);
	std::string GuidToStringNoBrackets(const GUID& guid);
	GUID CreateGUID();
}