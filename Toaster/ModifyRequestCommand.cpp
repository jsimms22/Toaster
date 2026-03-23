//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief Implementation of ModifyRequestCommand.
///
/// Handles modification workflows for existing job requests.
/// 
/// All operations enforce permission validation through the command manager before executing any state changes.
//---------------------------------------------------------------------------------------------------------------------
#include "BotUtility.h"
#include "Commands.h"
#include "JobQueue.h"
#include "RequestDlg.h"
#include "RequestID.h"
#include "NoteDialog.h"
#include "GuildSettings.h"
#include "PermissionsMgr.h"

#include "CraftingJobRequest.h"
#include "BuildingJobRequest.h"
#include "ComponentJobRequest.h"
#include "ResourceJobRequest.h"
#include "RefineryJobRequest.h"
#include "HazardousRequest.h"

#include "CustomerPanel.h"
#include "GlobalButtonPanel.h"
#include "WorkerPanel.h"
// fmt
#include <fmt/format.h>
// std library
#include <future>

//---------------------------------------------------------------------------------------------------------------------
/// \brief Handles the initial slash command interaction for job modification.
/// 
/// Depending on the selected command option, displays the corresponding modal dialog (Edit, Assign, Status, 
/// Priority, Delete).
///
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Interaction create event.
//---------------------------------------------------------------------------------------------------------------------
void ModifyRequestCommand::ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Cmd));

    std::string strJobID = std::get<std::string>(event.get_parameter(Parameter_Id));
    utils::FilterWhiteSpace(strJobID);

    const auto job = ctx.queue->GetJobByID(strJobID);
    if (!job)
    {
        event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("{} could not find {} to handle cmd {}", author.global_name, strJobID, strCmdID));
        return;
    }

    const auto pManager = PermissionsMgr::GetInstance();
    dpp::guild* pGuild = utils::FindGuildByID(ctx.cluster, event.command.guild_id);

    if (strCmdID == Option_Edit && pGuild && (pManager->CanEditJob(event, author.id, job, pGuild, ctx.guild) || ctx.debug))
    {
        EditRequestDlg modal(job);
        event.dialog(modal);
        return;
    }
    else if (strCmdID == Option_Assign && pGuild && (pManager->CanAssignJob(event, author.id, job, pGuild, ctx.guild) || ctx.debug))
    {
        AssignRequestDlg modal(job);
        event.dialog(modal);
        return;
    }
    else if (strCmdID == Option_Status && pGuild && (pManager->CanAssignJob(event, author.id, job, pGuild, ctx.guild) || ctx.debug))
    {
        StatusChangeRequestDlg modal(job);
        event.dialog(modal);
        return;
    }
    else if (strCmdID == Option_Priority && pGuild && (pManager->CanAssignJob(event, author.id, job, pGuild, ctx.guild) || ctx.debug))
    {
        PriorityChangeRequestDlg modal(job);
        event.dialog(modal);
        return;
    }
    else if (strCmdID == Option_Delete && pGuild && (pManager->CanDeleteJob(event, author.id, job, pGuild, ctx.guild) || ctx.debug))
    {
        std::string desc = job->PrintJobDetails(ctx.cluster, event.command.guild_id, true);
        utils::RemoveChar(desc, '*');
        DeleteRequestDlg modal(job, desc);
        event.dialog(modal);
        return;
    }
    else if (strCmdID == Option_Note && pGuild && (pManager->CanAddNote(event, author.id, job, pGuild, ctx.guild) || ctx.debug))
    {
        NoteDialog modal(ctx, job);
        event.dialog(modal);
        return;
    }
    else
    {
        // Permission denied
        event.reply(dpp::message(fmt::format("You do not have permissions to modify {}.", strJobID)).set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("{} attempted to modify job {} with command {}.", author.global_name, strJobID, strCmdID));
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Processes modal form submissions for job modifications. 
/// 
/// Applies changes through JobQueue::RequestModifyJob and generates ephemeral confirmation responses. Subscribed 
/// users are notified when applicable.
///
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Modal form submission event.
//---------------------------------------------------------------------------------------------------------------------
void ModifyRequestCommand::ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event)
{
    if (!ctx.queue)
    {
        return;
    }

    const std::string id = event.custom_id;
    auto parts = utils::Split(id, ':');

    //-------------------------------------------------------------------------------------------------------------
    // Handle EditRequestDlg
    //-------------------------------------------------------------------------------------------------------------
    if (parts[0] == EditRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        std::string strSCHandle = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
        // These parameters are input that depend on the dialog form's order as defined by the type of the job
        std::string strParam1 = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";
        std::string strParam2 = event.components.size() > 3 ? std::get<std::string>(event.components[3].value) : "";
        std::string strParam3 = event.components.size() > 4 ? std::get<std::string>(event.components[4].value) : "";

        const auto job = ctx.queue->GetJobByID(strID);
        if (!job)
        {
            event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
            return;
        }

        // Acknowledge immediately to avoid timing out
        event.reply(dpp::message("...this could take a second. Please hold.").set_flags(dpp::m_ephemeral));

        utils::FilterUserString(strSCHandle);
        utils::FilterUserString(strParam1);
        utils::FilterUserString(strParam2);
        utils::FilterUserString(strParam3);

        const std::string strOldJobDetails = job->PrintJobDetails(ctx.cluster, event.command.guild_id);
        if (job->SupportsType(JOB_TYPE_CRAFTING))
        {
            ctx.queue->RequestModify(job->GetID(),
                [strSCHandle, strParam1, strParam2](std::shared_ptr<JobRequest> job)
                {
                    std::shared_ptr<CraftingJobRequest> craft = std::dynamic_pointer_cast<CraftingJobRequest>(job);
                    craft->SetSCHandle(strSCHandle);
                    craft->SetItemDesc(strParam1);
                    craft->SetQuantity(strParam2);
                });
        }
        else if (job->SupportsType(JOB_TYPE_BUILDING))
        {
            ctx.queue->RequestModify(job->GetID(),
                [strSCHandle, strParam1, strParam2, strParam3](std::shared_ptr<JobRequest> job)
                {
                    std::shared_ptr<BuildingJobRequest> bldg = std::dynamic_pointer_cast<BuildingJobRequest>(job);
                    bldg->SetSCHandle(strSCHandle);
                    bldg->SetBuildDesign(strParam1);
                    bldg->SetBuildRequirments(strParam2);
                    bldg->SetBuildZone(strParam3);
                });
        }
        else if (job->SupportsType(JOB_TYPE_COMPONENT))
        {
            ctx.queue->RequestModify(job->GetID(),
                [strSCHandle, strParam1](std::shared_ptr<JobRequest> job)
                {
                    std::shared_ptr<ComponentJobRequest> comp = std::dynamic_pointer_cast<ComponentJobRequest>(job);
                    comp->SetSCHandle(strSCHandle);
                    comp->SetComponentList(strParam1);
                });
        }
        else if (job->SupportsType(JOB_TYPE_RESOURCE))
        {
            ctx.queue->RequestModify(job->GetID(),
                [strSCHandle, strParam1](std::shared_ptr<JobRequest> job)
                {
                    std::shared_ptr<ResourceJobRequest> resrc = std::dynamic_pointer_cast<ResourceJobRequest>(job);
                    resrc->SetSCHandle(strSCHandle);
                    resrc->SetResourcelist(strParam1);
                });
        }
        else if (job->SupportsType(JOB_TYPE_REFINERY))
        {
            ctx.queue->RequestModify(job->GetID(),
                [strSCHandle, strParam1, strParam2](std::shared_ptr<JobRequest> job)
                {
                    std::shared_ptr<RefineryJobRequest> jobRefine = std::dynamic_pointer_cast<RefineryJobRequest>(job);
                    jobRefine->SetSCHandle(strSCHandle);
                    jobRefine->SetResourcelist(strParam1);
                    jobRefine->SetRefinery(strParam2);
                });
        }
        else if (job->SupportsType(JOB_TYPE_HAZARD))
        {
            ctx.queue->RequestModify(job->GetID(),
                [strSCHandle, strParam1, strParam2, strParam3](std::shared_ptr<JobRequest> job)
                {
                    std::shared_ptr<HazardousRequest> jobHazard = std::dynamic_pointer_cast<HazardousRequest>(job);
                    jobHazard->SetSCHandle(strSCHandle);
                    jobHazard->SetThreatLevel(HazardousRequest::StringToThreat(strParam1));
                    jobHazard->SetItemLocation(strParam2);
                    jobHazard->SetItemList(strParam3);
                });
        }

        // Edit the original message
        event.edit_original_response(SendPanel(ctx, event, job, author.id).set_flags(dpp::m_ephemeral));

        ctx.cluster.log(dpp::ll_info, fmt::format("{} edited {}.", author.global_name, strID));

        const std::string jobDetails = job->PrintJobDetails(ctx.cluster, event.command.guild_id);

        const dpp::snowflake customer = job->GetCustomerID();
        if ((job->IsCustomerSubscribed() && author.id != customer && strOldJobDetails != jobDetails) || ctx.debug)
        {
            utils::NotifyIssuerMsgWithEmbed(
                ctx.cluster,
                event,
                customer,
                fmt::format("{} has edited your request {}.", fmt::format("<@{}>", author.id), ToString(job->GetID())),
                "Request Details",
                job->PrintJobDetails(ctx.cluster, ctx.guild->GetGuildID())
            );
        }

        GuildSettings::AnnounceOnUpdate(ctx, job);
    }
    //-------------------------------------------------------------------------------------------------------------
// Handle AssignRequestDlg
//-------------------------------------------------------------------------------------------------------------
else if (parts[0] == AssignRequestDlg::modalID)
{
    const dpp::user author = event.command.get_issuing_user();
    const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
    const std::string action = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "1";
    std::string workerID1 = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";
    std::string workerID2 = event.components.size() > 3 ? std::get<std::string>(event.components[3].value) : "";
    const std::string strStatus = event.components.size() > 4 ? std::get<std::string>(event.components[4].value) : "";

    const auto job = ctx.queue->GetJobByID(strID);
    if (!job)
    {
        event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
        return;
    }

    auto is_all_numbers = [](const std::string& s) -> bool {
        return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) {
            return std::isdigit(c);
            });
        };

    auto pManager = PermissionsMgr::GetInstance();
    auto* pGuild = utils::FindGuildByID(ctx.cluster, event.command.guild_id);

    fmt::memory_buffer workerbuf;
    fmt::memory_buffer errorbuf;

    bool bModified = false;

    std::array<std::string, 2> workers = { workerID1, workerID2 };
    for (auto& worker : workers)
    {
        utils::FilterUserString(worker);
        if (worker.empty())
            continue;

        if (!is_all_numbers(worker))
        {
            fmt::format_to(std::back_inserter(errorbuf), "Invalid input for id {}.\n", worker);
            continue;
        }

        if (!pManager->IsWorker(worker, pGuild, ctx.guild))
        {
            fmt::format_to(std::back_inserter(errorbuf), "User id <@{}> does not have a worker role.\n", worker);
            continue;
        }

        fmt::format_to(std::back_inserter(workerbuf), "<@{}> ", worker);

        ctx.queue->RequestModify(job->GetID(),
            [worker, strStatus, action](std::shared_ptr<JobRequest> job)
            {
                bool bResult = false;

                if (action == "1") // Assign
                    bResult = job->AddWorkerID(worker);
                else               // Unassign
                    bResult = job->RemoveWorkerID(worker);

                if (bResult)
                    job->SetStatus(JobRequest::StringToStatus(strStatus));
            });

        bModified = true;
    }

    fmt::memory_buffer msg;
    if (bModified)
    {
        fmt::format_to(std::back_inserter(msg), "Workers updated: {}\n", fmt::to_string(workerbuf));
    }

    if (errorbuf.size() != 0)
    {
        fmt::format_to(std::back_inserter(msg), "\nErrors:\n{}", fmt::to_string(errorbuf));
    }

    if (msg.size() != 0)
    {
        const std::string jobDetails = job->PrintJobDetails(ctx.cluster, event.command.guild_id);

        dpp::embed embed;
        embed.set_title("Updated Assignments")
            .set_description(jobDetails)
            .set_color(0x3498db);

        event.reply(
            dpp::message(fmt::to_string(msg))
            .add_embed(embed)
            .set_flags(dpp::m_ephemeral));

        ctx.cluster.log(dpp::ll_info, fmt::format("{} updated {} with [{}]", author.global_name, strID, fmt::to_string(workerbuf)));

        const dpp::snowflake customer = job->GetCustomerID();
        if ((job->IsCustomerSubscribed() && author.id != customer) || ctx.debug)
        {
            utils::NotifyIssuerMsgWithEmbed(
                ctx.cluster,
                event,
                customer,
                fmt::format("{} were updated on your request by <@{}>.", fmt::to_string(workerbuf), author.id),
                "Request Details",
                jobDetails
            );
        }
    }
    }
    //-------------------------------------------------------------------------------------------------------------
    // Handle StatusChangeRequestDlg
    //-------------------------------------------------------------------------------------------------------------
    else if (parts[0] == StatusChangeRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strNewStatus = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";

        const auto job = ctx.queue->GetJobByID(strID);
        if (!job)
        {
            event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
            return;
        }

        const auto oldStatus = job->GetStatus();
        ctx.queue->RequestModify(job->GetID(),
            [strNewStatus](std::shared_ptr<JobRequest> job)
            {
                job->SetStatus(CraftingJobRequest::StringToStatus(strNewStatus));
            });

        const auto newStatus = job->GetStatus();
        const std::string jobDetails = job->PrintJobDetails(ctx.cluster, event.command.guild_id);

        dpp::embed embed;
        embed.set_title("Status changed for job:")
            .set_description(jobDetails)
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

        ctx.cluster.log(dpp::ll_info, fmt::format("{} updated the status for {} to {}", author.global_name, strID, strNewStatus));

        const dpp::snowflake customer = job->GetCustomerID();
        if ((job->IsCustomerSubscribed() && author.id != customer && oldStatus != newStatus) || ctx.debug)
        {
            utils::NotifyIssuerMsgWithEmbed(
                ctx.cluster,
                event,
                customer,
                fmt::format("Your request's status has moved from {} to {} by {}.", JobRequest::StatusToString(oldStatus), strNewStatus, fmt::format("<@{}>", author.id)),
                "Request Details",
                jobDetails
            );
        }

        if (oldStatus != JobRequest::status::complete && newStatus != oldStatus && newStatus == JobRequest::status::complete)
        {
            GuildSettings::AnnounceOnComplete(ctx, job);
        }
    }
    //-------------------------------------------------------------------------------------------------------------
    // Handle PriorityChangeRequestDlg
    //-------------------------------------------------------------------------------------------------------------
    else if (parts[0] == PriorityChangeRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strPriority = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";

        const auto job = ctx.queue->GetJobByID(strID);
        if (!job)
        {
            event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
            return;
        }

        const std::string strOldPriority = JobRequest::PriorityToString(job->GetPriority());
        ctx.queue->RequestModify(job->GetID(),
            [strPriority](std::shared_ptr<JobRequest> job)
            {
                job->SetPriority(JobRequest::StringToPriority(strPriority));
            });

        const std::string jobDetails = job->PrintJobDetails(ctx.cluster, event.command.guild_id);

        dpp::embed embed;
        embed.set_title("Priority changed for job:")
            .set_description(jobDetails)
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

        ctx.cluster.log(dpp::ll_info, fmt::format("{} updated the priority for {} to {}", author.global_name, strID, strPriority));

        const dpp::snowflake customer = job->GetCustomerID();
        if ((job->IsCustomerSubscribed() && author.id != job->GetCustomerID() && strOldPriority != strPriority) || ctx.debug)
        {
            utils::NotifyIssuerMsgWithEmbed(
                ctx.cluster,
                event,
                customer,
                fmt::format("Your request's priority has moved from {} to {} by {}.", strOldPriority, strPriority, fmt::format("<@{}>", author.id)),
                "Request Details",
                jobDetails
            );
        }
    }
    //-------------------------------------------------------------------------------------------------------------
    // Handle DeleteRequestDlg
    //-------------------------------------------------------------------------------------------------------------
    else if (parts[0] == DeleteRequestDlg::modalID)
    {
        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        //const std::string strDesc = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
        std::string strJustification = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";

        const auto job = ctx.queue->GetJobByID(strID);
        if (!job)
        {
            event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
            return;
        }

        const auto rID = job->GetID();
        const dpp::user author = event.command.get_issuing_user();
        const std::string jobDetails = job->PrintJobDetails(ctx.cluster, event.command.guild_id);
        const std::size_t jobType = job->JobType();
        const dpp::snowflake customer = job->GetCustomerID();
        const bool bNotifyCustomer = job->IsCustomerSubscribed();

        const bool result = ctx.queue->RequestDelete(rID);
        if (!result)
        {
            event.reply(dpp::message("Failed to delete job.")
                .set_flags(dpp::m_ephemeral));
            return;
        }

        utils::FilterUserString(strJustification);
        dpp::embed embed;
        embed.set_title("Job Request Deleted:")
            .set_description(fmt::format("**Reason:** {}\n\n{}", strJustification, jobDetails))
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning,fmt::format("{} deleted {}. Reason: {}",author.global_name,strID,strJustification));

        // Notify original customer if needed
        if ((bNotifyCustomer && event.command.usr.id != customer) || ctx.debug)
        {
            utils::NotifyIssuerMsgWithEmbed(
                ctx.cluster,
                event,
                customer,
                fmt::format("Request {} has been deleted by {}.\n\n**Reason:** {}", strID, fmt::format("<@{}>", author.id), strJustification),
                "Request Details",
                jobDetails
            );
        }

        GuildSettings::AnnounceOnDelete(ctx, jobType, jobDetails);
    }
    //-------------------------------------------------------------------------------------------------------------
    // Handle NoteDlg
    //-------------------------------------------------------------------------------------------------------------
    else if (parts[0] == NoteDialog::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        // Grab the job ID and note from the modal components
        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        std::string strNote = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";

        const auto job = ctx.queue->GetJobByID(strID);
        if (!job)
        {
            event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.")
                .set_flags(dpp::m_ephemeral));
            return;
        }

        // Filter the note string for safety
        utils::FilterUserString(strNote);
        if (strNote.empty())
        {
            event.reply(dpp::message("Cannot add an empty note.").set_flags(dpp::m_ephemeral));
            return;
        }

        // Record the note with timestamp
        JobRequest::NoteMetaData noteEntry;
        noteEntry.guildID = event.command.guild_id;
        noteEntry.timestamp = utils::GetEpochTimestamp();
        noteEntry.note = strNote;

        ctx.queue->RequestModify(job->GetID(),
            [authorID = author.id, noteEntry](std::shared_ptr<JobRequest> job)
            {
                job->AddNote(authorID, std::move(noteEntry));
            });

        // Acknowledge immediately
        event.reply(dpp::message("Your note has been added to this request.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_info, fmt::format("{} added a note to {}.", author.global_name, strID));

        // Notify original customer if needed
        const dpp::snowflake customer = job->GetCustomerID();
        if ((job->IsCustomerSubscribed() && event.command.usr.id != customer) || ctx.debug)
        {
            utils::NotifyIssuerMsgWithEmbed(
                ctx.cluster,
                event,
                customer,
                fmt::format("{} has added a note to your request {}.\n\n **Note:**\n{}", fmt::format("<@{}>", author.id), strID, strNote),
                "Request Details",
                job->PrintJobDetails(ctx.cluster, ctx.guild->GetGuildID())
            );
        }
        // todo: need a way to allow workers to stop notifications
        /*
        else if (!job->IsWorker(event.command.usr.id) || ctx.debug)
        {
            utils::NotifyIssuerMsg(ctx.cluster,
                job->GetWorkerID(),
                event,
                fmt::format("A note has been added by {} to job request {}.\n\n **Note:**\n{}",
                    utils::FindPreferredNameByID(ctx.cluster, author.id, event.command.guild_id),
                    strID,
                    strNote));
        }
        */
    }
    //-------------------------------------------------------------------------------------------------------------
    // Handle ReOpenArchivedJobDlg
    //-------------------------------------------------------------------------------------------------------------
    else if (parts[0] == ReOpenArchivedJobDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        // Grab the job ID
        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";

        const auto job = ctx.queue->GetJobByID(strID, true);
        if (!job)
        {
            event.reply(dpp::message("This job was not found in the archived requests. It may have been reopened.")
                .set_flags(dpp::m_ephemeral));
            return;
        }

        ctx.queue->ReOpenArchivedJob(job->GetID());

        const auto reopenedJob = ctx.queue->GetJobByID(strID);
        if (!reopenedJob)
        {
            event.reply(dpp::message("This job was not found in the queue after reopening.")
                .set_flags(dpp::m_ephemeral));
            return;
        }

        // Acknowledge immediately
        event.reply(dpp::message("The job request has been reopened.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_info, fmt::format("{} reopened job request {}.", author.global_name, strID));

        // Notify original customer if needed
        const dpp::snowflake customer = reopenedJob->GetCustomerID();
        if ((reopenedJob->IsCustomerSubscribed() && event.command.usr.id != reopenedJob->GetCustomerID()) || ctx.debug)
        {
            utils::NotifyIssuerMsgWithEmbed(
                ctx.cluster,
                event,
                customer,
                fmt::format("{} has reopened your request {}.", strID, fmt::format("<@{}>", author.id)),
                "Request Details",
                reopenedJob->PrintJobDetails(ctx.cluster, event.command.guild_id)
            );
        }

        GuildSettings::AnnounceOnNew(ctx, reopenedJob, true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Handles interactive button clicks for modified job requests.
///
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Button click interaction event.
//---------------------------------------------------------------------------------------------------------------------
void ModifyRequestCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
    const std::string id = event.custom_id; // "modify_type:workerid:rID"

    if (id.starts_with(fmt::format("{}_refresh:", this->name)))
    {
        auto parts = utils::Split(id, ':');
        dpp::snowflake user = parts[1];
        const std::string rID = parts[2];
        const auto job = ctx.queue->GetJobByID(rID);

        // Edit the original message
        event.reply(dpp::ir_update_message, SendPanel(ctx, event, job, user).set_flags(dpp::m_ephemeral));
        return;
    }
    else if (id.starts_with(fmt::format("{}_complete:", this->name)))
    {
        WorkerPanel::CompleteButton(id, ctx, event);

        auto parts = utils::Split(id, ':');
        dpp::snowflake user = parts[1];
        const std::string rID = parts[2];
        const auto job = ctx.queue->GetJobByID(rID);

        // Edit the original message
        event.reply(dpp::ir_update_message, SendPanel(ctx, event, job, user).set_flags(dpp::m_ephemeral));
        return;
    }
    else  if (id.starts_with(fmt::format("{}_edit:", this->name)))
    {
        GeneralUserPanel::EditButton(id, ctx, event);
        return;
    }
    //-------------------------------------------------------------------------------------------------------------
    // Add note handler
    //-------------------------------------------------------------------------------------------------------------
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
        const std::string rID = parts[2];
        const auto job = ctx.queue->GetJobByID(rID);

        // Edit the original message
        event.reply(dpp::ir_update_message, SendPanel(ctx, event, job, user).set_flags(dpp::m_ephemeral));
        return;
    }
    //-------------------------------------------------------------------------------------------------------------
    // Subscribe / Unsubscribe handler
    //-------------------------------------------------------------------------------------------------------------
    else if (id.starts_with(fmt::format("{}_subscribe:", this->name)))
    {
        CustomerPanel::SubscribeButton(id, ctx, event);

        auto parts = utils::Split(id, ':');
        dpp::snowflake user = parts[1];
        const std::string rID = parts[2];
        const auto job = ctx.queue->GetJobByID(rID);

        // Edit the original message
        event.reply(dpp::ir_update_message, SendPanel(ctx, event, job, user).set_flags(dpp::m_ephemeral));
        return;
    }
    //-------------------------------------------------------------------------------------------------------------
    // Delete handler
    //-------------------------------------------------------------------------------------------------------------
    else  if (id.starts_with(fmt::format("{}_delete:", this->name)))
    {
        GeneralUserPanel::DeleteButton(id, ctx, event);
        return;
    }
    else if (id.starts_with(fmt::format("{}_allnotes:", this->name)))
    {
        GeneralUserPanel::ShowNotesButton(id, ctx, event);
        return;
    }
    else if (id.starts_with("global_assign:"))
    {
        if (GlobalButtonPanel::AssignButton(id, ctx, event))
        {
            auto parts = utils::Split(id, ':');
            const std::string rID = parts[1];
            const auto job = ctx.queue->GetJobByID(rID);
            const bool bReopenedJob = parts[2] == "true" ? true : false;

            if (!job) return;
            GlobalButtonPanel panel{ ToString(job->GetID()), bReopenedJob };
            panel.AddEmbed(bReopenedJob ? "Reopened Job Request" : "New Job Request", job->PrintJobDetails(ctx.cluster, event.command.guild_id));

            event.reply(dpp::ir_update_message, panel);
        }
    }
    else if (id.starts_with("global_unassign:"))
    {
        if (GlobalButtonPanel::UnassignButton(id, ctx, event))
        {
            auto parts = utils::Split(id, ':');
            const std::string rID = parts[1];
            const auto job = ctx.queue->GetJobByID(rID);
            const bool bReopenedJob = parts[2] == "true" ? true : false;

            if (!job) return;
            GlobalButtonPanel panel{ ToString(job->GetID()), bReopenedJob };
            panel.AddEmbed(bReopenedJob ? "Reopened Job Request" : "New Job Request", job->PrintJobDetails(ctx.cluster, event.command.guild_id));

            event.reply(dpp::ir_update_message, panel);
        }
    }
    else if (id.starts_with("global_addnotes:"))
    {
        GlobalButtonPanel::AddNoteButton(id, ctx, event);
        return;
    }
    else if (id.starts_with("global_shownotes:"))
    {
        GlobalButtonPanel::ShowNotesButton(id, ctx, event);
        return;
    }
}

dpp::message ModifyRequestCommand::SendPanel(CommandContext& ctx, const dpp::interaction_create_t& event, const std::shared_ptr<const JobRequest>& job, const dpp::snowflake& user) const
{
    if (user == job->GetCustomerID() && !job->IsWorker(user))
    {
        CustomerPanel panel(this->name, user, ToString(job->GetID()), job->IsCustomerSubscribed());
        panel.AddEmbed("You Edited the Request", job->PrintJobDetails(ctx.cluster, event.command.guild_id));
        return panel;
    }
    else
    {
        WorkerPanel panel(this->name, user, ToString(job->GetID()), job->IsWorker(user));
        panel.AddEmbed("You Edited the Request", job->PrintJobDetails(ctx.cluster, event.command.guild_id));
        return panel;
    }
}