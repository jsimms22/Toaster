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
// tinyxml
#include "tinyxml2.h"
// microsoft
#include <guiddef.h>
// std library
#include <chrono>

//---------------------------------------------------------------------------------------------------------------------
// \brief Constructor
//---------------------------------------------------------------------------------------------------------------------
ToasterBot::ToasterBot(dpp::cluster& cluster, const uint32_t clusterId, const bool bDebug)
    : m_cluster(cluster), m_clusterId(clusterId), m_debug(bDebug), m_iShardCount{ 0 } 
{
    LoadGuildSettings();
    LoadQueueData();
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
    std::chrono::time_point<std::chrono::steady_clock> start;
    if (m_debug)
    {
        start = std::chrono::steady_clock::now();
    }

    std::shared_ptr<JobQueue> queue = GetOrCreateQueue(event.command.guild_id);
    GuildSettings& guild = GetOrCreateSettings(event.command.guild_id);

    CommandContext ctx{
        m_cluster,
        queue,
        guild,
        m_debug
    };

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

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void ToasterBot::onInteractionCreate(const dpp::interaction_create_t& event)
{
    std::chrono::time_point<std::chrono::steady_clock> start;
    if (m_debug)
    {
        start = std::chrono::steady_clock::now();
    }

    std::shared_ptr<JobQueue> queue = GetOrCreateQueue(event.command.guild_id);
    GuildSettings& guild = GetOrCreateSettings(event.command.guild_id);

    CommandContext ctx{
        m_cluster,
        queue,
        guild,
        m_debug
    };

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

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void ToasterBot::onFormSubmit(const dpp::form_submit_t& event)
{
    std::chrono::time_point<std::chrono::steady_clock> start;
    if (m_debug)
    {
        start = std::chrono::steady_clock::now();
    }

    std::shared_ptr<JobQueue> queue = GetOrCreateQueue(event.command.guild_id);
    GuildSettings& guild = GetOrCreateSettings(event.command.guild_id);

    CommandContext ctx{
        m_cluster,
        queue,
        guild,
        m_debug
    };

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

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void ToasterBot::onButtonClick(const dpp::button_click_t& event)
{
    std::chrono::time_point<std::chrono::steady_clock> start;
    if (m_debug)
    {
        start = std::chrono::steady_clock::now();
    }

    std::shared_ptr<JobQueue> queue = GetOrCreateQueue(event.command.guild_id);
    GuildSettings& guild = GetOrCreateSettings(event.command.guild_id);

    CommandContext ctx{
        m_cluster,
        queue,
        guild,
        m_debug
    };

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

void ToasterBot::LoadGuildSettings()
{
    std::unique_lock<std::shared_mutex> lock(m_mtxShared);

    tinyxml2::XMLDocument doc;
    const char* path = "../guilds.xml";

    tinyxml2::XMLError result = doc.LoadFile(path);

    if (result == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
    {
        // Create new file with root
        auto* root = doc.NewElement("GuildList");
        doc.InsertEndChild(root);

        doc.SaveFile(path);
        return; // No guilds yet
    }

    if (result != tinyxml2::XML_SUCCESS)
    {
        // Handle corrupted file if desired
        return;
    }

    tinyxml2::XMLElement* root = doc.RootElement();
    if (!root)
    {
        root = doc.NewElement("GuildList");
        doc.InsertEndChild(root);
        doc.SaveFile(path);
        return;
    }

    // Iterate over all <Request> elements within each <User>
    for (tinyxml2::XMLElement* xmlNode = root->FirstChildElement("Guild"); xmlNode != nullptr; xmlNode = xmlNode->NextSiblingElement("Guild"))
    {
        const dpp::snowflake id = xmlNode->Unsigned64Attribute("ID", 0);

        if (id)
        {
            GuildSettings guild;
            guild.ReadAttributes(xmlNode, root);
            g_settings.emplace(id, std::move(guild));
        }
    }
}

void ToasterBot::SaveGuildSettings()
{
    std::unique_lock<std::shared_mutex> lock(m_mtxShared);

    tinyxml2::XMLDocument doc;
    doc.LoadFile("../guilds.xml");
    doc.Clear();

    // Get the root element (<RequestQueue>)
    tinyxml2::XMLElement* root = doc.RootElement();
    if (!root)
    {
        root = doc.NewElement("GuildList");
        doc.InsertEndChild(root);
    }

    for (const auto& [id, settings] : g_settings)
    {
        tinyxml2::XMLElement* xmlNode = root->InsertNewChildElement("Guild");
        root->SetAttribute("ID", id);

        settings.WriteAttributes(xmlNode, root);
        root->InsertEndChild(xmlNode);
    }

    // Save the updated XML to a file
    doc.SaveFile("../guilds.xml");
}


void ToasterBot::LoadQueueData()
{
    std::unique_lock<std::shared_mutex> lock(m_mtxShared);

    tinyxml2::XMLDocument doc;
    const char* path = "../queue.xml";

    auto result = doc.LoadFile(path);
    if (result == tinyxml2::XML_ERROR_FILE_NOT_FOUND)
    {
        auto* root = doc.NewElement("Queues");
        doc.InsertEndChild(root);
        doc.SaveFile(path);
    }
    else if (result != tinyxml2::XML_SUCCESS)
    {
        return; // Corrupted or unreadable
    }

    tinyxml2::XMLElement* root = doc.RootElement();
    if (!root || std::string(root->Name()) != "Queues")
    {
        doc.Clear();
        root = doc.NewElement("Queues");
        doc.InsertEndChild(root);
        doc.SaveFile(path);
    }

    // Clear any existing queues
    m_spQueue.clear();

    // Iterate all <RequestList> nodes
    for (tinyxml2::XMLElement* requestList = root->FirstChildElement("RequestList");
        requestList != nullptr;
        requestList = requestList->NextSiblingElement("RequestList"))
    {
        const char* guildAttr = requestList->Attribute("GuildID");
        if (!guildAttr)
            continue;

        uint64_t guildID = std::stoull(guildAttr);

        // Let JobQueue handle loading its own jobs
        auto queue = std::make_shared<JobQueue>(guildID, requestList);

        m_spQueue.emplace(guildID, std::move(queue));
    }
}

std::shared_ptr<JobQueue> ToasterBot::GetOrCreateQueue(const dpp::snowflake& guildID)
{
    std::unique_lock<std::shared_mutex> lock(m_mtxShared);

    auto it = m_spQueue.find(guildID);
    if (it != m_spQueue.end())
        return it->second;

    // Create a new queue for this guild
    auto queue = std::make_shared<JobQueue>(guildID, nullptr);

    m_spQueue.emplace(guildID, queue);
    return queue;
}

GuildSettings& ToasterBot::GetOrCreateSettings(const dpp::snowflake& guildID)
{
    auto it = g_settings.find(guildID);
    if (it != g_settings.end())
        return it->second;

    // Create default settings if none exist
    g_settings[guildID] = GuildSettings{};
    return g_settings[guildID];
}