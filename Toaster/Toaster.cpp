#include "Toaster.h"

#include "BotUtility.h"
#include "CommandContext.h"
#include "Commands.h"
#include "JobQueue.h"
#include "Resource.h"
// d++
#include <dpp/channel.h>
#include <dpp/dpp.h>
#include <dpp/snowflake.h>
// fmt
#include <fmt/base.h>
#include <fmt/format.h>
// microsoft
#include <guiddef.h>
// std library
#include <chrono>
#include <unordered_map>

ToasterBot::ToasterBot(dpp::cluster& cluster, const uint32_t clusterId, const std::shared_ptr<JobQueue>& spQueue, const bool bDebug)
    : m_cluster(cluster), m_clusterId(clusterId), m_spQueue(spQueue), m_debug(bDebug), m_iShardCount{ 0 } { }

void ToasterBot::onReady(const dpp::ready_t& event)
{
    if (dpp::run_once<struct clear_bot_commands>()) 
    {
        //ICustomCommand::UnregisterAll(m_cluster);
        //ICustomCommand::UnregisterGuildAll(&m_cluster, 1472034166869852287);
    }
    if (dpp::run_once<struct register_bot_commands>())
    {
        //ICustomCommand::UnregisterAll(m_cluster);
        ICustomCommand::RegisterGuildAll(&m_cluster, 1472034166869852287, Toaster::BotCommands);
    }
}

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

void ToasterBot::onSlashCommand(const dpp::slashcommand_t& event)
{
    std::chrono::time_point<std::chrono::steady_clock> start;
    if (m_debug)
    {
        start = std::chrono::steady_clock::now();
    }

    CommandContext ctx{ m_cluster, m_spQueue, m_debug, m_vWorkers, PermissionsMgr::GetInstance()};
    for (auto cmd : Toaster::BotCommands)
    {
        cmd->ExecuteCommand(ctx, event);
    }

    if (m_debug)
    {
        auto end = std::chrono::steady_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        m_cluster.log(
            dpp::ll_debug,
            fmt::format(
                "Slash '{}' executed in '{}' ms (user: '{}').",
                event.command.get_command_name(),
                duration,
                event.command.get_issuing_user().id
            )
        );
    }

    m_cluster.log(dpp::ll_info, fmt::format("TOASTER processed SLASHCOMMAND '{}' for USER '{}'.", event.command.get_command_name(), event.command.get_issuing_user().id));
}

void ToasterBot::onInteractionCreate(const dpp::interaction_create_t& event)
{
    std::chrono::time_point<std::chrono::steady_clock> start;
    if (m_debug)
    {
        start = std::chrono::steady_clock::now();
    }

    CommandContext ctx{ m_cluster, m_spQueue, m_debug, m_vWorkers, PermissionsMgr::GetInstance() };
    for (auto cmd : Toaster::BotCommands)
    {
        cmd->ExecuteInteraction(ctx, event);
    }

    if (m_debug)
    {
        auto end = std::chrono::steady_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        m_cluster.log(
            dpp::ll_debug,
            fmt::format(
                "Interaction '{}' executed in '{}' ms (user: '{}').",
                event.command.get_command_name(),
                duration,
                event.command.get_issuing_user().id
            )
        );
    }

    m_cluster.log(dpp::ll_info, fmt::format("TOASTER processed INTERACTION '{}' for USER '{}'.", event.command.get_command_name(), event.command.get_issuing_user().id));
}

void ToasterBot::onFormSubmit(const dpp::form_submit_t& event)
{
    std::chrono::time_point<std::chrono::steady_clock> start;
    if (m_debug)
    {
        start = std::chrono::steady_clock::now();
    }

    CommandContext ctx{ m_cluster, m_spQueue, m_debug, m_vWorkers, PermissionsMgr::GetInstance() };
    for (auto cmd : Toaster::BotCommands)
    {
        cmd->ExecuteFormSubmit(ctx, event);
    }

    if (m_debug)
    {
        auto end = std::chrono::steady_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        m_cluster.log(
            dpp::ll_debug,
            fmt::format(
                "Form '{}' executed in '{}' ms (user: '{}').",
                event.custom_id,
                duration,
                event.command.get_issuing_user().id
            )
        );
    }

    m_cluster.log(dpp::ll_info, fmt::format("TOASTER processed FORM_SUBMIT '{}' for USER '{}'.", event.custom_id, event.command.get_issuing_user().id));
}

void ToasterBot::onButtonClick(const dpp::button_click_t& event)
{
    std::chrono::time_point<std::chrono::steady_clock> start;
    if (m_debug)
    {
        start = std::chrono::steady_clock::now();
    }

    CommandContext ctx{ m_cluster, m_spQueue, m_debug, m_vWorkers, PermissionsMgr::GetInstance() };
    for (auto cmd : Toaster::BotCommands)
    {
        cmd->ExecuteButtonClick(ctx, event);
    }

    if (m_debug)
    {
        auto end = std::chrono::steady_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        m_cluster.log(
            dpp::ll_debug,
            fmt::format(
                "Button '{}' executed in '{}' ms (user: '{}').",
                event.custom_id,
                duration,
                event.command.get_issuing_user().id
            )
        );
    }

    m_cluster.log(dpp::ll_info, fmt::format("TOASTER processed BUTTON_CLICK '{}' for USER '{}'.", event.custom_id, event.command.get_issuing_user().id));
}