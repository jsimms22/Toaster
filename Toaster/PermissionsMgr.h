//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief
//---------------------------------------------------------------------------------------------------------------------
#pragma once
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
class JobRequest;

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
    bool IsWorkerOwner(const std::shared_ptr<const JobRequest>& job);
    bool IsRequestWorker(const dpp::snowflake& user, const std::shared_ptr<const JobRequest>& job);
    bool IsRequestOwner(const dpp::snowflake& user, const std::shared_ptr<const JobRequest>& job);

    // Guild specific perms
    bool IsGuildMember(const dpp::snowflake& user, const dpp::guild* guild);
    bool HasRole(const dpp::guild_member& member, const std::optional<dpp::snowflake>& role_id);
    bool IsWorker(const dpp::snowflake& user, const dpp::guild* guild, const std::shared_ptr<const GuildSettings>&settings);
    bool IsManager(const dpp::snowflake& user, const dpp::guild* guild, const std::shared_ptr<const GuildSettings>&settings);
    bool IsMod(const dpp::snowflake& user, const dpp::interaction_create_t& event);
    bool IsGuildAdmin(const dpp::snowflake& user, const dpp::interaction_create_t& event);
    
    // Actions perms based on the above methods
    bool CanCreateJob(const dpp::interaction_create_t& event, const dpp::snowflake& user, const dpp::guild* guild, const std::shared_ptr<const GuildSettings>&settings);
    bool CanAssignJob(const dpp::interaction_create_t& event, const dpp::snowflake& user, const std::shared_ptr<const JobRequest>& job, const dpp::guild* guild, const std::shared_ptr<const GuildSettings>&settings);
    bool CanEditJob(const dpp::interaction_create_t& event, const dpp::snowflake& user, const std::shared_ptr<const JobRequest>& job, const dpp::guild* guild, const std::shared_ptr<const GuildSettings>& settings);
    bool CanDeleteJob(const dpp::interaction_create_t& event, const dpp::snowflake& user, const std::shared_ptr<const JobRequest>& job, const dpp::guild* guild, const std::shared_ptr<const GuildSettings>&settings);
    bool CanAddNote(const dpp::interaction_create_t& event, const dpp::snowflake& user, const std::shared_ptr<const JobRequest>& job, const dpp::guild* guild, const std::shared_ptr<const GuildSettings>&settings);
    bool CanAccessAdminPanel(const dpp::interaction_create_t& event, const dpp::snowflake& user, const dpp::guild* guild, const std::shared_ptr<const GuildSettings>&settings);

private: // Members
    PermissionsMgr() = delete;  // Prevent instantiation
    // Allows Application to participate in its own lifetime
    static std::weak_ptr<PermissionsMgr> s_applicationInstance;
};