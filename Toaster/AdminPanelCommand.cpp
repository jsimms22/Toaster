//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief Implementation of the AdminPanelCommand.
///
/// Contains the implementation of AdminPanelCommand declared in Commands.h.
//---------------------------------------------------------------------------------------------------------------------
#include "Commands.h"
#include "BotUtility.h"
#include "JobQueue.h"
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
    if (!(pManager->IsGuildAdmin(author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id)) ||
          pManager->IsBotOwner(author.id)) && !ctx.debug)
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("USER '{}' was DENIED access to use '{}' command", author.id, event.command.get_command_name()));
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
        event.edit_original_response(CreateQueuePanel(ctx,event));
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
    if (id.starts_with("admin_shutdown:") || 
        id.starts_with("admin_logs:") || 
        id.starts_with("admin_queue:"))
    {

        if (id.starts_with("admin_logs:"))
        {
            event.reply(dpp::ir_update_message, CreateBotPanel(ctx, event).add_file("testLog.txt", "Imagine a forest. There might be a bear. But definitely some logs."));
            return;
        }
        else if (id.starts_with("admin_queue:"))
        {
            event.reply(dpp::ir_update_message, CreateBotPanel(ctx, event).add_file("testQueue.txt", "Imagine a sea of requests. You begin to feel anxious."));
            return;
        }
        else if (id.starts_with("admin_shutdown:"))
        {
            event.reply(dpp::ir_update_message, CreateBotPanel(ctx, event));
            return;
        }

        event.reply(dpp::ir_update_message, CreateBotPanel(ctx, event));
        return;
    }
    /*-------- Queue buttons --------*/
    else if (id.starts_with("admin_addworker:") || 
        id.starts_with("admin_modifyworker:") || 
        id.starts_with("admin_bulkassign:") ||
        id.starts_with("admin_archive:"))
    {
        event.reply(dpp::ir_update_message, CreateQueuePanel(ctx, event));

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

    dpp::embed embed = CreateBotEmbed(ctx, event);
    dpp::component row = CreateBotButtonRow(author.id);

    return dpp::message().add_embed(embed).add_component(row).set_flags(dpp::m_ephemeral);
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Helper function to create an embed object for the admin panel for the bot.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
/// 
/// \return Returns an embed object to attach to a message.
//---------------------------------------------------------------------------------------------------------------------
dpp::embed AdminPanelCommand::CreateBotEmbed(CommandContext& ctx, const dpp::interaction_create_t& event) const
{
    // todo stats on bot

    dpp::embed embed;
    embed.set_title("Bot Admin Panel:")
        .set_description(fmt::format("Beep..."))
        .set_color(0x3498db);

    return embed;
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Helper function to build a row of buttons for the bot panel.
///
/// \param[out] user     The unique user id to store as state in the button ids.
/// 
/// \return Returns a component object containing a row of buttons.
//---------------------------------------------------------------------------------------------------------------------
dpp::component AdminPanelCommand::CreateBotButtonRow(const dpp::snowflake& user) const
{
    dpp::component row;
    row.add_component(
        dpp::component()
        .set_type(dpp::cot_button)
        .set_label("Shutdown")
        .set_style(dpp::cos_danger)
        .set_id(fmt::format("admin_shutdown:{}", user)));

    row.add_component(
        dpp::component()
        .set_type(dpp::cot_button)
        .set_label("Send Logs")
        .set_style(dpp::cos_primary)
        .set_id(fmt::format("admin_logs:{}", user)));

    row.add_component(
        dpp::component()
        .set_type(dpp::cot_button)
        .set_label("Send Queue")
        .set_style(dpp::cos_primary)
        .set_id(fmt::format("admin_queue:{}", user)));

    return row;
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Helper function to create a message object for the admin panel for the queue.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
/// 
/// \return Returns a message object to send back to the user.
//---------------------------------------------------------------------------------------------------------------------
dpp::message AdminPanelCommand::CreateQueuePanel(CommandContext& ctx, const dpp::interaction_create_t& event) const
{
    const dpp::user author = event.command.get_issuing_user();

    dpp::embed embed = CreateQueueEmbed(ctx, event);
    dpp::component row = CreateQueueButtonRow(author.id);

    return dpp::message().add_embed(embed).add_component(row).set_flags(dpp::m_ephemeral);
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Helper function to create an embed object for the admin panel for the queue.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
/// 
/// \return Returns an embed object to attach to a message.
//---------------------------------------------------------------------------------------------------------------------
dpp::embed AdminPanelCommand::CreateQueueEmbed(CommandContext& ctx, const dpp::interaction_create_t& event) const
{
    const std::string header = "Queue Admin Panel:";
    dpp::embed embed;
    if (ctx.queue->GetQueueSize() == 0)
    {
        const std::string summmary = "Request queue is currently empty.";
        embed.set_title(header)
            .set_description(summmary)
            .set_color(0x3498db);
    }
    else
    {
        const std::string summmary = ctx.queue->PrintQueueAdminSummary(ctx.cluster);
        embed.set_title(header)
            .set_description(summmary)
            .set_color(0x3498db);
    }

    return embed;
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Helper function to build a row of buttons for the admin queue panel.
///
/// \param[out] user     The unique user id to store as state in the button ids.
/// 
/// \return Returns a component object containing a row of buttons.
//---------------------------------------------------------------------------------------------------------------------
dpp::component AdminPanelCommand::CreateQueueButtonRow(const dpp::snowflake& user) const
{
    dpp::component row;
    row.add_component(
        dpp::component()
        .set_type(dpp::cot_button)
        .set_label("Add Worker")
        .set_style(dpp::cos_primary)
        .set_id(fmt::format("admin_addworker:{}", user)));

    row.add_component(
        dpp::component()
        .set_type(dpp::cot_button)
        .set_label("Modify Workers")
        .set_style(dpp::cos_primary)
        .set_id(fmt::format("admin_modifyworker:{}", user)));

    row.add_component(
        dpp::component()
        .set_type(dpp::cot_button)
        .set_label("Bulk Assign")
        .set_style(dpp::cos_primary)
        .set_id(fmt::format("admin_bulkassign:{}", user)));

    row.add_component(
        dpp::component()
        .set_type(dpp::cot_button)
        .set_label("Archive")
        .set_style(dpp::cos_primary)
        .set_id(fmt::format("admin_archive:{}", user)));

    return row;
}