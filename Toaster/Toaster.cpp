#include "Toaster.h"

#include "IJobRepo.h"
#include "BotUtility.h"
#include "CommandContext.h"
#include "Commands.h"
#include "GuildSettings.h"
#include "JobQueue.h"
#include "Resource.h"
// d++
#include <dpp/channel.h>
#include <dpp/dpp.h>
#include <dpp/snowflake.h>
// fmt
#include <fmt/base.h>
#include <fmt/format.h>
// tinyxml
#include "tinyxml2.h"
// microsoft
#include <guiddef.h>
// std library
#include <chrono>

//---------------------------------------------------------------------------------------------------------------------
// \brief Constructor
//---------------------------------------------------------------------------------------------------------------------
ToasterBot::ToasterBot(dpp::cluster& cluster, const uint32_t clusterId, const std::shared_ptr<IJobRepo>& repo, const bool bDebug)
    : m_cluster(cluster), m_clusterId(clusterId), m_iShardCount{ 0 }, m_repo{ repo }, m_debug{ bDebug }
{
    LoadDatabase();
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void ToasterBot::onReady(const dpp::ready_t& event)
{
    if (dpp::run_once<struct clear_bot_commands>()) 
    {
        //ICustomCommand::UnregisterAll(&m_cluster);
        //ICustomCommand::UnregisterGuildAll(&m_cluster, 1472034166869852287);
    }
    if (dpp::run_once<struct register_bot_commands>())
    {
        ICustomCommand::RegisterAll(&m_cluster, Toaster::BotCommands);
        //ICustomCommand::RegisterGuildAll(&m_cluster, 1472034166869852287, Toaster::BotCommands);
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void ToasterBot::onMessage(const dpp::message_create_t& event)
{
    if (!event.msg.author.id) 
    {
        m_cluster.log(dpp::ll_error, fmt::format("Message dropped. No author '{}'.", event.msg.content));
        return;
    } 
    else if (event.msg.author.id == m_cluster.me.id) 
    {
        dpp::channel* channel = dpp::find_channel(event.msg.channel_id);
        if (channel && channel->get_type() == dpp::channel_type::DM)
        {
            for (auto& idUser : channel->recipients)
            {
                dpp::user user = utils::FindUserByID(m_cluster, idUser);
                m_cluster.log(dpp::ll_info, fmt::format("Sending outgoing direct message to '{}'.", user.global_name));
            }
        }
        else if (channel)
        {
            // Guild channel
            m_cluster.log(dpp::ll_info,
                fmt::format("Sending outgoing message in channel '{}' - '{}'.", event.msg.channel_id, channel->name));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void ToasterBot::onSlashCommand(const dpp::slashcommand_t& event)
{
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    if (m_debug)
    {
        start = std::chrono::high_resolution_clock::now();
    }

    std::shared_ptr<JobQueue> queue = GetOrCreateQueue(event.command.guild_id);
    std::shared_ptr<GuildSettings> guild = GetOrCreateSettings(event.command.guild_id);

    CommandContext ctx{
        m_cluster,
        queue,
        guild,
        m_repo,
        m_debug
    };

    for (auto cmd : Toaster::BotCommands)
    {
        cmd->ExecuteCommand(ctx, event);
    }

    if (m_debug)
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        m_cluster.log(
            dpp::ll_info,
            fmt::format(
                "Slash '{}' executed in '{}' micro-seconds.",
                event.command.get_command_name(),
                duration
            )
        );
    }

    m_cluster.log(dpp::ll_info, fmt::format("TOASTER processed SLASHCOMMAND '{}' for USER '{}'.", event.command.get_command_name(), event.command.get_issuing_user().id));
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void ToasterBot::onInteractionCreate(const dpp::interaction_create_t& event)
{
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    if (m_debug)
    {
        start = std::chrono::high_resolution_clock::now();
    }

    std::shared_ptr<JobQueue> queue = GetOrCreateQueue(event.command.guild_id);
    std::shared_ptr<GuildSettings> guild = GetOrCreateSettings(event.command.guild_id);

    CommandContext ctx{
        m_cluster,
        queue,
        guild,
        m_repo,
        m_debug
    };

    for (auto cmd : Toaster::BotCommands)
    {
        cmd->ExecuteInteraction(ctx, event);
    }

    if (m_debug)
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        m_cluster.log(
            dpp::ll_info,
            fmt::format(
                "Interaction '{}' executed in '{}' micro-seconds.",
                event.command.get_command_name(),
                duration
            )
        );
    }

    m_cluster.log(dpp::ll_info, fmt::format("TOASTER processed INTERACTION '{}' for USER '{}'.", event.command.get_command_name(), event.command.get_issuing_user().id));
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void ToasterBot::onFormSubmit(const dpp::form_submit_t& event)
{
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    if (m_debug)
    {
        start = std::chrono::high_resolution_clock::now();
    }

    std::shared_ptr<JobQueue> queue = GetOrCreateQueue(event.command.guild_id);
    std::shared_ptr<GuildSettings> guild = GetOrCreateSettings(event.command.guild_id);

    CommandContext ctx{
        m_cluster,
        queue,
        guild,
        m_repo,
        m_debug
    };

    for (auto cmd : Toaster::BotCommands)
    {
        cmd->ExecuteFormSubmit(ctx, event);
    }

    if (m_debug)
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        m_cluster.log(
            dpp::ll_info,
            fmt::format(
                "Form '{}' executed in '{}' micro-seconds.",
                event.custom_id,
                duration
            )
        );
    }

    m_cluster.log(dpp::ll_info, fmt::format("TOASTER processed FORM_SUBMIT '{}' for USER '{}'.", event.custom_id, event.command.get_issuing_user().id));
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void ToasterBot::onButtonClick(const dpp::button_click_t& event)
{
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    if (m_debug)
    {
        start = std::chrono::high_resolution_clock::now();
    }

    std::shared_ptr<JobQueue> queue = GetOrCreateQueue(event.command.guild_id);
    std::shared_ptr<GuildSettings> guild = GetOrCreateSettings(event.command.guild_id);

    CommandContext ctx{
        m_cluster,
        queue,
        guild,
        m_repo,
        m_debug
    };

    for (auto cmd : Toaster::BotCommands)
    {
        cmd->ExecuteButtonClick(ctx, event);
    }

    if (m_debug)
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        m_cluster.log(
            dpp::ll_info,
            fmt::format(
                "Button '{}' executed in '{}' micro-seconds.",
                event.custom_id,
                duration
            )
        );
    }

    m_cluster.log(dpp::ll_info, fmt::format("TOASTER processed BUTTON_CLICK '{}' for USER '{}'.", event.custom_id, event.command.get_issuing_user().id));
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<JobQueue> ToasterBot::GetOrCreateQueue(const dpp::snowflake& guildID)
{
    std::unique_lock<std::shared_mutex> lock(m_mtxShared);

    auto it = m_spQueue.find(guildID);
    if (it != m_spQueue.end())
        return it->second;

    // Create a new queue for this guild
    auto queue = std::make_shared<JobQueue>(guildID, m_repo);
    m_spQueue.emplace(guildID, queue);

    return queue;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
std::shared_ptr<GuildSettings> ToasterBot::GetOrCreateSettings(const dpp::snowflake& guildID)
{
    auto it = g_settings.find(guildID);
    if (it != g_settings.end())
        return it->second;

    // Create default settings if none exist, then update database with defaults
    auto settings = std::make_shared<GuildSettings>(guildID);
    g_settings.emplace(guildID, settings);
    m_repo->InsertGuild(settings);

    return settings;
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void ToasterBot::LoadDatabase()
{
    // Load persistent queues
    auto queues = m_repo->GetGuildsWithJobs();
    for (const auto& guild : queues)
    {
        m_spQueue.emplace(guild, std::make_shared<JobQueue>(guild, m_repo));
    }

    // Load persistent guild settings
    auto settings = m_repo->GetGuildsWithSettings();
    for (const auto& guild : settings)
    {
        auto setting = std::make_shared<GuildSettings>(guild);
        auto bResult = m_repo->LoadGuildSettings(setting);
        g_settings.emplace(guild, setting);
    }
}