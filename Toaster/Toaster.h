//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
// d++
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
// std library
#include <cstdlib>
#include <cstdint>
#include <memory>
#include <shared_mutex>
#include <stop_token>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

class IJobRepo;
class JobQueue;
class GuildSettings;

//---------------------------------------------------------------------------------------------------------------------
/// \class ToasterBot
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class ToasterBot final
{
public:
	ToasterBot(dpp::cluster& cluster, const uint32_t clusterId, const std::shared_ptr<IJobRepo>& repo, const bool bDebug = false);
	~ToasterBot();

	// Bot event handlers
	void onReady(const dpp::ready_t& event);
	void onMessage(const dpp::message_create_t& event);
	void onSlashCommand(const dpp::slashcommand_t& event);
	void onButtonClick(const dpp::button_click_t& event);
	void onInteractionCreate(const dpp::interaction_create_t& event);
	void onFormSubmit(const dpp::form_submit_t& event);

private:
	mutable std::shared_mutex m_mtxGuildShared;
	mutable std::shared_mutex m_mtxQueueShared;
	bool m_debug = false;

	// Archival worker
	std::jthread m_worker;
	void AutomatedBotTasks(std::stop_token stopToken);

	// Discord details
	dpp::cluster& m_cluster;
	std::uint32_t m_clusterId = 0;
	std::uint32_t m_iShardCount = 0;

	// Guild Data
	std::shared_ptr<GuildSettings> GetOrCreateSettings(const dpp::snowflake& guildID);
	std::unordered_map<dpp::snowflake, std::shared_ptr<GuildSettings>> g_settings;
	
	// Queue Data
	std::shared_ptr<JobQueue> GetOrCreateQueue(const dpp::snowflake& guildID);
	std::unordered_map<dpp::snowflake, std::shared_ptr<JobQueue>> m_spQueue;

	// Database
	std::shared_ptr<IJobRepo> m_repo;
	void LoadDatabase();
};

