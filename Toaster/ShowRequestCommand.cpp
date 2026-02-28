//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief Implementation of the ShowRequestCommand.
///
/// Contains the implementation of ShowRequestCommand declared in Commands.h.
//---------------------------------------------------------------------------------------------------------------------
#include "Commands.h"
#include "JobQueue.h"
#include "BotUtility.h"
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

    const dpp::user author = event.command.get_issuing_user();

    std::string strJobID = std::get<std::string>(event.get_parameter(Parameter_Id));
    utils::FilterWhiteSpace(strJobID);

    const std::shared_ptr<JobRequest> job = ctx.queue->GetJobByGUID(strJobID);
    if (!job)
    {
        event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
        return;
    }

    if (!(ctx.manager->IsRequestOwner(author.id, job) ||
          ctx.manager->IsRequestWorker(author.id, job) ||
          ctx.manager->IsActiveWorker(author.id, ctx.workers) ||
          ctx.manager->IsGuildAdmin(author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id)) ||
          ctx.manager->IsBotOwner(author.id)))
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("USER '{}' was DENIED access to use '{}' command", author.id, event.command.get_command_name()));
        return;
    }

    dpp::embed embed;
    embed.set_title("Here is the request:")
        .set_description(job->PrintJobDetails(ctx.cluster, event.command.guild_id))
        .set_color(0x3498db);

    dpp::message msg;
    if (author.id == job->GetCustomerID() || author.id == job->GetWorkerID())
    {
        dpp::component row = CreateButtonRow(author.id, job);
        msg.add_component(row);
    }

    event.reply(msg.add_embed(embed).set_flags(dpp::m_ephemeral));
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Event handler for button interactions.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
//---------------------------------------------------------------------------------------------------------------------
void ShowRequestCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
    const std::string id = event.custom_id; // "show_request:type:workerid:guid"

    if (id.starts_with("show_request:"))
    {
        event.reply(dpp::message("These buttons are not functioning rn, but hello there!").set_flags(dpp::m_ephemeral));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Helper function to create a row of buttons for a job request.
///
/// Generates buttons like Complete, Edit, Add Note, Unassign, Subscribe, or Delete depending on whether the user 
/// is the worker or customer.
///
/// \param[out] user  The unique user ID for whom the buttons are generated.
/// \param[out] job   The job request object for which buttons are being created.
/// 
/// \return Returns a component object containing a row of buttons.
//---------------------------------------------------------------------------------------------------------------------
dpp::component ShowRequestCommand::CreateButtonRow(const dpp::snowflake& user, const std::shared_ptr<JobRequest>& job) const
{
    dpp::component row;

    const GUID jobID = job->GetID();

    if (user == job->GetWorkerID())
    {
        row.add_component(dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Complete")
            .set_style(dpp::cos_success)
            .set_id(fmt::format("show_request:complete:{}:{}", user, utils::GuidToStringNoBrackets(jobID))));
    }

    row.add_component(dpp::component()
        .set_type(dpp::cot_button)
        .set_label("Edit")
        .set_style(dpp::cos_primary)
        .set_id(fmt::format("show_request:edit:{}:{}", user, utils::GuidToStringNoBrackets(jobID))));

    row.add_component(dpp::component()
        .set_type(dpp::cot_button)
        .set_label("Add Note")
        .set_style(dpp::cos_primary)
        .set_id(fmt::format("show_request:note{}:{}", user, utils::GuidToStringNoBrackets(jobID))));

    if (user == job->GetCustomerID() && user != job->GetWorkerID())
    {
        row.add_component(dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Unsubscribed")
            .set_style(dpp::cos_secondary)
            .set_id(fmt::format("show_request:subscribe:{}:{}", user, utils::GuidToStringNoBrackets(jobID))));
    }
    else if (user == job->GetWorkerID())
    {
        row.add_component(dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Unassign Me")
            .set_style(dpp::cos_primary)
            .set_id(fmt::format("show_request:unassign:{}:{}", user, utils::GuidToStringNoBrackets(jobID))));
    }

    row.add_component(dpp::component()
        .set_type(dpp::cot_button)
        .set_label("Delete")
        .set_style(dpp::cos_danger)
        .set_id(fmt::format("show_request:delete:{}:{}", user, utils::GuidToStringNoBrackets(jobID))));

    return row;
}