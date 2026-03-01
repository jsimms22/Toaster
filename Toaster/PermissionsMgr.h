//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "JobRequest.h"
// d++
#include <dpp/cluster.h>
#include <dpp/guild.h>
#include <dpp/snowflake.h>
// std library
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <set>

class GuildSettings;

//---------------------------------------------------------------------------------------------------------------------
/// \class ToasterBot
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class PermissionsMgr
{
    struct Private { explicit Private() = default; };
public:
    // Private Constructor
    explicit PermissionsMgr(Private /*p*/);

    static std::shared_ptr<PermissionsMgr> GetInstance();
    static std::mutex mutex;

    // Non-Guild perms
    bool IsBotOwner(const dpp::snowflake& user);
    bool IsWorkerOwner(const std::shared_ptr<JobRequest> job);
    bool IsRequestWorker(const dpp::snowflake& user, const std::shared_ptr<JobRequest> job);
    bool IsRequestOwner(const dpp::snowflake& user, const std::shared_ptr<JobRequest> job);

    // Guild specific perms
    bool IsGuildMember(const dpp::snowflake& user, const dpp::guild* guild);
    bool HasRole(const dpp::guild_member& member, const std::optional<dpp::snowflake>& role_id);
    bool IsWorker(const dpp::snowflake& user, const dpp::guild* guild, const GuildSettings& settings);
    bool IsManager(const dpp::snowflake& user, const dpp::guild* guild, const GuildSettings& settings);
    bool IsMod(const dpp::snowflake& user, const dpp::guild* guild);
    bool IsGuildAdmin(const dpp::snowflake& user, const dpp::guild* guild);
    
    // Actions perms based on the above methods
    bool CanCreateJob(const dpp::snowflake& user, const dpp::guild* guild, const GuildSettings& settings);
    bool CanAssignJob(const dpp::snowflake& user, const std::shared_ptr<JobRequest>& job, const dpp::guild* guild, const GuildSettings& settings);
    bool CanEditJob(const dpp::snowflake& user, const std::shared_ptr<JobRequest>& job, const dpp::guild* guild, const GuildSettings& settings);
    bool CanDeleteJob(const dpp::snowflake& user, const std::shared_ptr<JobRequest>& job, const dpp::guild* guild, const GuildSettings& settings);
    bool CanAddNote(const dpp::snowflake& user, const std::shared_ptr<JobRequest>& job, const dpp::guild* guild, const GuildSettings& settings);
    bool CanAccessAdminPanel(const dpp::snowflake& user, const dpp::guild* guild, const GuildSettings& settings);

private: // Members
    PermissionsMgr() = delete;  // Prevent instantiation
    // Allows Application to participate in its own lifetime
    static std::weak_ptr<PermissionsMgr> s_applicationInstance;
};