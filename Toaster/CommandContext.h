//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
// d++
#include <dpp/cluster.h>
// std library
#include <memory>
#include <vector>

class JobQueue;
class GuildSettings;
class IJobRepo;

//---------------------------------------------------------------------------------------------------------------------
/// \class 
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
struct CommandContext
{
	dpp::cluster& cluster;
	std::shared_ptr<JobQueue> queue;
	std::shared_ptr<GuildSettings> guild;
	std::shared_ptr<IJobRepo> repo;
	bool debug = false;
};