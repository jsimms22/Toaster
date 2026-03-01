//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "GuildSettings.h"
// d++
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
// std library
#include <cstdlib>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class JobQueue;

//---------------------------------------------------------------------------------------------------------------------
/// \class ToasterBot
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class ToasterBot final
{
public:
	ToasterBot(dpp::cluster& cluster, const uint32_t clusterId, 
			   const std::shared_ptr<JobQueue>& spQueue, 
		       const bool bDebug = false);
	~ToasterBot() { SaveGuildSettings(); }

	// Bot event handlers
	void onReady(const dpp::ready_t& event);
	void onMessage(const dpp::message_create_t& event);
	void onSlashCommand(const dpp::slashcommand_t& event);
	void onButtonClick(const dpp::button_click_t& event);
	void onInteractionCreate(const dpp::interaction_create_t& event);
	void onFormSubmit(const dpp::form_submit_t& event);
	
private:
	mutable std::shared_mutex m_mtxShared;

	void LoadGuildSettings();
	void SaveGuildSettings();

	bool m_debug = false;

	std::shared_ptr<JobQueue> m_spQueue;

	// Discord details
	dpp::cluster& m_cluster;
	std::uint32_t m_clusterId = 0;
	std::uint32_t m_iShardCount = 0;
	const std::vector<dpp::snowflake> m_vWorkers = 
	{	
		464542267395538944, 
		332728115430162444,												  
		710847331871883294,																	  
		195997205864120320 
	};

	std::unordered_map<dpp::snowflake, GuildSettings> g_settings;

};

