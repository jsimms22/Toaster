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
    const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Cmd));

    std::string strJobID = std::get<std::string>(event.get_parameter(Parameter_Id));
    utils::FilterWhiteSpace(strJobID);

    const std::shared_ptr<JobRequest> job = ctx.queue->GetJobByGUID(strJobID);
    if (!job)
    {
        event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("{} could not find {} to handle cmd {}", author.global_name, strJobID, strCmdID));
        return;
    }

    if (strCmdID == Option_Edit && 
        ctx.manager->CanEditJob(author.id, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id)))
    {
        EditRequestDlg modal(job);
        event.dialog(modal);
        return;
    }
    else if (strCmdID == Option_Assign && 
            (ctx.manager->CanAssignJob(author.id, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id)) ||
             ctx.manager->IsGuildAdmin(author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id))))
    {
        const std::string current = job->GetWorkerName(ctx.cluster,event.command.guild_id);

        std::unordered_map<dpp::snowflake, std::string> mapWorkerNames;
        for (const auto& id : ctx.workers)
        {
            mapWorkerNames[id] = utils::FindPreferredNameByID(ctx.cluster, id, event.command.guild_id);
        }

        AssignRequestDlg modal(job, mapWorkerNames, current);
        event.dialog(modal);
        return;
    }
    else if (strCmdID == Option_Status &&
            (ctx.manager->CanAssignJob(author.id, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id)) ||
             ctx.manager->IsGuildAdmin(author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id))))
    {
        StatusChangeRequestDlg modal(job);
        event.dialog(modal);
        return;
    }
    else if (strCmdID == Option_Priority &&
            (ctx.manager->CanAssignJob(author.id, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id)) ||
             ctx.manager->IsGuildAdmin(author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id))))
    {
        PriorityChangeRequestDlg modal(job);
        event.dialog(modal);
        return;
    }
    else if (strCmdID == Option_Delete &&
        ctx.manager->CanDeleteJob(author.id, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id)))
    {
        DeleteRequestDlg modal(job, job->PrintJobDetails(ctx.cluster, event.command.guild_id));
        event.dialog(modal);
        return;
    }
    else
    {
        event.reply(dpp::message(fmt::format("You do not have permissions to modify {}.", strJobID)).set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("{} attempted to modify job {} with command {}.", author.global_name, strJobID, strCmdID));
        return;
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
        std::string strSCHandle = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
        // These parameters are input that depend on the dialog form's order as defined by the type of the job
        std::string strParam1 = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";
        std::string strParam2 = event.components.size() > 3 ? std::get<std::string>(event.components[3].value) : "";
        std::string strParam3 = event.components.size() > 4 ? std::get<std::string>(event.components[4].value) : "";

        std::shared_ptr<JobRequest> job = ctx.queue->GetJobByGUID(strID);
        if (!job)
        {
            event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
            return;
        }

        utils::FilterUserString(strSCHandle);
        utils::FilterUserString(strParam1);
        utils::FilterUserString(strParam2);
        utils::FilterUserString(strParam3);

        job->SetLastEditTime(utils::GetEpochTimestamp());
        const std::string strOldJobDetails = job->PrintJobDetails(ctx.cluster, event.command.guild_id);
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

        dpp::component row;
        dpp::component button1 = dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Edit")
            .set_style(dpp::cos_primary)
            .set_id(fmt::format("modify_edit:{}:{}", author.id, strID));
        row.add_component(button1);

        dpp::component button2 = dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Add Note")
            .set_style(dpp::cos_primary)
            .set_id(fmt::format("modify_note:{}:{}", author.id, strID));
        row.add_component(button2);

        if (job->GetCustomerID() == author.id)
        {
            dpp::component button3 = dpp::component()
                .set_type(dpp::cot_button)
                .set_label(job->IsCustomerSubscribed() ? "Subscribed" : "Unsubscribed")
                .set_style(job->IsCustomerSubscribed() ? dpp::cos_primary : dpp::cos_secondary)
                .set_id(fmt::format("modify_subscribe:{}:{}", author.id, strID));
            row.add_component(button3);
        }

        dpp::component button4 = dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Delete")
            .set_style(dpp::cos_danger)
            .set_id(fmt::format("modify_delete:{}:{}", author.id, strID));
        row.add_component(button4);

        const std::string strNewJobDetails = job->PrintJobDetails(ctx.cluster, event.command.guild_id);
        dpp::embed embed;
        embed.set_title("Edited job:")
            .set_description(strNewJobDetails)
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).add_component(row).set_flags(dpp::m_ephemeral));
        ctx.queue->SaveQueueToFile();

        ctx.cluster.log(dpp::ll_info, fmt::format("{} edited {}.", author.global_name, strID));

        if ((job->IsCustomerSubscribed() && author.id != job->GetCustomerID() && strOldJobDetails != strNewJobDetails) || ctx.debug)
        {
            utils::NotifyIssuerMsg(ctx.cluster, job->GetCustomerID(), event,
                fmt::format("Request {} has been edited by {}:\n\n New Job Details:\n{}",
                    utils::GuidToStringNoBrackets(job->GetID()), author.global_name, strNewJobDetails));
        }
    }
    else if (event.custom_id == AssignRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const dpp::snowflake workerID = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
        const std::string strStatus = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";

        std::shared_ptr<JobRequest> job = ctx.queue->GetJobByGUID(strID);
        if (!job)
        {
            event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
            return;
        }

        job->SetLastEditTime(utils::GetEpochTimestamp());
        job->SetWorkerID(workerID);
        job->SetStatus(CraftingJobRequest::StringToStatus(strStatus));

        dpp::embed embed;
        embed.set_title("Assigned job:")
            .set_description(job->PrintJobDetails(ctx.cluster, event.command.guild_id))
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        ctx.queue->SaveQueueToFile();

        const std::string strWorker = job->GetWorkerName(ctx.cluster, event.command.guild_id);
        ctx.cluster.log(dpp::ll_info, fmt::format("{} assigned {} to {}", author.global_name, strID, strWorker));

        if ((job->IsCustomerSubscribed() && author.id != job->GetCustomerID()) || ctx.debug)
        {
            utils::NotifyIssuerMsg(ctx.cluster, job->GetCustomerID(), event,
                fmt::format("Your request has been assigned to {} by {}:\n\n{}", strWorker, author.global_name, embed.description));
        }
    }
    else if (event.custom_id == StatusChangeRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strStatus = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";

        std::shared_ptr<JobRequest> job = ctx.queue->GetJobByGUID(strID);
        if (!job)
        {
            event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
            return;
        }

        job->SetLastEditTime(utils::GetEpochTimestamp());
        const std::string strOldStatus = JobRequest::StatusToString(job->GetStatus());
        job->SetStatus(CraftingJobRequest::StringToStatus(strStatus));

        dpp::embed embed;
        embed.set_title("Status changed for job:")
            .set_description(job->PrintJobDetails(ctx.cluster, event.command.guild_id))
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        ctx.queue->SaveQueueToFile();

        ctx.cluster.log(dpp::ll_info, fmt::format("{} updated the status for {} to {}", author.global_name, strID, strStatus));
        if ((job->IsCustomerSubscribed() && author.id != job->GetCustomerID() && strOldStatus != strStatus) || ctx.debug)
        {
            utils::NotifyIssuerMsg(ctx.cluster, job->GetCustomerID(), event,
                fmt::format("Your request's status has moved from {} to {} by {}:\n\n{}", strOldStatus, strStatus, author.global_name, job->PrintJobDetails(ctx.cluster, event.command.guild_id)));
        }
    }
    else if (event.custom_id == PriorityChangeRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strPriority = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";

        std::shared_ptr<JobRequest> job = ctx.queue->GetJobByGUID(strID);
        if (!job)
        {
            event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
            return;
        }

        job->SetLastEditTime(utils::GetEpochTimestamp());
        const std::string strOldPriority = JobRequest::PriorityToString(job->GetPriority());
        job->SetPriority(JobRequest::StringToPriority(strPriority));

        dpp::embed embed;
        embed.set_title("Priority changed for job:")
            .set_description(job->PrintJobDetails(ctx.cluster, event.command.guild_id))
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        ctx.queue->SaveQueueToFile();

        ctx.cluster.log(dpp::ll_info, fmt::format("{} updated the priority for {} to {}", author.global_name, strID, strPriority));
        if ((job->IsCustomerSubscribed() && author.id != job->GetCustomerID() && strOldPriority != strPriority) || ctx.debug)
        {
            utils::NotifyIssuerMsg(ctx.cluster, job->GetCustomerID(), event,
                fmt::format("Your request's priority has moved from {} to {} by {}:\n\n{}", strOldPriority, strPriority, author.global_name, job->PrintJobDetails(ctx.cluster, event.command.guild_id)));
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
            event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
            return;
        }

        const dpp::user author = event.command.get_issuing_user();

        job->SetLastEditTime(utils::GetEpochTimestamp());
        const std::string jobDetails = job->PrintJobDetails(ctx.cluster, event.command.guild_id);
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
                author.global_name,
                strID,
                strJustification));

        // Notify original customer if needed
        if ((job->IsCustomerSubscribed() && event.command.usr.id != customerID) || ctx.debug)
        {
            utils::NotifyIssuerMsg(ctx.cluster,
                                   customerID,
                                   event,
                                   fmt::format("Request {} has been deleted by {}.\n\n**Reason:** {}\n\n{}",
                                        strID,
                                        author.global_name,
                                        strJustification,
                                        jobDetails));
        }
    }
}

void ModifyRequestCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
    const std::string id = event.custom_id; // "modify_type:workerid:guid"

    if (id.starts_with("modify_edit:"))
    {
        auto parts = utils::Split(id, ':');
        const dpp::snowflake user = parts[1];
        const std::string guid = parts[2];

        auto job = ctx.queue->GetJobByGUID(guid);
        if (job &&
            ctx.manager->CanEditJob(user, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id)))
        {
            EditRequestDlg modal(job);
            event.dialog(modal);
        }
        else if (!job)
        {
            event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
            return;
        }
        else
        {
            event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
            ctx.cluster.log(dpp::ll_debug, fmt::format("USER '{}' was DENIED access to use '{}' button", user, parts[0]));
            return;
        }
    }
    else if (id.starts_with("modify_note:"))
    {
        auto parts = utils::Split(id, ':');
        const dpp::snowflake user = parts[1];
        const std::string guid = parts[2];

        auto job = ctx.queue->GetJobByGUID(guid);
        if (job && ctx.manager->CanAddNote(user, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id)))
        {
            /* todo add note functionality to job class */
            event.reply(dpp::message("Functionality currently not supported.").set_flags(dpp::m_ephemeral));
            return;

            ctx.queue->SaveQueueToFile();
            // todo note modal dlg
        }
        else if (!job)
        {
            event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
            return;
        }
        else
        {
            event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
            ctx.cluster.log(dpp::ll_debug, fmt::format("USER '{}' was DENIED access to use '{}' button", user, parts[0]));
            return;
        }
    }
    else if (id.starts_with("modify_subscribe:"))
    {
        auto parts = utils::Split(id, ':');
        const dpp::snowflake user = parts[1];
        const std::string guid = parts[2];

        auto job = ctx.queue->GetJobByGUID(guid);
        if (job && job->GetCustomerID() == user)
        {
            const std::string strJobID = utils::GuidToStringNoBrackets(job->GetID());
            const dpp::user author = event.command.get_issuing_user();

            const std::size_t timestamp = utils::GetEpochTimestamp();
            job->SubscribeCustomer(!job->IsCustomerSubscribed());
            job->SetLastEditTime(timestamp);
            ctx.queue->SaveQueueToFile();

            dpp::component button1 = dpp::component()
                .set_type(dpp::cot_button)
                .set_label("Edit")
                .set_style(dpp::cos_primary)
                .set_id(fmt::format("modify_edit:{}:{}", author.id, strJobID));

            dpp::component button2 = dpp::component()
                .set_type(dpp::cot_button)
                .set_label("Add Note")
                .set_style(dpp::cos_primary)
                .set_id(fmt::format("modify_note:{}:{}", author.id, strJobID));

            dpp::component button3 = dpp::component()
                .set_type(dpp::cot_button)
                .set_label(job->IsCustomerSubscribed() ? "Subscribed" : "Unsubscribed")
                .set_style(job->IsCustomerSubscribed() ? dpp::cos_primary : dpp::cos_secondary)
                .set_id(fmt::format("modify_subscribe:{}:{}", author.id, strJobID));

            dpp::component button4 = dpp::component()
                .set_type(dpp::cot_button)
                .set_label("Delete")
                .set_style(dpp::cos_danger)
                .set_id(fmt::format("modify_delete:{}:{}", author.id, strJobID));

            dpp::component row = dpp::component()
                .set_type(dpp::cot_action_row)
                .add_component(button1)
                .add_component(button2)
                .add_component(button3)
                .add_component(button4);

            dpp::embed embed;
            embed.set_title("Submitted job request:")
                .set_description(job->PrintJobDetails(ctx.cluster, event.command.guild_id))
                .set_color(0x3498db);

            event.reply(dpp::ir_update_message,
                dpp::message().add_embed(embed).add_component(row).set_flags(dpp::m_ephemeral));

            ctx.cluster.log(dpp::ll_info, fmt::format("User changed subscribed status for {} to {}", strJobID,
                !job->IsCustomerSubscribed() ? "Unsubscribe" : "Subscribe"));
        }
        else if (!job)
        {
            event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
            return;
        }
        else
        {
            event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
            ctx.cluster.log(dpp::ll_debug, fmt::format("USER '{}' was DENIED access to use '{}' button", user, parts[0]));
            return;
        }
    }
    else if (id.starts_with("modify_delete:"))
    {
        auto parts = utils::Split(id, ':');
        const dpp::snowflake user = parts[1];
        const std::string guid = parts[2];

        auto job = ctx.queue->GetJobByGUID(guid);
        if (job && ctx.manager->CanDeleteJob(user, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id)))
        {
            DeleteRequestDlg modal(job, job->PrintJobDetails(ctx.cluster, event.command.guild_id));
            event.dialog(modal);
        }
        else if (!job)
        {
            event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
            return;
        }
        else
        {
            event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
            ctx.cluster.log(dpp::ll_debug, fmt::format("USER '{}' was DENIED access to use '{}' button", user, parts[0]));
            return;
        }
    }
}