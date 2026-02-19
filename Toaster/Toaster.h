#pragma once
#include <dpp/dispatcher.h>
#include <dpp/cluster.h>
#include <cstdint>

class JobQueue;

class ToasterBot final
{
public:
	ToasterBot(dpp::cluster* cluster, const uint32_t clusterId, const std::shared_ptr<JobQueue>& spQueue, const bool bDebug = false);
	~ToasterBot() = default;

	// Bot event handlers
	void onReady(const dpp::ready_t& event);
	void onSlashCommand(const dpp::slashcommand_t& event);
	void onInteractionCreate(const dpp::interaction_create_t& event);
	void onFormSubmit(const dpp::form_submit_t& event);

private:
	bool m_debug = false;

	std::shared_ptr<JobQueue> m_spQueue;

	// Discord details
	dpp::cluster* m_cluster = nullptr;
	std::uint32_t m_clusterId = 0;
	std::uint32_t m_iShardCount = 0;
};

