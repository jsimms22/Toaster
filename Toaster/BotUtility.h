#pragma once
#include "Resource.h"
// microsoft
#include <guiddef.h>
// std library
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
}