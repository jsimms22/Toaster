#include "BotUtility.h"
#include "Commands.h"
#include "JobQueue.h"
#include "RequestDlg.h"
// fmt
#include <fmt/format.h>

void ModifyRequestCommand::ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event)
{
}

void ModifyRequestCommand::ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::snowflake guild = event.command.guild_id;
    const dpp::user author = event.command.get_issuing_user();
    const std::string strJobID = std::get<std::string>(event.get_parameter(Parameter_Id));
    const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Cmd));

    const std::shared_ptr<JobRequest> job = ctx.queue->GetJobByGUID(strJobID);
    if (!job)
    {
        event.reply(dpp::message(fmt::format("Could not find id {}.", strJobID)).set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("{} could not find {} to handle cmd {}", author.username, strJobID, strCmdID));
        return;
    }
    // Check for permissions to edit a job request
    else if (author.id != job->GetCustomerID() && !ctx.debug/*|| todo get permissions list */)
    {
        event.reply(dpp::message(fmt::format("You do not have permissions to modify {}.", strJobID)).set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("{} attempted to modify job {} with command {}.", author.username, strJobID, strCmdID));
        return;
    }

    if (strCmdID == Option_Edit)
    {
        EditRequestDlg editModal(job);
        event.dialog(editModal);
    }
    else if (strCmdID == Option_Assign)
    {
        const std::string_view sv = job->GetWorkerName(ctx.cluster);
        AssignRequestDlg assignModal(job, ctx.workers, sv);
        event.dialog(assignModal);
    }
    else if (strCmdID == Option_Status)
    {
        StatusChangeRequestDlg statusModal(job);
        event.dialog(statusModal);
    }
    else if (strCmdID == Option_Priority)
    {
        PriorityChangeRequestDlg statusModal(job);
        event.dialog(statusModal);
    }
    else if (strCmdID == Option_Delete)
    {
        job->SetLastEditTime(utils::GetEpochTimestamp());
        const std::string jobDetails = job->PrintJobDetails(ctx.cluster);
        const bool result = ctx.queue->DeleteJobByGUID(strJobID);
        if (!result)
        {
            event.reply(dpp::message(fmt::format("Failed to delete id: ", strJobID)).set_flags(dpp::m_ephemeral));
            ctx.cluster.log(dpp::ll_warning, fmt::format("{} failed to delete {}.", author.username, strJobID));
        }

        event.reply(dpp::message(fmt::format("Deleted job request:\n\n{}", jobDetails)).set_flags(dpp::m_ephemeral));
        ctx.queue->SaveQueueToFile();
        ctx.cluster.log(dpp::ll_warning, fmt::format("{} deleted {}", author.username, strJobID));

        if (author.id != job->GetCustomerID() && !ctx.debug/*|| todo get permissions list */)
        {
            utils::NotifyIssuerMsg(ctx.cluster, job->GetCustomerID(), event,
                fmt::format("Request {} has been deleted by {}:\n\n{}", strJobID, author.username, jobDetails));
        }
    }
}

void ModifyRequestCommand::ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event)
{
}