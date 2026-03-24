//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief Implementation of the AdminPanelCommand.
///
/// Contains the implementation of AdminPanelCommand declared in Commands.h.
//---------------------------------------------------------------------------------------------------------------------
#include "Commands.h"

#include "AdminButtonPanel.h"
#include "BotUtility.h"
#include "JobQueue.h"
#include "JobRequest.h"
#include "GuildSettings.h"
#include "PermissionsMgr.h"
#include "PaginationPanel.h"
// fmt
#include <fmt/format.h>

//---------------------------------------------------------------------------------------------------------------------
/// \brief Event handler for create interation processes.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
//---------------------------------------------------------------------------------------------------------------------
void AdminPanelCommand::ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Type));

    const auto pManager = PermissionsMgr::GetInstance();
    if (!pManager->CanAccessAdminPanel(event, author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) && !ctx.debug)
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("USER '{}' was DENIED access to use '{}' command", author.global_name, event.command.get_command_name()));
        return;
    }

    if (strCmdID == Option_Bot)
    {
        // acknowledge immediately to avoid timing out
        event.reply(dpp::message("...this could take a second. Please hold.").set_flags(dpp::m_ephemeral));
        event.edit_original_response(CreateBotPanel(ctx, event));

    }
    else if (strCmdID == Option_Queue)
    {
        // acknowledge immediately to avoid timing out
        event.reply(dpp::message("...this could take a second. Please hold.").set_flags(dpp::m_ephemeral));
        event.edit_original_response(CreateQueuePanel(ctx, event, 0 /* page */));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Event handler for button interactions.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
//---------------------------------------------------------------------------------------------------------------------
void AdminPanelCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
    const std::string id = event.custom_id; // "admin_type:workerid:"

    /*-------- Bot buttons --------*/ 
    if (id.starts_with(fmt::format("{}_botrefresh:", this->name)) ||
        id.starts_with(fmt::format("{}_changerole:", this->name)) ||
        id.starts_with(fmt::format("{}_channelrules:", this->name)) ||
        id.starts_with(fmt::format("{}_changecooldown:", this->name)) ||
        // row 2
        id.starts_with(fmt::format("{}_changemaxrequests:", this->name)) ||
        id.starts_with(fmt::format("{}_changearchive:", this->name)) ||
        id.starts_with(fmt::format("{}_changestalled:", this->name)) ||
        // row 3
        id.starts_with(fmt::format("{}_pingrules:", this->name)) || 
        id.starts_with(fmt::format("{}_sendlogs:", this->name)) ||
        id.starts_with(fmt::format("{}_sendqueue:", this->name)) ||
        id.starts_with(fmt::format("{}_deleteguild:", this->name)))
    {
        if (id.starts_with(fmt::format("{}_botrefresh:", this->name)))
        {
            AdminQueueButtonPanel::RefreshButton(id, ctx, event);
            // do not return
        }
        else if (id.starts_with(fmt::format("{}_changerole:", this->name)))
        {
            AdminBotButtonPanel::ChangeRoleButton(id, ctx, event);
            return;
        }
        else if (id.starts_with(fmt::format("{}_channelrules:", this->name)))
        {
            AdminBotButtonPanel::ChangeChannelButton(id, ctx, event);
            return;
        }
        else if (id.starts_with(fmt::format("{}_pingrules:", this->name)))
        {
            AdminBotButtonPanel::PingRulesButton(id, ctx, event);
            return;
        }
        else if (id.starts_with(fmt::format("{}_changecooldown:", this->name)))
        {
            AdminBotButtonPanel::ChangeCooldown(id, ctx, event);
            return;
        }
        // row 2
        else if (id.starts_with(fmt::format("{}_changemaxrequests:", this->name)))
        {
            AdminBotButtonPanel::ChangeMaxOpenRequests(id, ctx, event);
            return;
        }
        else if (id.starts_with(fmt::format("{}_changearchive:", this->name)))
        {
            AdminBotButtonPanel::ChangeArchiveThreshold(id, ctx, event);
            return;
        }
        else if (id.starts_with(fmt::format("{}_changestalled:", this->name)))
        {
            AdminBotButtonPanel::ChangeStallThreshold(id, ctx, event);
            return;
        }
        // row 3
        else if (id.starts_with(fmt::format("{}_sendlogs:", this->name)))
        {
            AdminBotButtonPanel::SendLogsButton(id, ctx, event);
            return;
        }
        else if (id.starts_with(fmt::format("{}_sendqueue:", this->name)))
        {
            AdminBotButtonPanel::SendQueueButton(id, ctx, event);
            return;
        }
        else if (id.starts_with(fmt::format("{}_deleteguild:", this->name)))
        {
            AdminBotButtonPanel::DeleteDataButton(id, ctx, event);
            return;
        }

        event.reply(dpp::ir_update_message, CreateBotPanel(ctx, event));
        return;
    }
    /*-------- Queue buttons --------*/
    else if (id.starts_with(fmt::format("{}_queuerefresh:", this->name)) || 
             id.starts_with(fmt::format("{}_assignworkers:", this->name)) ||
             id.starts_with(fmt::format("{}_admincomplete:", this->name)) ||
             id.starts_with(fmt::format("{}_adminhold:", this->name)) ||
             id.starts_with(fmt::format("{}_admindelete:", this->name)) ||
             // row 2
             id.starts_with(fmt::format("{}_showworkers:", this->name)) ||
             id.starts_with(fmt::format("{}_downloadworkers:", this->name)) ||
             id.starts_with(fmt::format("{}_downloadarchive:", this->name)) ||
             id.starts_with(fmt::format("{}_reopenarchive:", this->name)))
    {
        if (id.starts_with(fmt::format("{}_queuerefresh:", this->name)))
        {
            AdminQueueButtonPanel::RefreshButton(id, ctx, event);
            // do not return
        }
        else if (id.starts_with(fmt::format("{}_assignworkers:", this->name)))
        {
            AdminQueueButtonPanel::AssignWorkersButton(id, ctx, event);
            return;
        }
        else if (id.starts_with(fmt::format("{}_admincomplete:", this->name)))
        {
            AdminQueueButtonPanel::MarkCompleteButton(id, ctx, event);
            // do not return
        }
        else if (id.starts_with(fmt::format("{}_adminhold:", this->name)))
        {
            AdminQueueButtonPanel::MarkOnHoldButton(id, ctx, event);
            // do not return
        }
        else if (id.starts_with(fmt::format("{}_admindelete:", this->name)))
        {
            AdminQueueButtonPanel::AdminDelete(id, ctx, event);
            //return;
        }
        else if (id.starts_with(fmt::format("{}_showworkers:", this->name)))
        {
            AdminQueueButtonPanel::ShowWorkersButton(id, ctx, event);
            return;
        }
        else if (id.starts_with(fmt::format("{}_downloadworkers:", this->name)))
        {
            AdminQueueButtonPanel::DownloadWorkersButton(id, ctx, event);
            //return;
        }
        else if (id.starts_with(fmt::format("{}_downloadarchive:", this->name)))
        {
            AdminQueueButtonPanel::DownloadArchiveButton(id, ctx, event);
            return;
        }
        else if (id.starts_with(fmt::format("{}_reopenarchive:", this->name)))
        {
            AdminQueueButtonPanel::ReopenArchiveButton(id, ctx, event);
            return;
        }

        event.reply(dpp::ir_update_message, CreateQueuePanel(ctx, event, 0));
        return;
    }
    /*-------- Page (Queue) buttons --------*/
    else if (id.starts_with(fmt::format("{}_adminnext:", this->name)) ||
             id.starts_with(fmt::format("{}_adminprev:", this->name)))
    {
        // Deconstruct custom id for state information
        const auto parts = utils::Split(id, ':');
        const std::size_t page = parts[1] != std::to_string(std::numeric_limits<std::size_t>::max()) ? std::stoul(parts[1]) : 0;

        event.reply(dpp::ir_update_message, CreateQueuePanel(ctx, event, page));
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Helper function to create a message object for the admin panel for the bot.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
/// 
/// \return Returns a message object to send back to the user.
//---------------------------------------------------------------------------------------------------------------------
dpp::message AdminPanelCommand::CreateBotPanel(CommandContext& ctx, const dpp::interaction_create_t& event) const
{
    const dpp::user author = event.command.get_issuing_user();

    AdminBotButtonPanel botPanel(this->name, author.id);
    botPanel.AddEmbed("Bot Admin Panel:", CreateBotEmbed(ctx, event));

    return botPanel.set_flags(dpp::m_ephemeral);
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Helper function to create an embed object for the admin panel for the bot.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
/// 
/// \return Returns an embed object to attach to a message.
//---------------------------------------------------------------------------------------------------------------------
const std::string AdminPanelCommand::CreateBotEmbed(CommandContext& ctx, const dpp::interaction_create_t& event) const
{
    auto FormatChannel = [](const std::optional<dpp::snowflake>& id)
        {
            return id.has_value()
                ? fmt::format("<#{}>", *id)
                : std::string("*Not Set*");
        };

    auto FormatRole = [](const std::optional<dpp::snowflake>& id)
        {
            return id.has_value()
                ? fmt::format("<@&{}>", *id)
                : std::string("*Not Set*");
        };

    const std::string settings = fmt::format(
        "### Channel Announcements\n"
        "- New Job: {}\n"
        "- Update Job: {}\n"
        "- Delete Job: {}\n"
        "- Complete Job: {}\n"

        "### Cooldown\n"
        "- Announcement Cooldown: {} seconds\n"

        "### Per Customer Settings\n"
        "- Max Open Requests: {} requests\n"

        "### Automated Queue Settings\n"
        "- Auto-Archive Threshold: {} hours\n"
        "- Auto-Stalled Threshold: {} hours\n\n"

        "**Note:** The bot will conduct scans every so often. These scans handle certain automated tasks for managers:\n"
        "- Searches for **Completed** job requests that have not been edited for some amount of time and adds them to the archive.\n"
        "- Searches for jobs in **Open** or **Assigned** status. If they have not been edited for some amount of time the bot relabels them as **Stalled**.\n"
        "- If the 'Last Edit' timestamp is older than the set thresholds, the bot will apply these actions automatically.\n"

        "### Ping Rules\n"
        "- On New: {}\n"
        "- On Update: {}\n"
        "- On Delete: {}\n"
        "- On Complete: {}\n"

        "### Roles\n"
        "- General Ping Role: {}\n"
        "- Crafter: {}\n"
        "- Builder: {}\n"
        "- Component: {}\n"
        "- Gatherer: {}\n"
        "- Refiner: {}\n"
        "- Hazmat: {}\n"
        "- Manager: {}",

        FormatChannel(ctx.guild->idNewJobChannel),
        FormatChannel(ctx.guild->idUpdateJobChannel),
        FormatChannel(ctx.guild->idDeleteJobChannel),
        FormatChannel(ctx.guild->idCompleteJobChannel),

        ctx.guild->announcement_cooldown.count(),

        ctx.guild->requestLimitPerUser,

        ctx.guild->archival_age.count(),
        ctx.guild->stalled_age.count(),

        ctx.guild->bPingOnNew ? "Enabled" : "Disabled",
        ctx.guild->bPingOnUpdate ? "Enabled" : "Disabled",
        ctx.guild->bPingOnDelete ? "Enabled" : "Disabled",
        ctx.guild->bPingOnComplete ? "Enabled" : "Disabled",

        FormatRole(ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Ping)]),
        FormatRole(ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Crafter)]),
        FormatRole(ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Builder)]),
        FormatRole(ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Comp)]),
        FormatRole(ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Gatherer)]),
        FormatRole(ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Refiner)]),
        FormatRole(ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Hazmat)]),
        FormatRole(ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Manager)])
    );

    return settings;
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Helper function to create a message object for the admin panel for the queue.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
/// 
/// \return Returns a message object to send back to the user.
//---------------------------------------------------------------------------------------------------------------------
dpp::message AdminPanelCommand::CreateQueuePanel(CommandContext& ctx, const dpp::interaction_create_t& event, const std::size_t page) const
{
    const dpp::user author = event.command.get_issuing_user();

    // Retrieve the job
    const auto compare = [](const std::shared_ptr<const JobRequest> job) -> bool 
                         { return job->GetWorkerIDs().empty() && job->GetStatus() < JobRequest::status::hold; };
    const auto job = ctx.queue->FirstAssignment(compare, page);
    const std::size_t size = ctx.queue->GetQueueSize(true, compare);
    
    // Construct the admin panel
    const std::string strID = job ? ToString(job->GetID()) : "";
    AdminQueueButtonPanel queuePanel(this->name, author.id, strID);
    queuePanel.AddPageRow(page, size);
    queuePanel.AddEmbed("Queue Admin Panel:", CreateQueueEmbed(ctx, event));
    queuePanel.AddEmbed(fmt::format("Unassigned Job (Page {} of {})", page + 1, size == 0 ? 1 : size), 
                        job ? job->PrintJobDetails(ctx.cluster, event.command.guild_id) : "No unassigned jobs in queue.");

    return queuePanel.set_flags(dpp::m_ephemeral);
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Helper function to create an embed object for the admin panel for the queue.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
/// 
/// \return Returns an embed object to attach to a message.
//---------------------------------------------------------------------------------------------------------------------
const std::string AdminPanelCommand::CreateQueueEmbed(CommandContext& ctx, const dpp::interaction_create_t& event) const
{
    return (ctx.queue->GetQueueSize() == 0) ?
        "Request queue is currently empty." : 
        ctx.queue->PrintQueueAdminSummary(ctx.cluster);
}