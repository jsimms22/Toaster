//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "GuildSettings.h"
#include "IJobRepo.h"
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
	ToasterBot(dpp::cluster& cluster, const uint32_t clusterId, const std::shared_ptr<IJobRepo>& repo, const bool bDebug = false);
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
	bool m_debug = false;

	// Discord details
	dpp::cluster& m_cluster;
	std::uint32_t m_clusterId = 0;
	std::uint32_t m_iShardCount = 0;

	// Guild Data
	void LoadGuildSettings();
	void SaveGuildSettings();
	GuildSettings& GetOrCreateSettings(const dpp::snowflake& guildID);
	std::unordered_map<dpp::snowflake, GuildSettings> g_settings;
	
	// Queue Data
	void LoadQueueData();
	std::shared_ptr<JobQueue> GetOrCreateQueue(const dpp::snowflake& guildID);
	std::unordered_map<dpp::snowflake, std::shared_ptr<JobQueue>> m_spQueue;

	// Database
	std::shared_ptr<IJobRepo> m_repo;
	void LoadDBQueueData();
};

