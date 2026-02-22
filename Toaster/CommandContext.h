#pragma once
// d++
#include <dpp/cluster.h>
// std library
#include <memory>

class JobQueue;

struct CommandContext
{
	dpp::cluster& cluster;
	std::shared_ptr<JobQueue> queue;
	bool debug = false;
	const std::unordered_map<dpp::snowflake, std::string_view>& workers;
};