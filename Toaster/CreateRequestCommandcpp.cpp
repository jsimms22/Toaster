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
#include "GuildSettings.h"
#include "PermissionsMgr.h"

#include "RequestDlg.h"
#include "RequestID.h"
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
    dpp::guild* guild = utils::FindGuildByID(ctx.cluster, event.command.guild_id);
    if (!pManager->CanCreateJob(event, author.id, guild, ctx.guild) && !ctx.debug)
    {
        // Permission denied
        event.reply(dpp::message("You do not have permissions to create requests.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("{} attempted to create a request with command {}.", author.global_name, strCmdID));
        return;
    }
    
    // Eventually this will be dynamic
    if (!pManager->CanAccessAdminPanel(event, author.id, guild, ctx.guild) &&
        ctx.guild->requestLimitPerUser <= ctx.queue->GetQueueSize(true, [user = author.id](const std::shared_ptr<const JobRequest> job) -> bool { return job->GetCustomerID() == user && job->GetStatus() < JobRequest::status::hold; }))
    {
        // Too many requests in queue
        event.reply(dpp::message("You currently have too many requests in queue. Current capacity is 15 open per user. Completed or On Hold does not influence this cap.").set_flags(dpp::m_ephemeral));
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
    const std::string id = event.custom_id;
    auto parts = utils::Split(id, ':');

    if (!ctx.queue || 
        !(parts[0] == CraftRequestDlg::modalID ||
          parts[0] == BuildRequestDlg::modalID ||
          parts[0] == ComponentRequestDlg::modalID ||
          parts[0] == ResourceRequestDlg::modalID ||
          parts[0] == RefineryRequestDlg::modalID ||
          parts[0] == HazardousRequestDlg::modalID))
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
    RequestID jobID;
    //-------------------------------------------------------------------------------------------------------------
    // Construct job object based on modal type
    //-------------------------------------------------------------------------------------------------------------
    if (parts[0] == CraftRequestDlg::modalID)
    {
        std::shared_ptr<CraftingJobRequest> jobCraft = std::make_shared<CraftingJobRequest>(event.command.guild_id, author.id);
        jobID = IDGenerator::Generate(jobCraft->JobType(), author.id, ctx.guild);
        jobCraft->SetID(jobID);
        jobCraft->SetSCHandle(strSCHandle);
        jobCraft->SetItemDesc(strParam1);
        jobCraft->SetQuantity(strParam2);
        jobCraft->SetQualityThres(strParam3);
        jobCraft->SetPriority(JobRequest::StringToPriority(strParam4));
        jobDetails = jobCraft->PrintJobDetails(ctx.cluster, event.command.guild_id);
        ctx.cluster.log(dpp::ll_info, fmt::format("'{}' added new CRAFTING request '{}'.", author.global_name, ToString(jobID)));
        ctx.queue->RequestAdd(std::move(jobCraft));
    }
    else if (parts[0] == BuildRequestDlg::modalID)
    {
        std::shared_ptr<BuildingJobRequest> jobBuild = std::make_shared<BuildingJobRequest>(event.command.guild_id, author.id);
        jobID = IDGenerator::Generate(jobBuild->JobType(), author.id, ctx.guild);
        jobBuild->SetID(jobID);
        jobBuild->SetSCHandle(strSCHandle);
        jobBuild->SetBuildDesign(strParam1);
        jobBuild->SetBuildRequirments(strParam2);
        jobBuild->SetBuildZone(strParam3);
        jobBuild->SetPriority(JobRequest::StringToPriority(strParam4));
        jobDetails = jobBuild->PrintJobDetails(ctx.cluster, event.command.guild_id);
        ctx.cluster.log(dpp::ll_info, fmt::format("'{}' added new BASE BUILDING request '{}'.", author.id, ToString(jobID)));
        ctx.queue->RequestAdd(std::move(jobBuild));
    }
    else if (parts[0] == ComponentRequestDlg::modalID)
    {
        std::shared_ptr<ComponentJobRequest> jobComp = std::make_shared<ComponentJobRequest>(event.command.guild_id, author.id);
        jobID = IDGenerator::Generate(jobComp->JobType(), author.id, ctx.guild);
        jobComp->SetID(jobID);
        jobComp->SetSCHandle(strSCHandle);
        jobComp->SetComponentList(strParam1);
        jobComp->SetPriority(JobRequest::StringToPriority(strParam2));
        jobDetails = jobComp->PrintJobDetails(ctx.cluster, event.command.guild_id);
        ctx.cluster.log(dpp::ll_info, fmt::format("USER '{}' added new COMPONENT request '{}'.", author.id, ToString(jobID)));
        ctx.queue->RequestAdd(std::move(jobComp));
    }
    else if (parts[0] == ResourceRequestDlg::modalID)
    {
        std::shared_ptr<ResourceJobRequest> jobRes = std::make_shared<ResourceJobRequest>(event.command.guild_id, author.id);
        jobID = IDGenerator::Generate(jobRes->JobType(), author.id, ctx.guild);
        jobRes->SetID(jobID);
        jobRes->SetSCHandle(strSCHandle);
        jobRes->SetResourceState(ResourceJobRequest::StringToState(strParam1));
        jobRes->SetResourcelist(strParam2);
        jobRes->SetQualityThres(strParam3);
        jobRes->SetPriority(JobRequest::StringToPriority(strParam4));
        jobDetails = jobRes->PrintJobDetails(ctx.cluster, event.command.guild_id);
        ctx.cluster.log(dpp::ll_info, fmt::format("USER '{}' added new RESOURCE request '{}'.", author.id, ToString(jobID)));
        ctx.queue->RequestAdd(std::move(jobRes));
    }
    else if (parts[0] == RefineryRequestDlg::modalID)
    {
        std::shared_ptr<RefineryJobRequest> jobRefine = std::make_shared<RefineryJobRequest>(event.command.guild_id, author.id);
        jobID = IDGenerator::Generate(jobRefine->JobType(), author.id, ctx.guild);
        jobRefine->SetID(jobID);
        jobRefine->SetSCHandle(strSCHandle);
        jobRefine->SetResourceState(RefineryJobRequest::StringToState(strParam1));
        jobRefine->SetResourcelist(strParam2);
        jobRefine->SetRefinery(strParam3);
        jobRefine->SetPriority(JobRequest::StringToPriority(strParam4));
        jobDetails = jobRefine->PrintJobDetails(ctx.cluster, event.command.guild_id);
        ctx.cluster.log(dpp::ll_info, fmt::format("USER '{}' added new REFINERY request '{}'.", author.id, ToString(jobID)));
        ctx.queue->RequestAdd(std::move(jobRefine));
    }
    else if (parts[0] == HazardousRequestDlg::modalID)
    {
        std::shared_ptr<HazardousRequest> jobHazard = std::make_shared<HazardousRequest>(event.command.guild_id, author.id);
        jobID = IDGenerator::Generate(jobHazard->JobType(), author.id, ctx.guild);
        jobHazard->SetID(jobID);
        jobHazard->SetSCHandle(strSCHandle);
        jobHazard->SetThreatLevel(HazardousRequest::StringToThreat(strParam1));
        jobHazard->SetItemLocation(strParam2);
        jobHazard->SetItemList(strParam3);
        jobHazard->SetPriority(JobRequest::StringToPriority(strParam4));
        jobDetails = jobHazard->PrintJobDetails(ctx.cluster, event.command.guild_id);
        ctx.cluster.log(dpp::ll_info, fmt::format("USER '{}' added new REFINERY request '{}'.", author.id, ToString(jobID)));
        ctx.queue->RequestAdd(std::move(jobHazard));
    }
    else
    {
        event.reply(dpp::message("Unknown job request type.").set_flags(dpp::m_ephemeral));
    }

    const auto job = ctx.queue->GetJobByID(jobID);

    ctx.guild->SaveGuildSettings(ctx.repo);

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
        const std::string rID = parts[2];
        const auto job = ctx.queue->GetJobByID(rID);

        // Edit the original message
        event.reply(dpp::ir_update_message, SendPanel(ctx, event, job, user).set_flags(dpp::m_ephemeral));
        return;
    }
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
    else if (id.starts_with(fmt::format("{}_delete:", this->name)))
    {
        GeneralUserPanel::DeleteButton(id, ctx, event);
        return;
    }
    else if (id.starts_with(fmt::format("{}_allnotes:", this->name)))
    {
        GeneralUserPanel::ShowNotesButton(id, ctx, event);
        return;
    }
}

dpp::message CreateRequestCommand::SendPanel(CommandContext& ctx, const dpp::interaction_create_t& event, const std::shared_ptr<const JobRequest>& job, const dpp::snowflake& user) const
{
    if (user == job->GetCustomerID() && !job->IsWorker(user))
    {
        CustomerPanel panel(this->name, user, ToString(job->GetID()), job->IsCustomerSubscribed());
        panel.AddEmbed("You Submitted a New Request", job->PrintJobDetails(ctx.cluster, event.command.guild_id));
        return panel;
    }
    else
    {
        WorkerPanel panel(this->name, user, ToString(job->GetID()), user);
        panel.AddEmbed("You Submitted a New Request", job->PrintJobDetails(ctx.cluster, event.command.guild_id));
        return panel;
    }
}