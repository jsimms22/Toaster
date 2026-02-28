#include "PermissionsMgr.h"
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
bool PermissionsMgr::IsRequestWorker(const dpp::snowflake& user, const std::shared_ptr<JobRequest> job)
{
    if (!job) return false;
    return user == job->GetWorkerID();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::IsRequestOwner(const dpp::snowflake& user, const std::shared_ptr<JobRequest> job)
{
    if (!job) return false;
    return user == job->GetCustomerID();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::IsWorkerOwner(const std::shared_ptr<JobRequest> job)
{
    if (!job) return false;
    return job->GetCustomerID() == job->GetCustomerID();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::HasPermission(const dpp::snowflake& user, dpp::permissions perm, const dpp::guild* guild)
{
    if (!guild) return false;
    // todo logic
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::HasRole(const dpp::snowflake& user, const dpp::snowflake& role_id, const dpp::guild* guild)
{
    if (!guild) return false;
    // todo logic
    return false;
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
bool PermissionsMgr::IsGuildMemberWithRole(const dpp::snowflake& user, const dpp::snowflake& role_id, const dpp::guild* guild)
{
    if (!guild) return false;
    auto member_it = guild->members.find(user);
    if (member_it == guild->members.end())
        return false;

    for (const auto& member_role : member_it->second.get_roles())
    {
        if (member_role == role_id)
            return true;
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::IsMod(const dpp::snowflake& user, const dpp::guild* guild)
{
    if (!guild) return false;
    auto member_it = guild->members.find(user);
    if (member_it == guild->members.end())
        return false;

    for (const auto& member_role : member_it->second.get_roles())
    {
        // todo logic
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::IsGuildAdmin(const dpp::snowflake& user, const dpp::guild* guild)
{
    if (!guild) return false;
    auto member_it = guild->members.find(user);
    if (member_it == guild->members.end())
        return false;

    for (const auto& member_role : member_it->second.get_roles())
    {
        // todo logic
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::IsActiveWorker(const dpp::snowflake& user, const dpp::guild* guild)
{
    if (!guild) return false;
    auto member_it = guild->members.find(user);
    if (member_it == guild->members.end())
        return false;

    // todo logic

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::IsActiveWorker(const dpp::snowflake& user, const std::vector<dpp::snowflake>& workers)
{
    return workers.cend() != std::find_if(workers.cbegin(), 
                                          workers.cend(), 
                                          [&user](const dpp::snowflake& worker) -> bool { return worker == user; });
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::CanAssignJob(const dpp::snowflake& user, const std::shared_ptr<JobRequest>& job, const std::vector<dpp::snowflake> workers)
{
    if (!job) return false;
    return IsActiveWorker(user, workers) || 
           IsRequestWorker(user, job) || 
           IsBotOwner(user);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::CanAssignJob(const dpp::snowflake& user, const std::shared_ptr<JobRequest>& job, const dpp::guild* guild)
{
    if (!job || !guild) return false;
    return IsRequestWorker(user, job) ||
           IsGuildAdmin(user, guild) ||
           IsBotOwner(user);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::CanEditJob(const dpp::snowflake& user, const std::shared_ptr<JobRequest>& job, const dpp::guild* guild)
{
    if (!job || !guild) return false;
    return (IsRequestOwner(user, job) && job->GetStatus() < JobRequest::status::active) ||
           (IsRequestOwner(user, job) && job->GetStatus() == JobRequest::status::hold) ||
            IsRequestWorker(user, job) ||
            IsGuildAdmin(user, guild) ||
            IsBotOwner(user);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::CanDeleteJob(const dpp::snowflake& user, const std::shared_ptr<JobRequest>& job, const dpp::guild* guild)
{
    if (!job || !guild) return false;
    return (IsRequestOwner(user, job) && job->GetStatus() < JobRequest::status::active) ||
           (IsRequestOwner(user, job) && job->GetStatus() == JobRequest::status::hold) ||
            IsRequestWorker(user, job) ||
            IsGuildAdmin(user, guild) ||
            IsBotOwner(user);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::CanAddNote(const dpp::snowflake& user, const std::shared_ptr<JobRequest>& job, const dpp::guild* guild)
{
    if (!job || !guild) return false;
    return IsRequestOwner(user, job) ||
           IsRequestWorker(user, job) ||
           IsGuildAdmin(user, guild) ||
           IsBotOwner(user);
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
bool PermissionsMgr::CanAccessAdminPanel(const dpp::snowflake& user, const std::shared_ptr<JobRequest>& job, const dpp::guild* guild)
{
    if (!job || !guild) return false;
    return IsGuildAdmin(user, guild) ||
           IsBotOwner(user);
}