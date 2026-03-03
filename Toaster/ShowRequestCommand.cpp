//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief Implementation of the ShowRequestCommand.
///
/// Contains the implementation of ShowRequestCommand declared in Commands.h.
//---------------------------------------------------------------------------------------------------------------------
#include "Commands.h"
#include "JobQueue.h"
#include "BotUtility.h"
#include "RequestDlg.h"
#include "PermissionsMgr.h"
#include "WorkerPanel.h"
#include "CustomerPanel.h"
// fmt
#include <fmt/format.h>

//---------------------------------------------------------------------------------------------------------------------
/// \brief Event handler for create interaction processes.
///
/// Handles the command by displaying the details of a specific job request. Verifies that the user has permission 
/// to view the request.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
//---------------------------------------------------------------------------------------------------------------------
void ShowRequestCommand::ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    std::string strJobID = std::get<std::string>(event.get_parameter(Parameter_Id));
    utils::FilterWhiteSpace(strJobID);
    const auto job = ctx.queue->GetJobByGUID(strJobID);
    if (!job)
    {
        event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    const auto pManager = PermissionsMgr::GetInstance();
    if (!(pManager->IsRequestOwner(author.id, job) ||
          pManager->IsRequestWorker(author.id, job) ||
          pManager->IsWorker(author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) ||
          pManager->IsManager(author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) ||
          pManager->IsGuildAdmin(author.id, event) ||
          pManager->IsBotOwner(author.id)) && !ctx.debug)
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("USER '{}' was DENIED access to use '{}' command", author.id, event.command.get_command_name()));
        return;
    }

    // Acknowledge immediately to avoid timing out
    event.reply(dpp::message("...this could take a second. Please hold.").set_flags(dpp::m_ephemeral));

    // Edit the original message
    event.edit_original_response(SendPanel(ctx, event, job, author.id).set_flags(dpp::m_ephemeral));
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Event handler for button interactions.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
//---------------------------------------------------------------------------------------------------------------------
void ShowRequestCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
    const std::string id = event.custom_id;

    if (id.starts_with(fmt::format("{}_complete:", this->name)))
    {
        WorkerPanel::CompleteButton(id, ctx, event);

        auto parts = utils::Split(id, ':');
        dpp::snowflake user = parts[1];
        const std::string guid = parts[2];
        const auto job = ctx.queue->GetJobByGUID(guid);

        // Edit the original message
        event.reply(dpp::ir_update_message, SendPanel(ctx, event, job, user).set_flags(dpp::m_ephemeral));
        return;
    }
    else if (id.starts_with(fmt::format("{}_edit:", this->name)))
    {
        GeneralUserPanel::EditButton(id, ctx, event);
        return;
    }
    else if (id.starts_with(fmt::format("{}_note:", this->name)))
    {
        GeneralUserPanel::NoteButton(id, ctx, event);
        return;
    }
    else if (id.starts_with(fmt::format("{}_unassign:", this->name)))
    {
        WorkerPanel::UnassignButton(id, ctx, event);

        auto parts = utils::Split(id, ':');
        dpp::snowflake user = parts[1];
        const std::string guid = parts[2];
        const auto job = ctx.queue->GetJobByGUID(guid);

        // Edit the original message
        event.reply(dpp::ir_update_message, SendPanel(ctx, event, job, user).set_flags(dpp::m_ephemeral));
        return;
    }
    else if (id.starts_with(fmt::format("{}_subscribe:", this->name)))
    {
        CustomerPanel::SubscribeButton(id, ctx, event);

        auto parts = utils::Split(id, ':');
        dpp::snowflake user = parts[1];
        const std::string guid = parts[2];
        const auto job = ctx.queue->GetJobByGUID(guid);

        // Edit the original message
        event.reply(dpp::ir_update_message, SendPanel(ctx, event, job, user).set_flags(dpp::m_ephemeral));
        return;
    }
    else if (id.starts_with(fmt::format("{}_delete:", this->name)))
    {
        GeneralUserPanel::DeleteButton(id, ctx, event);
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
dpp::message ShowRequestCommand::SendPanel(
    CommandContext& ctx, 
    const dpp::interaction_create_t& event, 
    const std::shared_ptr<const JobRequest>& job,
    const dpp::snowflake& user) const
{
    if (user == job->GetCustomerID() && user != job->GetWorkerID())
    {
        CustomerPanel panel(ctx, this->name, user, utils::GuidToStringNoBrackets(job->GetID()), job->IsCustomerSubscribed());
        panel.AddEmbed("Here is the Request", job->PrintJobDetails(ctx.cluster, event.command.guild_id));
        return panel;
    }
    else
    {
        WorkerPanel panel(ctx, this->name, user, utils::GuidToStringNoBrackets(job->GetID()), job->GetWorkerID());
        panel.AddEmbed("Here is the Request", job->PrintJobDetails(ctx.cluster, event.command.guild_id));
        return panel;
    }
}