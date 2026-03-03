#include "BotUtility.h"
#include "GuildSettings.h"
#include "PermissionsMgr.h"
#include "JobRequest.h"
// microsoft
#include <objbase.h>
// fmt
#include <fmt/format.h>
// std library
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <future>

namespace utils
{
    //---------------------------------------------------------------------------------------------------------------------
    // \brief
    //---------------------------------------------------------------------------------------------------------------------
    std::string LoadSecret(const std::string& filename, const std::string& find)
    {
        std::ifstream file(filename);
        std::string line;

        while (std::getline(file, line))
        {
            if (line.find(find + "=") == 0)
            {
                return line.substr(std::string(find + "=").length());
            }
        }

        throw std::runtime_error(fmt::format("[{}] not found in config file",find));
    }

    //---------------------------------------------------------------------------------------------------------------------
    // \brief Helper function to convert string priority (e.g., "low", "med", "high") to numeric values
    //---------------------------------------------------------------------------------------------------------------------
    int PriorityToString(const std::string& priority_str) 
    {
        if (priority_str == "critical") return 4;
        if (priority_str == "high") return 3;
        if (priority_str == "med") return 2;
        if (priority_str == "low") return 1;
        return -1;  // Default to -1 for unknown priorities
    }

    //---------------------------------------------------------------------------------------------------------------------
    // \brief
    //---------------------------------------------------------------------------------------------------------------------
    std::string GuidToString(const GUID& guid) 
    {
        char guidString[39]; // 36 characters + null terminator

        // Format the GUID as a string "{xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx}"
        snprintf(guidString, sizeof(guidString),
            "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
            guid.Data1, guid.Data2, guid.Data3,
            guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
            guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

        return std::string(guidString);
    }

    //---------------------------------------------------------------------------------------------------------------------
    // \brief
    //---------------------------------------------------------------------------------------------------------------------
    std::string GuidToStringNoBrackets(const GUID& guid)
    {
        char guidString[37]; // 36 characters + null terminator

        // Format the GUID as a string "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
        snprintf(guidString, sizeof(guidString),
            "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            guid.Data1, guid.Data2, guid.Data3,
            guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
            guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

        return std::string(guidString);
    }

    //---------------------------------------------------------------------------------------------------------------------
    // \brief
    //---------------------------------------------------------------------------------------------------------------------
    const GUID StringToGuid(const std::string& guidStr)
    {
        GUID guid;
        // Convert the GUID string to a GUID structure using IIDFromString
        HRESULT hr = IIDFromString(std::wstring(guidStr.begin(), guidStr.end()).c_str(), &guid);

        // Check if the conversion was successful
        if (FAILED(hr))
        {
            throw std::invalid_argument("Invalid GUID string format");
        }

        return guid;
    }

    //---------------------------------------------------------------------------------------------------------------------
    // \brief
    //---------------------------------------------------------------------------------------------------------------------
    const GUID CreateGUID() 
    {
        GUID guid;
        HRESULT hr = CoCreateGuid(&guid);

        // Check if the conversion was successful
        if (FAILED(hr))
        {
            throw std::invalid_argument("Invalid GUID string format");
        }

        return guid;
    }

    //---------------------------------------------------------------------------------------------------------------------
    // \brief
    //---------------------------------------------------------------------------------------------------------------------
    const std::size_t GetEpochTimestamp()
    {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    //---------------------------------------------------------------------------------------------------------------------
    // \brief
    //---------------------------------------------------------------------------------------------------------------------
    const std::size_t CmdStringToJobType(const std::string& cmd)
    {
        if (cmd == Option_ItemCrafting) return JOB_TYPE_CRAFTING;
        else if (cmd == Option_BaseBuidling) return JOB_TYPE_BUILDING;
        else if (cmd == Option_ComponentRequest) return JOB_TYPE_COMPONENT;
        else if (cmd == Option_ResourceCollect) return JOB_TYPE_RESOURCE;
        else if (cmd == Option_RefineryJob) return JOB_TYPE_REFINERY;
        else return JOB_TYPE_GENERAL;
    }

    //---------------------------------------------------------------------------------------------------------------------
    // \brief
    //---------------------------------------------------------------------------------------------------------------------
    const std::string JobTypeToString(const std::size_t& type)
    {
        switch (type) {
            case (JOB_TYPE_CRAFTING):       return "Crafting";
            case (JOB_TYPE_BUILDING):   return "Basse Building";
            case (JOB_TYPE_COMPONENT):  return "Component Request";
            case (JOB_TYPE_RESOURCE):   return "Resource Collection";
            case (JOB_TYPE_REFINERY):   return "Refinery Job";
            default: return "General";
        }
    }

    //---------------------------------------------------------------------------------------------------------------------
    // \brief
    //---------------------------------------------------------------------------------------------------------------------
    void NotifyIssuerMsg(dpp::cluster& cluster, const dpp::snowflake& idUser, const dpp::event_dispatch_t& event, const std::string& msg)
    {
        cluster.direct_message_create(idUser, dpp::message(msg), [&cluster, &idUser](const dpp::confirmation_callback_t& callback) {
            if (callback.is_error())
            {
                cluster.log(dpp::ll_error, fmt::format("Error sending direct message to user id '{}'.", idUser));
            }
            else
            {
                dpp::user user = utils::FindUserByID(cluster, idUser);
                cluster.log(dpp::ll_info, fmt::format("Sending direct message to '{}'.", user.global_name));
            }});
    }

    //---------------------------------------------------------------------------------------------------------------------
    // \brief
    //---------------------------------------------------------------------------------------------------------------------
    dpp::user FindUserByID(dpp::cluster& cluster, const dpp::snowflake& id)
    {
        if (id == USERID_NULL)
        {
            cluster.log(dpp::ll_debug, fmt::format("Skipped look up for user id '{}'.", id));
            return {};
        }
        else
        {
            cluster.log(dpp::ll_debug, fmt::format("Retrieving user data for id '{}'.", id));
        }

        std::promise<dpp::user> promise;
        std::future<dpp::user> future = promise.get_future();
        dpp::user* user = dpp::find_user(id);
        if (user)
        {
            cluster.log(dpp::ll_debug, fmt::format("Found cached user data '{}'.", user->global_name));
            promise.set_value(*user);
            return future.get();
        }

        cluster.user_get(id, [&promise](const dpp::confirmation_callback_t& callback)
            {
                if (!callback.is_error()) 
                {
                    dpp::user_identified user = std::get<dpp::user_identified>(callback.value);
                    callback.bot->log(dpp::ll_debug, fmt::format("Found user data '{}'.", user.global_name));
                    promise.set_value(user);

                    // Manually cache the user
                    dpp::user* cache_user = new dpp::user(user);
                    dpp::get_user_cache()->store(cache_user);
                }
                else 
                {
                    promise.set_value({});  // Handle error case
                }
            });

        return future.get();
    }

    //---------------------------------------------------------------------------------------------------------------------
    // \brief
    //---------------------------------------------------------------------------------------------------------------------
    dpp::guild* FindGuildByID(dpp::cluster& cluster, const dpp::snowflake& id)
    {
        if (id == USERID_NULL)
        {
            cluster.log(dpp::ll_debug, fmt::format("Skipped look up for guild id '{}'.", id));
            return {};
        }
        else
        {
            cluster.log(dpp::ll_debug, fmt::format("Retrieving guild data for id '{}'.", id));
        }

        std::promise<dpp::guild*> promise;
        std::future<dpp::guild*> future = promise.get_future();
        dpp::guild* guild = dpp::find_guild(id);
        if (guild)
        {
            cluster.log(dpp::ll_debug, fmt::format("Found cached guild data '{}'.", guild->name));
            promise.set_value(guild);
            return future.get();
        }

        cluster.guild_get(id, [&promise, id](const dpp::confirmation_callback_t& callback)
            {
                if (!callback.is_error())
                {
                    dpp::guild response = std::get<dpp::guild>(callback.value);
                    callback.bot->log(dpp::ll_debug, fmt::format("Found guild data '{}'.", response.name));
                    promise.set_value(dpp::find_guild(id));
                }
                else
                {
                    promise.set_value({});  // Handle error case
                }
            });

        return future.get();
    }

    //---------------------------------------------------------------------------------------------------------------------
    // \brief
    //---------------------------------------------------------------------------------------------------------------------
    void FindGuildCallback(dpp::cluster& cluster, const dpp::snowflake& id, std::function<void(dpp::guild*)> callback)
    {
        if (id == USERID_NULL)
        {
            callback(nullptr);
            return;
        }

        if (auto* guild = dpp::find_guild(id))
        {
            callback(guild);
            return;
        }

        cluster.guild_get(id,
            [callback](const dpp::confirmation_callback_t& cc)
            {
                if (!cc.is_error())
                {
                    auto guild = std::get<dpp::guild>(cc.value);

                    // Now properly in DPP cache
                    callback(dpp::find_guild(guild.id));
                }
                else
                {
                    callback(nullptr);
                }
            });
    }

    //---------------------------------------------------------------------------------------------------------------------
    // \brief
    //---------------------------------------------------------------------------------------------------------------------
    std::vector<std::pair<dpp::snowflake, std::string>> BuildWorkerList(dpp::guild* guild, const std::shared_ptr<const JobRequest>& job, const GuildSettings& settings)
    {
        if (!guild)
            return {};

        std::vector<std::pair<dpp::snowflake, std::string>> vWorkerList;

        auto roleForType = settings.JobTypeToRole(job->JobType());
        if (!roleForType.has_value())
            return {};

        auto role_id = settings.roles[static_cast<std::size_t>(roleForType.value())];

        const auto pManager = PermissionsMgr::GetInstance();
        if (!role_id || !pManager)
            return {};

        for (const auto& member_pair : guild->members)
        {
            const auto& member = member_pair.second;

            if (pManager->HasRole(member, role_id))
            {
                vWorkerList.push_back({
                    member.user_id,
                    member.get_nickname().empty()
                        ? member.get_user()->username
                        : member.get_nickname()
                    });
            }
        }

        std::sort(vWorkerList.begin(), vWorkerList.end(),
            [](const std::pair<dpp::snowflake, std::string>& a, const std::pair<dpp::snowflake, std::string>& b)
            {
                return a.second < b.second;
            });

        return vWorkerList;
    }

    //---------------------------------------------------------------------------------------------------------------------
    // \brief
    //---------------------------------------------------------------------------------------------------------------------
    const std::string FindPreferredNameByID(dpp::cluster& cluster, const dpp::snowflake& idUser, const dpp::snowflake& idGuild)
    {
        dpp::user customer;
        std::string nickname;
        std::string global;
        if (idGuild)
        {
            // Fetch the guild from the cluster (check if the guild is cached)
            dpp::guild* guild = utils::FindGuildByID(cluster, idGuild);
            if (!guild)
            {
                // Handle case where guild is not found (maybe return the global username or an error)
                customer = utils::FindUserByID(cluster, idUser);
                global = customer.global_name;
            }
            else
            {
                // Check if the user is in the guild
                const auto member = guild->members.find(idUser);
                if (member != guild->members.end())
                {
                    // If the user has a nickname in the guild, return it, else return the global username
                    customer = *(member->second.get_user());
                    nickname = member->second.get_nickname();
                }
                else
                {
                    customer = utils::FindUserByID(cluster, idUser);
                    global = customer.global_name;
                }
            }
        }

        // If no nickname, check global, if no global, return their actual username
        return !nickname.empty() ? nickname : !global.empty() ? global : customer.username;
    }

    //---------------------------------------------------------------------------------------------------------------------
    // \brief
    //---------------------------------------------------------------------------------------------------------------------
    std::vector<std::string> SplitIntoPages(const std::string& input, size_t max_len)
    {
        std::vector<std::string> pages;
        const size_t page_count = (input.size() + max_len - 1) / max_len;
        pages.reserve(page_count); 

        for (size_t i = 0; i < input.length(); i += max_len) 
        {
            pages.push_back(input.substr(i, max_len));
        }

        return pages;
    }

    //---------------------------------------------------------------------------------------------------------------------
    // \brief
    //---------------------------------------------------------------------------------------------------------------------
    std::vector<std::string> Split(const std::string& input, char delimiter)
    {
        std::vector<std::string> result;

        size_t start = 0;
        size_t end = 0;

        while ((end = input.find(delimiter, start)) != std::string::npos)
        {
            result.emplace_back(input.substr(start, end - start));
            start = end + 1;
        }

        result.emplace_back(input.substr(start));

        return result;
    }

    //---------------------------------------------------------------------------------------------------------------------
    // \brief
    //---------------------------------------------------------------------------------------------------------------------
    void RemoveChar(std::string& str, const char sym)
    {
        str.erase(std::remove(str.begin(), str.end(), sym), str.end());
    }

    //---------------------------------------------------------------------------------------------------------------------
    // \brief
    //---------------------------------------------------------------------------------------------------------------------
    void FilterWhiteSpace(std::string& str)
    {
        str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());
    }

    //---------------------------------------------------------------------------------------------------------------------
    // \brief
    //---------------------------------------------------------------------------------------------------------------------
    void FilterUserString(std::string& str)
    {
        const std::vector<char> filterList{
            '\n', '\r', '\t',               // Newlines, carriage returns, tabs
            '`', '*', '~', '@', '<', '>',   // Discord formatting characters
            '|',                            // Potential table formatting
        };

        for (const auto sym : filterList)
            RemoveChar(str, sym);
    }
}