#include "PermissionsMgr.h"

#include "JobRequest.h"
#include "GuildSettings.h"
// std library
#include <algorithm>

std::weak_ptr<PermissionsMgr> PermissionsMgr::s_applicationInstance{};
std::mutex PermissionsMgr::mutex;

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
PermissionsMgr::PermissionsMgr(PermissionsMgr::Private /*p*/)
{
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<PermissionsMgr> PermissionsMgr::GetInstance()
{
    if (auto instance = s_applicationInstance.lock())
    {
        return instance;
    }
    else
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (!instance)
        {
            instance = std::make_shared<PermissionsMgr>(PermissionsMgr::Private());
            s_applicationInstance = instance;
        }

        return instance;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::IsBotOwner(const dpp::snowflake& user)
{
    return user == 464542267395538944;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::IsRequestWorker(const dpp::snowflake& user, const std::shared_ptr<const JobRequest>& job)
{
    if (!job) return false;
    return user == job->GetWorkerID();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::IsRequestOwner(const dpp::snowflake& user, const std::shared_ptr<const JobRequest>& job)
{
    if (!job) return false;
    return user == job->GetCustomerID();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::IsWorkerOwner(const std::shared_ptr<const JobRequest>& job)
{
    if (!job) return false;
    return job->GetCustomerID() == job->GetCustomerID();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::IsGuildMember(const dpp::snowflake& user, const dpp::guild* guild)
{
    if (!guild) return false;
    auto member_it = guild->members.find(user);
    return member_it != guild->members.end();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::HasRole(const dpp::guild_member& member, const std::optional<dpp::snowflake>& role_id)
{
    if (!role_id)
        return false;

    const auto& roles = member.get_roles();
    return std::find(roles.begin(), roles.end(), *role_id) != roles.end();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::IsMod(const dpp::snowflake& user, const dpp::interaction_create_t& event)
{
    dpp::permission perms = event.command.get_resolved_permission(user);
    if (perms.has(dpp::p_administrator) || perms.has(dpp::p_manage_guild))
        return true;

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::IsGuildAdmin(const dpp::snowflake& user, const dpp::interaction_create_t& event)
{
    dpp::permission perms = event.command.get_resolved_permission(user);
    if (perms.has(dpp::p_administrator))
        return true;

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::IsWorker(const dpp::snowflake& user, const dpp::guild* guild, const std::shared_ptr<const GuildSettings>& settings)
{
    if (!guild) return false;
    auto member_it = guild->members.find(user);
    if (member_it == guild->members.end())
        return false;

    const auto& member = member_it->second;

    const std::array<GuildSettings::Roles, 7> worker_roles =
    {
        GuildSettings::Roles::Ping,
        GuildSettings::Roles::Crafter,
        GuildSettings::Roles::Builder,
        GuildSettings::Roles::Comp,
        GuildSettings::Roles::Gatherer,
        GuildSettings::Roles::Refiner,
        GuildSettings::Roles::Hazmat
    };

    for (auto role : worker_roles)
    {
        if (HasRole(member, settings->roles[static_cast<size_t>(role)]))
        {
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::IsManager(const dpp::snowflake& user, const dpp::guild* guild, const std::shared_ptr<const GuildSettings>& settings)
{
    if (!guild) return false;
    auto member_it = guild->members.find(user);
    if (member_it == guild->members.end())
        return false;

    const auto& member = member_it->second;

    return HasRole( member, settings->roles[static_cast<std::size_t>(GuildSettings::Roles::Manager)]);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::CanCreateJob(
    const dpp::interaction_create_t& event,
    const dpp::snowflake& user,
    const dpp::guild* guild,
    const std::shared_ptr<const GuildSettings>& settings)
{
    if (!guild) return false;
    return IsGuildMember(user, guild) ||
           IsWorker(user, guild, settings) ||
           IsManager(user, guild, settings) ||
           IsGuildAdmin(user, event) ||
           IsMod(user, event) ||
           IsBotOwner(user);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::CanAssignJob(
    const dpp::interaction_create_t& event,
    const dpp::snowflake& user, 
    const std::shared_ptr<const JobRequest>& job,
    const dpp::guild* guild, 
    const std::shared_ptr<const GuildSettings>& settings)
{
    if (!job || !guild) return false;
    return IsRequestWorker(user, job) ||
           IsWorker(user, guild, settings) ||
           IsManager(user, guild, settings) ||
           IsGuildAdmin(user, event) ||
           IsMod(user, event) ||
           IsBotOwner(user);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::CanEditJob(
    const dpp::interaction_create_t& event,
    const dpp::snowflake& user, 
    const std::shared_ptr<const JobRequest>& job,
    const dpp::guild* guild, 
    const std::shared_ptr<const GuildSettings>& settings)
{
    if (!job || !guild) return false;
    return ((IsRequestOwner(user, job) && job->GetStatus() != JobRequest::status::active) &&
            (IsRequestOwner(user, job) && job->GetStatus() != JobRequest::status::complete)) ||
             IsRequestWorker(user, job) ||
             IsWorker(user, guild, settings) ||
             IsManager(user, guild, settings) ||
             IsGuildAdmin(user, event) ||
             IsMod(user, event) ||
             IsBotOwner(user);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::CanDeleteJob(
    const dpp::interaction_create_t& event,
    const dpp::snowflake& user, 
    const std::shared_ptr<const JobRequest>& job,
    const dpp::guild* guild, 
    const std::shared_ptr<const GuildSettings>& settings)
{
    if (!job || !guild) return false;
    return ((IsRequestOwner(user, job) && job->GetStatus() != JobRequest::status::active) &&
            (IsRequestOwner(user, job) && job->GetStatus() != JobRequest::status::complete)) ||
             IsRequestWorker(user, job) ||
             IsWorker(user, guild, settings) ||
             IsManager(user, guild, settings) ||
             IsGuildAdmin(user, event) ||
             IsMod(user, event) ||
             IsBotOwner(user);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::CanAddNote(
    const dpp::interaction_create_t& event,
    const dpp::snowflake& user, 
    const std::shared_ptr<const JobRequest>& job,
    const dpp::guild* guild, 
    const std::shared_ptr<const GuildSettings>& settings)
{
    if (!job || !guild) return false;
    return IsRequestOwner(user, job) ||
           IsRequestWorker(user, job) ||
           IsWorker(user, guild, settings) ||
           IsManager(user, guild, settings) ||
           IsGuildAdmin(user, event) ||
           IsMod(user, event) ||
           IsBotOwner(user);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::CanAccessAdminPanel(
    const dpp::interaction_create_t& event,
    const dpp::snowflake& user,
    const dpp::guild* guild,
    const std::shared_ptr<const GuildSettings>& settings)
{
    if (!guild) return false;
    return IsGuildAdmin(user, event) ||
           IsMod(user, event) ||
           IsManager(user, guild, settings) ||
           IsBotOwner(user);
}