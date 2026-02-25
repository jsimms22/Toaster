#pragma once
#include "PermissionsMgr.h"
// d++
#include <dpp/cluster.h>
// std library
#include <memory>
#include <vector>

class JobQueue;

struct CommandContext
{
	dpp::cluster& cluster;
	std::shared_ptr<JobQueue> queue;
	bool debug = false;
	const std::vector<dpp::snowflake>& workers;
	std::shared_ptr<PermissionsMgr> manager;
};