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
#include <vector>
#include <set>

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

    bool IsBotOwner(const dpp::snowflake& user);

    bool IsWorkerOwner(const std::shared_ptr<JobRequest> job);
    bool IsRequestWorker(const dpp::snowflake& user, const std::shared_ptr<JobRequest> job);
    bool IsRequestOwner(const dpp::snowflake& user, const std::shared_ptr<JobRequest> job);

    bool HasPermission(const dpp::snowflake& user, dpp::permissions perm, const dpp::guild* guild);
    bool HasRole(const dpp::snowflake& user, const dpp::snowflake& role_id, const dpp::guild* guild);

    bool IsGuildMember(const dpp::snowflake& user, const dpp::guild* guild);
    bool IsGuildMemberWithRole(const dpp::snowflake& user, const dpp::snowflake& role_id, const dpp::guild* guild);
    bool IsMod(const dpp::snowflake& user, const dpp::guild* guild);
    bool IsGuildAdmin(const dpp::snowflake& user, const dpp::guild* guild);

    bool IsActiveWorker(const dpp::snowflake& user, const dpp::guild* guild);
    bool IsActiveWorker(const dpp::snowflake& user, const std::vector<dpp::snowflake>& workers);

    bool CanAssignJob(const dpp::snowflake& user, const std::shared_ptr<JobRequest>& job, const dpp::guild* guild);
    bool CanAssignJob(const dpp::snowflake& user, const std::shared_ptr<JobRequest>& job, const std::vector<dpp::snowflake> workers);
    bool CanEditJob(const dpp::snowflake& user, const std::shared_ptr<JobRequest>& job, const dpp::guild* guild);
    bool CanDeleteJob(const dpp::snowflake& user, const std::shared_ptr<JobRequest>& job, const dpp::guild* guild);
    bool CanAddNote(const dpp::snowflake& user, const std::shared_ptr<JobRequest>& job, const dpp::guild* guild);
    bool CanAccessAdminPanel(const dpp::snowflake& user, const std::shared_ptr<JobRequest>& job, const dpp::guild* guild);

private: // Members
    PermissionsMgr() = delete;  // Prevent instantiation
    // Allows Application to participate in its own lifetime
    static std::weak_ptr<PermissionsMgr> s_applicationInstance;
};