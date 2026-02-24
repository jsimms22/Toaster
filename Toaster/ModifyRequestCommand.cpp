#include "BotUtility.h"
#include "Commands.h"
#include "JobQueue.h"
#include "RequestDlg.h"

#include "CraftingJobRequest.h"
#include "BuildingJobRequest.h"
#include "ComponentJobRequest.h"
#include "ResourceJobRequest.h"
#include "RefineryJobRequest.h"
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
        EditRequestDlg modal(job);
        event.dialog(modal);
    }
    else if (strCmdID == Option_Assign)
    {
        const std::string_view sv = job->GetWorkerName(ctx.cluster);
        AssignRequestDlg modal(job, ctx.workers, sv);
        event.dialog(modal);
    }
    else if (strCmdID == Option_Status)
    {
        StatusChangeRequestDlg modal(job);
        event.dialog(modal);
    }
    else if (strCmdID == Option_Priority)
    {
        PriorityChangeRequestDlg modal(job);
        event.dialog(modal);
    }
    else if (strCmdID == Option_Delete)
    {
        DeleteRequestDlg modal(job, job->PrintJobDetails(ctx.cluster));
        event.dialog(modal);
    }
}

void ModifyRequestCommand::ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event)
{
    if (!ctx.queue)
    {
        return;
    }

    if (event.custom_id == EditRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strSCHandle = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
        // These parameters are input that depend on the dialog form's order as defined by the type of the job
        const std::string strParam1 = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";
        const std::string strParam2 = event.components.size() > 3 ? std::get<std::string>(event.components[3].value) : "";
        const std::string strParam3 = event.components.size() > 4 ? std::get<std::string>(event.components[4].value) : "";

        std::shared_ptr<JobRequest> job = ctx.queue->GetJobByGUID(strID);
        if (!job || (author.id != job->GetCustomerID() && !ctx.debug/*|| todo get permissions list */))
        {
            ctx.cluster.log(dpp::ll_error, fmt::format("{} attempted to edit {}", author.username, strID));
            return;
        }
        job->SetLastEditTime(utils::GetEpochTimestamp());

        const std::string strOldJobDetails = job->PrintJobDetails(ctx.cluster);
        if (job->SupportsType(JOB_TYPE_CRAFTING))
        {
            std::shared_ptr<CraftingJobRequest> craft = std::dynamic_pointer_cast<CraftingJobRequest>(job);
            craft->SetSCHandle(strSCHandle);
            craft->SetItemDesc(strParam1);
            craft->SetQuantity(strParam2);
        }
        else if (job->SupportsType(JOB_TYPE_BUILDING))
        {
            std::shared_ptr<BuildingJobRequest> bldg = std::dynamic_pointer_cast<BuildingJobRequest>(job);
            bldg->SetSCHandle(strSCHandle);
            bldg->SetBuildDesign(strParam1);
            bldg->SetBuildRequirments(strParam2);
            bldg->SetBuildZone(strParam3);
        }
        else if (job->SupportsType(JOB_TYPE_COMPONENT))
        {
            std::shared_ptr<ComponentJobRequest> comp = std::dynamic_pointer_cast<ComponentJobRequest>(job);
            comp->SetSCHandle(strSCHandle);
            comp->SetComponentList(strParam1);
        }
        else if (job->SupportsType(JOB_TYPE_RESOURCE))
        {
            std::shared_ptr<ResourceJobRequest> resrc = std::dynamic_pointer_cast<ResourceJobRequest>(job);
            resrc->SetSCHandle(strSCHandle);
            resrc->SetResourcelist(strParam1);
        }
        else if (job->SupportsType(JOB_TYPE_REFINERY))
        {
            std::shared_ptr<RefineryJobRequest> jobRefine = std::dynamic_pointer_cast<RefineryJobRequest>(job);
            jobRefine->SetSCHandle(strSCHandle);
            jobRefine->SetResourcelist(strParam1);
            jobRefine->SetRefinery(strParam2);
        }

        const std::string strNewJobDetails = job->PrintJobDetails(ctx.cluster);
        dpp::embed embed;
        embed.set_title("Edited job:")
            .set_description(strNewJobDetails)
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        ctx.queue->SaveQueueToFile();

        ctx.cluster.log(dpp::ll_info, fmt::format("{} edited {}.", author.username, strID));

        if ((author.id != job->GetCustomerID() && strOldJobDetails != strNewJobDetails) || ctx.debug)
        {
            utils::NotifyIssuerMsg(ctx.cluster, job->GetCustomerID(), event,
                fmt::format("Request {} has been edited by {}:\n\n New Job Details:\n{}",
                    utils::GuidToStringNoBrackets(job->GetID()), author.username, strNewJobDetails));
        }
    }
    else if (event.custom_id == AssignRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const dpp::snowflake workerID = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
        const std::string strStatus = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";

        std::shared_ptr<JobRequest> job = ctx.queue->GetJobByGUID(strID);
        if (!job || (author.id != job->GetCustomerID() && !ctx.debug/*|| todo get permissions list */))
        {
            ctx.cluster.log(dpp::ll_error, fmt::format("{} attempted to assign {} to a worker.", author.username, strID));
            return;
        }

        job->SetLastEditTime(utils::GetEpochTimestamp());
        job->SetWorkerID(workerID);
        job->SetStatus(CraftingJobRequest::StringToStatus(strStatus));

        dpp::embed embed;
        embed.set_title("Assigned job:")
            .set_description(job->PrintJobDetails(ctx.cluster))
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        ctx.queue->SaveQueueToFile();

        const std::string strWorker = job->GetWorkerName(ctx.cluster);
        ctx.cluster.log(dpp::ll_info, fmt::format("{} assigned {} to {}", author.username, strID, strWorker));

        if (author.id != job->GetCustomerID() || ctx.debug)
        {
            utils::NotifyIssuerMsg(ctx.cluster, job->GetCustomerID(), event,
                fmt::format("Your request has been assigned to {} by {}:\n\n{}", strWorker, author.username, embed.description));
        }
    }
    else if (event.custom_id == StatusChangeRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strStatus = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";

        std::shared_ptr<JobRequest> job = ctx.queue->GetJobByGUID(strID);
        if (!job || (author.id != job->GetCustomerID() && !ctx.debug/*|| todo get permissions list */))
        {
            ctx.cluster.log(dpp::ll_error, fmt::format("{} attempted to change the status for {} to {}", author.username, strID, strStatus));
            return;
        }
        job->SetLastEditTime(utils::GetEpochTimestamp());

        const std::string strOldStatus = JobRequest::StatusToString(job->GetStatus());
        job->SetStatus(CraftingJobRequest::StringToStatus(strStatus));

        dpp::embed embed;
        embed.set_title("Status changed for job:")
            .set_description(job->PrintJobDetails(ctx.cluster))
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        ctx.queue->SaveQueueToFile();

        ctx.cluster.log(dpp::ll_info, fmt::format("{} updated the status for {} to {}", author.username, strID, strStatus));
        if ((author.id != job->GetCustomerID() && strOldStatus != strStatus) || ctx.debug)
        {
            utils::NotifyIssuerMsg(ctx.cluster, job->GetCustomerID(), event,
                fmt::format("Your request's status has moved from {} to {} by {}:\n\n{}", strOldStatus, strStatus, author.username, job->PrintJobDetails(ctx.cluster)));
        }
    }
    else if (event.custom_id == PriorityChangeRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strPriority = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";

        std::shared_ptr<JobRequest> job = ctx.queue->GetJobByGUID(strID);
        if (!job || (author.id != job->GetCustomerID() && !ctx.debug/*|| todo get permissions list */))
        {
            ctx.cluster.log(dpp::ll_error, fmt::format("{} attempted to change priority for {} to {}", author.username, strID, strPriority));
            return;
        }
        job->SetLastEditTime(utils::GetEpochTimestamp());

        const std::string strOldPriority = JobRequest::PriorityToString(job->GetPriority());
        job->SetPriority(JobRequest::StringToPriority(strPriority));

        dpp::embed embed;
        embed.set_title("Priority changed for job:")
            .set_description(job->PrintJobDetails(ctx.cluster))
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        ctx.queue->SaveQueueToFile();

        ctx.cluster.log(dpp::ll_info, fmt::format("{} updated the priority for {} to {}", author.username, strID, strPriority));
        if ((author.id != job->GetCustomerID() && strOldPriority != strPriority) || ctx.debug)
        {
            utils::NotifyIssuerMsg(ctx.cluster, job->GetCustomerID(), event,
                fmt::format("Your request's priority has moved from {} to {} by {}:\n\n{}", strOldPriority, strPriority, author.username, job->PrintJobDetails(ctx.cluster)));
        }
    }
    else if (event.custom_id == DeleteRequestDlg::modalID)
    {
        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        //const std::string strDesc = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
        const std::string strJustification = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";

        auto job = ctx.queue->GetJobByGUID(strID);
        if (!job)
        {
            event.reply(dpp::message("Job not found.")
                .set_flags(dpp::m_ephemeral));
            return;
        }

        const dpp::user author = event.command.get_issuing_user();

        job->SetLastEditTime(utils::GetEpochTimestamp());
        const std::string jobDetails = job->PrintJobDetails(ctx.cluster);
        const dpp::snowflake customerID = job->GetCustomerID();

        const bool result = ctx.queue->DeleteJobByGUID(strID);
        if (!result)
        {
            event.reply(dpp::message("Failed to delete job.")
                .set_flags(dpp::m_ephemeral));
            return;
        }

        ctx.queue->SaveQueueToFile();

        event.reply(dpp::message(
            fmt::format("Deleted job request.\n\n**Reason:** {}\n\n{}",
                strJustification, jobDetails))
            .set_flags(dpp::m_ephemeral));

        ctx.cluster.log(dpp::ll_warning,
            fmt::format("{} deleted {}. Reason: {}",
                author.username,
                strID,
                strJustification));

        // Notify original customer if needed
        if (event.command.usr.id != customerID || ctx.debug)
        {
            utils::NotifyIssuerMsg(ctx.cluster,
                                   customerID,
                                   event,
                                   fmt::format("Request {} has been deleted by {}.\n\n**Reason:** {}\n\n{}",
                                        strID,
                                        author.username,
                                        strJustification,
                                        jobDetails));
        }
    }
}