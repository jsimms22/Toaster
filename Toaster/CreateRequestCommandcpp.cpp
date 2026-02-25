#include "Commands.h"
#include "JobQueue.h"
#include "BotUtility.h"

#include "RequestDlg.h"
#include "CraftingJobRequest.h"
#include "BuildingJobRequest.h"
#include "ComponentJobRequest.h"
#include "ResourceJobRequest.h"
#include "RefineryJobRequest.h"
// fmt
#include <fmt/format.h>

void CreateRequestCommand::ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event)
{
}

void CreateRequestCommand::ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Type));

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
}

void CreateRequestCommand::ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event)
{
    if (!ctx.queue || 
        !(event.custom_id == CraftRequestDlg::modalID ||
        event.custom_id == BuildRequestDlg::modalID ||
        event.custom_id == ComponentRequestDlg::modalID ||
        event.custom_id == ResourceRequestDlg::modalID ||
        event.custom_id == RefineryRequestDlg::modalID))
    {
        return;
    }
    
    const dpp::user author = event.command.get_issuing_user();
    const std::string strSCHandle = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";

    // These parameters are input that depend on the dialog form's order as defined by the type of the job
    const std::string strParam1 = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
    const std::string strParam2 = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";
    const std::string strParam3 = event.components.size() > 3 ? std::get<std::string>(event.components[3].value) : "";
    const std::string strParam4 = event.components.size() > 4 ? std::get<std::string>(event.components[4].value) : "";

    std::string jobDetails;
    GUID jobID;
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
        ctx.queue->AddToQueue(std::move(jobCraft));
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
        ctx.cluster.log(dpp::ll_info, fmt::format("'{}' added new BASE BUILDING request '{}'.", author.id, utils::GuidToString(jobID)));
        ctx.queue->AddToQueue(std::move(jobBuild));
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
        ctx.cluster.log(dpp::ll_info, fmt::format("USER '{}' added new COMPONENT request '{}'.", author.id, utils::GuidToString(jobID)));
        ctx.queue->AddToQueue(std::move(jobComp));
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
        ctx.cluster.log(dpp::ll_info, fmt::format("USER '{}' added new RESOURCE request '{}'.", author.id, utils::GuidToString(jobID)));
        ctx.queue->AddToQueue(std::move(jobRes));
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
        ctx.cluster.log(dpp::ll_info, fmt::format("USER '{}' added new REFINERY request '{}'.", author.id, utils::GuidToString(jobID)));
        ctx.queue->AddToQueue(std::move(jobRefine));
    }

    dpp::component button1 = dpp::component()
        .set_type(dpp::cot_button)
        .set_label("Edit")
        .set_style(dpp::cos_primary)
        .set_id(fmt::format("create_edit:{}:{}", author.id, utils::GuidToStringNoBrackets(jobID)));

    dpp::component button2 = dpp::component()
        .set_type(dpp::cot_button)
        .set_label("Add Note")
        .set_style(dpp::cos_primary)
        .set_id(fmt::format("create_note:{}:{}", author.id, utils::GuidToStringNoBrackets(jobID)));

    dpp::component button3 = dpp::component()
        .set_type(dpp::cot_button)
        .set_label("Unsubscribed")
        .set_style(dpp::cos_secondary)
        .set_id(fmt::format("create_subscribe:{}:{}", author.id, utils::GuidToStringNoBrackets(jobID)));

    dpp::component button4 = dpp::component()
        .set_type(dpp::cot_button)
        .set_label("Delete")
        .set_style(dpp::cos_danger)
        .set_id(fmt::format("create_delete:{}:{}", author.id, utils::GuidToStringNoBrackets(jobID)));

    dpp::component row = dpp::component()
        .set_type(dpp::cot_action_row)
        .add_component(button1)
        .add_component(button2)
        .add_component(button3)
        .add_component(button4);

    dpp::embed embed;
    embed.set_title("Submitted job request:")
        .set_description(jobDetails)
        .set_color(0x3498db);

    // todo announce in a specific channel

    event.reply(dpp::message().add_embed(embed).add_component(row).set_flags(dpp::m_ephemeral));
}

void CreateRequestCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
    const std::string id = event.custom_id; // "create_type:workerid:guid"

    if (id.starts_with("create_edit:"))
    {
        auto parts = utils::Split(id, ':');
        const dpp::snowflake user = parts[1];
        const std::string guid = parts[2];

        auto job = ctx.queue->GetJobByGUID(guid);
        if (job && ctx.manager->CanEditJob(user, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id)))
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
    else if (id.starts_with("create_note:"))
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
    else if (id.starts_with("create_subscribe:"))
    {
        auto parts = utils::Split(id, ':');
        const dpp::snowflake user = parts[1];
        const std::string guid = parts[2];

        auto job = ctx.queue->GetJobByGUID(guid);
        if (job && ctx.manager->IsRequestOwner(user, job))
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
                .set_id(fmt::format("create_edit:{}:{}", author.id, strJobID));

            dpp::component button2 = dpp::component()
                .set_type(dpp::cot_button)
                .set_label("Add Note")
                .set_style(dpp::cos_primary)
                .set_id(fmt::format("create_note:{}:{}", author.id, strJobID));

            dpp::component button3 = dpp::component()
                .set_type(dpp::cot_button)
                .set_label(job->IsCustomerSubscribed() ? "Subscribed" : "Unsubscribed")
                .set_style(job->IsCustomerSubscribed() ? dpp::cos_primary : dpp::cos_secondary)
                .set_id(fmt::format("create_subscribe:{}:{}", author.id, strJobID));

            dpp::component button4 = dpp::component()
                .set_type(dpp::cot_button)
                .set_label("Delete")
                .set_style(dpp::cos_danger)
                .set_id(fmt::format("create_delete:{}:{}", author.id, strJobID));

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

            ctx.cluster.log(dpp::ll_info,fmt::format("User changed subscribed status for {} to {}", strJobID,
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
    else if (id.starts_with("create_delete:"))
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