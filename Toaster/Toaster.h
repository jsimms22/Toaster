//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
// d++
#include <dpp/dispatcher.h>
#include <dpp/cluster.h>
// std library
#include <cstdint>
#include <memory>

class JobQueue;

//---------------------------------------------------------------------------------------------------------------------
/// \class ToasterBot
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class ToasterBot final
{
	using JobType = std::size_t;
	using WorkerList = std::vector<dpp::snowflake>;

	struct GuildJobSettings
	{
		std::unordered_map<JobType, WorkerList> mapWorkers;
		dpp::snowflake idNewJobChannel;
		dpp::snowflake idUpdateJobChannel;
		dpp::snowflake idCompleteJobChannel;
		std::chrono::seconds announcement_cooldown{ 0 };
		bool bPingOnNew{ true };
		bool bPingOnUpdates{ false };
		bool bPingOnCompleted{ false };

		std::optional<dpp::snowflake> ping_role_id;

		const WorkerList& GetWorkers(JobType type) const
		{
			const auto it = mapWorkers.find(type);
			if (it != mapWorkers.cend())
				return it->second;
			else
				return WorkerList{};
		}

		void RemoveWorker(const JobType type, const dpp::snowflake& user)
		{
			auto it = mapWorkers.find(type);
			if (it == mapWorkers.end())
				return;

			auto& workers = it->second;
			std::erase_if(workers, [&user](const dpp::snowflake& worker) { return worker == user; });
		}
		void SetWorker(const JobType type, const dpp::snowflake& user) { mapWorkers[type].push_back(user); }
		bool HasAnnouncementChannel() const { return (idNewJobChannel || idUpdateJobChannel || idCompleteJobChannel); };
		bool HasPingRole() const { return ping_role_id.has_value(); }
	};

public:
	ToasterBot(dpp::cluster& cluster, const uint32_t clusterId, 
			   const std::shared_ptr<JobQueue>& spQueue, 
		       const bool bDebug = false);
	~ToasterBot() = default;

	// Bot event handlers
	void onReady(const dpp::ready_t& event);
	void onMessage(const dpp::message_create_t& event);
	void onSlashCommand(const dpp::slashcommand_t& event);
	void onButtonClick(const dpp::button_click_t& event);
	void onInteractionCreate(const dpp::interaction_create_t& event);
	void onFormSubmit(const dpp::form_submit_t& event);
	
private:
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

	std::unordered_map<dpp::snowflake, GuildJobSettings> g_settings;

};

