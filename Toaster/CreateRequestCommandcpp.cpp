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

    ctx.cluster.log(dpp::ll_info, fmt::format("{} used command {} with cmd option {}", author.username, event.command.get_command_name(), strCmdID));
}

void CreateRequestCommand::ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event)
{
    if (!ctx.queue || 
        event.custom_id != CraftRequestDlg::modalID ||
        event.custom_id != BuildRequestDlg::modalID ||
        event.custom_id != ComponentRequestDlg::modalID ||
        event.custom_id != ResourceRequestDlg::modalID ||
        event.custom_id != RefineryRequestDlg::modalID)
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
    if (event.custom_id == CraftRequestDlg::modalID)
    {
        std::shared_ptr<CraftingJobRequest> jobCraft = std::make_shared<CraftingJobRequest>();
        jobCraft->SetCustomerID(author.id);
        jobCraft->SetSCHandle(strSCHandle);
        jobCraft->SetItemDesc(strParam1);
        jobCraft->SetQuantity(strParam2);
        jobCraft->SetQualityThres(strParam3);
        jobCraft->SetPriority(JobRequest::StringToPriority(strParam4));
        jobDetails = jobCraft->PrintJobDetails(ctx.cluster);
        ctx.cluster.log(dpp::ll_info, fmt::format("{} added new request {}.", author.username, utils::GuidToString(jobCraft->GetID())));
        ctx.queue->AddToQueue(std::move(jobCraft));
    }
    else if (event.custom_id == BuildRequestDlg::modalID)
    {
        std::shared_ptr<BuildingJobRequest> jobBuild = std::make_shared<BuildingJobRequest>();
        jobBuild->SetCustomerID(author.id);
        jobBuild->SetSCHandle(strSCHandle);
        jobBuild->SetBuildDesign(strParam1);
        jobBuild->SetBuildRequirments(strParam2);
        jobBuild->SetBuildZone(strParam3);
        jobBuild->SetPriority(JobRequest::StringToPriority(strParam4));
        jobDetails = jobBuild->PrintJobDetails(ctx.cluster);
        ctx.cluster.log(dpp::ll_info, fmt::format("{} added new request {}.", author.username, utils::GuidToString(jobBuild->GetID())));
        ctx.queue->AddToQueue(std::move(jobBuild));
    }
    else if (event.custom_id == ComponentRequestDlg::modalID)
    {
        std::shared_ptr<ComponentJobRequest> jobComp = std::make_shared<ComponentJobRequest>();
        jobComp->SetCustomerID(author.id);
        jobComp->SetSCHandle(strSCHandle);
        jobComp->SetComponentList(strParam1);
        jobComp->SetPriority(JobRequest::StringToPriority(strParam2));
        jobDetails = jobComp->PrintJobDetails(ctx.cluster);
        ctx.cluster.log(dpp::ll_info, fmt::format("{} added new request {}.", author.username, utils::GuidToString(jobComp->GetID())));
        ctx.queue->AddToQueue(std::move(jobComp));
    }
    else if (event.custom_id == ResourceRequestDlg::modalID)
    {
        std::shared_ptr<ResourceJobRequest> jobRes = std::make_shared<ResourceJobRequest>();
        jobRes->SetCustomerID(author.id);
        jobRes->SetSCHandle(strSCHandle);
        jobRes->SetResourceState(ResourceJobRequest::StringToState(strParam1));
        jobRes->SetResourcelist(strParam2);
        jobRes->SetQualityThres(strParam3);
        jobRes->SetPriority(JobRequest::StringToPriority(strParam4));
        jobDetails = jobRes->PrintJobDetails(ctx.cluster);
        ctx.cluster.log(dpp::ll_info, fmt::format("{} added new request {}.", author.username, utils::GuidToString(jobRes->GetID())));
        ctx.queue->AddToQueue(std::move(jobRes));
    }
    else if (event.custom_id == RefineryRequestDlg::modalID)
    {
        std::shared_ptr<RefineryJobRequest> jobRefine = std::make_shared<RefineryJobRequest>();
        jobRefine->SetCustomerID(author.id);
        jobRefine->SetSCHandle(strSCHandle);
        jobRefine->SetResourceState(RefineryJobRequest::StringToState(strParam1));
        jobRefine->SetResourcelist(strParam2);
        jobRefine->SetRefinery(strParam3);
        jobRefine->SetPriority(JobRequest::StringToPriority(strParam4));
        jobDetails = jobRefine->PrintJobDetails(ctx.cluster);
        ctx.cluster.log(dpp::ll_info, fmt::format("{} added new request {}.", author.username, utils::GuidToString(jobRefine->GetID())));
        ctx.queue->AddToQueue(std::move(jobRefine));
    }

    dpp::embed embed;
    embed.set_title("Submitted job request:")
        .set_description(jobDetails)
        .set_color(0x3498db);

    event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
}