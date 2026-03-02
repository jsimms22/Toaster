//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief Implementation of CreateRequestCommand.
///
/// Handles creation of new job requests through slash commands, modal dialogs, and interactive button components.
///
/// Responsibilities:
/// - Display appropriate modal dialogs based on request type
/// - Process modal submissions and create job objects
/// - Add jobs to the queue
/// - Provide interactive buttons for editing, subscribing, and deleting jobs
//---------------------------------------------------------------------------------------------------------------------
#include "Commands.h"
#include "JobQueue.h"
#include "BotUtility.h"

#include "RequestDlg.h"
#include "CraftingJobRequest.h"
#include "BuildingJobRequest.h"
#include "ComponentJobRequest.h"
#include "ResourceJobRequest.h"
#include "RefineryJobRequest.h"
#include "HazardousRequest.h"

#include "WorkerPanel.h"
#include "CustomerPanel.h"
// fmt
#include <fmt/format.h>

//---------------------------------------------------------------------------------------------------------------------
/// \brief Handles slash command interaction to open the appropriate request dialog.
///
/// Validates the command name and displays the corresponding modal dialog based on the selected request type.
///
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Interaction create event.
//---------------------------------------------------------------------------------------------------------------------
void CreateRequestCommand::ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Type));

    const auto pManager = PermissionsMgr::GetInstance();
    if (!pManager->CanCreateJob(event, author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) && !ctx.debug)
    {
        // Permission denied
        event.reply(dpp::message("You do not have permissions to create requests.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("{} attempted to create a request with command {}.", author.global_name, strCmdID));
        return;
    }

    if (strCmdID == Option_ItemCrafting)
    {
        CraftRequestDlg modal;
        event.dialog(modal);
    }
    else if (strCmdID == Option_ComponentRequest)
    {
        ComponentRequestDlg modal;
        event.dialog(modal);
    }
    else if (strCmdID == Option_BaseBuidling)
    {
        BuildRequestDlg modal;
        event.dialog(modal);
    }
    else if (strCmdID == Option_ResourceCollect)
    {
        ResourceRequestDlg modal;
        event.dialog(modal);
    }
    else if (strCmdID == Option_RefineryJob)
    {
        RefineryRequestDlg modal;
        event.dialog(modal);
    }
    else if (strCmdID == Option_HazardousJob)
    {
        HazardousRequestDlg modal;
        event.dialog(modal);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Processes modal form submissions and creates job requests.
///
/// Extracts user input from modal components, sanitizes parameters, constructs the corresponding JobRequest 
/// object, and adds it to the queue. An ephemeral confirmation message with interactive controls is returned.
///
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Modal form submission event.
//---------------------------------------------------------------------------------------------------------------------
void CreateRequestCommand::ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event)
{
    if (!ctx.queue || 
        !(event.custom_id == CraftRequestDlg::modalID ||
          event.custom_id == BuildRequestDlg::modalID ||
          event.custom_id == ComponentRequestDlg::modalID ||
          event.custom_id == ResourceRequestDlg::modalID ||
          event.custom_id == RefineryRequestDlg::modalID ||
          event.custom_id == HazardousRequestDlg::modalID))
    {
        return;
    }
    
    const dpp::user author = event.command.get_issuing_user();
    std::string strSCHandle = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";

    // These parameters are input that depend on the dialog form's order as defined by the type of the job
    std::string strParam1 = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
    std::string strParam2 = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";
    std::string strParam3 = event.components.size() > 3 ? std::get<std::string>(event.components[3].value) : "";
    std::string strParam4 = event.components.size() > 4 ? std::get<std::string>(event.components[4].value) : "";

    // Acknowledge immediately to avoid timing out
    event.reply(dpp::message("...this could take a second. Please hold.").set_flags(dpp::m_ephemeral));

    utils::FilterUserString(strSCHandle);
    utils::FilterUserString(strParam1);
    utils::FilterUserString(strParam2);
    utils::FilterUserString(strParam3);
    utils::FilterUserString(strParam4);

    std::string jobDetails;
    GUID jobID;
    //-------------------------------------------------------------------------------------------------------------
    // Construct job object based on modal type
    //-------------------------------------------------------------------------------------------------------------
    if (event.custom_id == CraftRequestDlg::modalID)
    {
        std::shared_ptr<CraftingJobRequest> jobCraft = std::make_shared<CraftingJobRequest>();
        jobID = jobCraft->GetID();
        jobCraft->SetCustomerID(author.id);
        jobCraft->SetSCHandle(strSCHandle);
        jobCraft->SetItemDesc(strParam1);
        jobCraft->SetQuantity(strParam2);
        jobCraft->SetQualityThres(strParam3);
        jobCraft->SetPriority(JobRequest::StringToPriority(strParam4));
        jobDetails = jobCraft->PrintJobDetails(ctx.cluster, event.command.guild_id);
        ctx.cluster.log(dpp::ll_info, fmt::format("'{}' added new CRAFTING request '{}'.", author.global_name, utils::GuidToString(jobID)));
        ctx.queue->RequestAddToQueue(std::move(jobCraft));
    }
    else if (event.custom_id == BuildRequestDlg::modalID)
    {
        std::shared_ptr<BuildingJobRequest> jobBuild = std::make_shared<BuildingJobRequest>();
        jobID = jobBuild->GetID();
        jobBuild->SetCustomerID(author.id);
        jobBuild->SetSCHandle(strSCHandle);
        jobBuild->SetBuildDesign(strParam1);
        jobBuild->SetBuildRequirments(strParam2);
        jobBuild->SetBuildZone(strParam3);
        jobBuild->SetPriority(JobRequest::StringToPriority(strParam4));
        jobDetails = jobBuild->PrintJobDetails(ctx.cluster, event.command.guild_id);
        ctx.cluster.log(dpp::ll_info, fmt::format("'{}' added new BASE BUILDING request '{}'.", author.id, utils::GuidToStringNoBrackets(jobID)));
        ctx.queue->RequestAddToQueue(std::move(jobBuild));
    }
    else if (event.custom_id == ComponentRequestDlg::modalID)
    {
        std::shared_ptr<ComponentJobRequest> jobComp = std::make_shared<ComponentJobRequest>();
        jobID = jobComp->GetID();
        jobComp->SetCustomerID(author.id);
        jobComp->SetSCHandle(strSCHandle);
        jobComp->SetComponentList(strParam1);
        jobComp->SetPriority(JobRequest::StringToPriority(strParam2));
        jobDetails = jobComp->PrintJobDetails(ctx.cluster, event.command.guild_id);
        ctx.cluster.log(dpp::ll_info, fmt::format("USER '{}' added new COMPONENT request '{}'.", author.id, utils::GuidToStringNoBrackets(jobID)));
        ctx.queue->RequestAddToQueue(std::move(jobComp));
    }
    else if (event.custom_id == ResourceRequestDlg::modalID)
    {
        std::shared_ptr<ResourceJobRequest> jobRes = std::make_shared<ResourceJobRequest>();
        jobID = jobRes->GetID();
        jobRes->SetCustomerID(author.id);
        jobRes->SetSCHandle(strSCHandle);
        jobRes->SetResourceState(ResourceJobRequest::StringToState(strParam1));
        jobRes->SetResourcelist(strParam2);
        jobRes->SetQualityThres(strParam3);
        jobRes->SetPriority(JobRequest::StringToPriority(strParam4));
        jobDetails = jobRes->PrintJobDetails(ctx.cluster, event.command.guild_id);
        ctx.cluster.log(dpp::ll_info, fmt::format("USER '{}' added new RESOURCE request '{}'.", author.id, utils::GuidToStringNoBrackets(jobID)));
        ctx.queue->RequestAddToQueue(std::move(jobRes));
    }
    else if (event.custom_id == RefineryRequestDlg::modalID)
    {
        std::shared_ptr<RefineryJobRequest> jobRefine = std::make_shared<RefineryJobRequest>();
        jobID = jobRefine->GetID();
        jobRefine->SetCustomerID(author.id);
        jobRefine->SetSCHandle(strSCHandle);
        jobRefine->SetResourceState(RefineryJobRequest::StringToState(strParam1));
        jobRefine->SetResourcelist(strParam2);
        jobRefine->SetRefinery(strParam3);
        jobRefine->SetPriority(JobRequest::StringToPriority(strParam4));
        jobDetails = jobRefine->PrintJobDetails(ctx.cluster, event.command.guild_id);
        ctx.cluster.log(dpp::ll_info, fmt::format("USER '{}' added new REFINERY request '{}'.", author.id, utils::GuidToStringNoBrackets(jobID)));
        ctx.queue->RequestAddToQueue(std::move(jobRefine));
    }
    else if (event.custom_id == HazardousRequestDlg::modalID)
    {
        std::shared_ptr<HazardousRequest> jobHazard = std::make_shared<HazardousRequest>();
        jobID = jobHazard->GetID();
        jobHazard->SetCustomerID(author.id);
        jobHazard->SetSCHandle(strSCHandle);
        jobHazard->SetThreatLevel(HazardousRequest::StringToThreat(strParam1));
        jobHazard->SetItemLocation(strParam2);
        jobHazard->SetItemList(strParam3);
        jobHazard->SetPriority(JobRequest::StringToPriority(strParam4));
        jobDetails = jobHazard->PrintJobDetails(ctx.cluster, event.command.guild_id);
        ctx.cluster.log(dpp::ll_info, fmt::format("USER '{}' added new REFINERY request '{}'.", author.id, utils::GuidToStringNoBrackets(jobID)));
        ctx.queue->RequestAddToQueue(std::move(jobHazard));
    }

    std::shared_ptr<JobRequest> job = ctx.queue->GetJobByGUID(jobID);
    while (!job)
    {
        job = ctx.queue->GetJobByGUID(jobID);
    }

    // Edit the original message
    event.edit_original_response(SendPanel(ctx, event, job, author.id).set_flags(dpp::m_ephemeral));

    // Announce in channel (if set)
    GuildSettings::AnnounceOnNew(ctx, job->JobType(), job->PrintJobDetails(ctx.cluster, event.command.guild_id));
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Handles button click interactions for created job requests.
///
/// Supports the following actions:
/// - Edit job
/// - Add note (reserved)
/// - Subscribe / Unsubscribe
/// - Delete job
///
/// Validates permissions before executing any action and responds with ephemeral feedback if access is denied.
///
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Button click interaction event.
//---------------------------------------------------------------------------------------------------------------------
void CreateRequestCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
    const std::string id = event.custom_id;

    if (id.starts_with(fmt::format("{}_complete:", this->name)))
    {
        WorkerPanel::CompleteButton(id, ctx, event);

        Sleep(10);

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

        Sleep(10);
        return;
    }
    else if (id.starts_with(fmt::format("{}_note:", this->name)))
    {
        GeneralUserPanel::NoteButton(id, ctx, event);

        Sleep(10);
        return;
    }
    else if (id.starts_with(fmt::format("{}_unassign:", this->name)))
    {
        WorkerPanel::UnassignButton(id, ctx, event);

        Sleep(10);

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

        Sleep(10);

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

        Sleep(10);
        return;
    }
}

dpp::message CreateRequestCommand::SendPanel(CommandContext& ctx, const dpp::interaction_create_t& event, const std::shared_ptr<JobRequest>& job, const dpp::snowflake& user) const
{
    if (user == job->GetCustomerID() && user != job->GetWorkerID())
    {
        CustomerPanel panel(ctx, this->name, user, utils::GuidToStringNoBrackets(job->GetID()), job->IsCustomerSubscribed());
        panel.AddEmbed("You Submitted a New Request", job->PrintJobDetails(ctx.cluster, event.command.guild_id));
        return panel;
    }
    else
    {
        WorkerPanel panel(ctx, this->name, user, utils::GuidToStringNoBrackets(job->GetID()), job->GetWorkerID());
        panel.AddEmbed("You Submitted a New Request", job->PrintJobDetails(ctx.cluster, event.command.guild_id));
        return panel;
    }
}