//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "GuildSettings.h"
#include "PermissionsMgr.h"
#include "Toaster.h"
// d++
#include <dpp/cluster.h>
// std library
#include <memory>
#include <vector>

class JobQueue;

//---------------------------------------------------------------------------------------------------------------------
/// \class 
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
struct CommandContext
{
	dpp::cluster& cluster;
	std::shared_ptr<JobQueue>& queue;
	GuildSettings& guild;
	bool debug = false;
};