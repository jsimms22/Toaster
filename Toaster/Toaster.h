#pragma once
// d++
#include <dpp/dispatcher.h>
#include <dpp/cluster.h>
// std library
#include <cstdint>

class JobQueue;

class ToasterBot final
{
public:
	ToasterBot(dpp::cluster* cluster, const uint32_t clusterId, const std::shared_ptr<JobQueue>& spQueue, const bool bDebug = false);
	~ToasterBot() = default;

	// Bot event handlers
	void onReady(const dpp::ready_t& event);
	void onMessage(const dpp::message_create_t& event);
	void onSlashCommand(const dpp::slashcommand_t& event);
	void onButtonClick(const dpp::button_click_t& event);
	void onInteractionCreate(const dpp::interaction_create_t& event);
	void onFormSubmit(const dpp::form_submit_t& event);

	void NotifyIssuerMsg(const dpp::snowflake& userID, const dpp::event_dispatch_t& event, const std::string& msg);
	
private:
	bool m_debug = false;

	std::shared_ptr<JobQueue> m_spQueue;

	// Discord details
	dpp::cluster* m_cluster = nullptr;
	std::uint32_t m_clusterId = 0;
	std::uint32_t m_iShardCount = 0;
	const std::unordered_map<dpp::snowflake, std::string_view> m_vWorkers = { std::pair(464542267395538944, "aimx83"),
																			  std::pair(332728115430162444, "mike.d.spectre"),
																			  std::pair(710847331871883294, "linealign"),
																			  std::pair(195997205864120320, "leaf1318") };
};

