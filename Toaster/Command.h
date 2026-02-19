#pragma once
#include <dpp/appcommand.h>

#include <string>
#include <vector>

struct BotModule
{
	// TODO: Set up some classes to encapsulate the various command actions
	using CommandList = std::vector<std::pair<std::string, dpp::slashcommand>>;
public:
	static CommandList commands;
};