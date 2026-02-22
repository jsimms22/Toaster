#include "Toaster.h"

#include "BotUtility.h"
#include "CommandContext.h"
#include "Commands.h"
#include "JobQueue.h"
#include "Resource.h"

#include "JobRequestFactory.h"
#include "RequestDlg.h"
#include "CraftingJobRequest.h"
#include "BuildingJobRequest.h"
#include "ComponentJobRequest.h"
#include "ResourceJobRequest.h"
// d++
#include <dpp/channel.h>
#include <dpp/dpp.h>
#include <dpp/snowflake.h>
// fmt
#include <fmt/base.h>
#include <fmt/format.h>
// microsoft
#include <guiddef.h>
// std library
#include <unordered_map>

namespace
{
    std::size_t CmdStringToJobType(const std::string& cmd)
    {
        if (cmd == Option_ItemCrafting) return JOB_TYPE_CRAFTING;
        else if (cmd == Option_BaseBuidling) return JOB_TYPE_BUILDING;
        else if (cmd == Option_ComponentRequest) return JOB_TYPE_COMPONENT;
        else if (cmd == Option_ResourceCollect) return JOB_TYPE_RESOURCE;
        else if (cmd == Option_RefineryJob) return JOB_TYPE_REFINERY;
        else return JOB_TYPE_GENERAL;
    }
}

ToasterBot::ToasterBot(dpp::cluster& cluster, const uint32_t clusterId, const std::shared_ptr<JobQueue>& spQueue, const bool bDebug)
    : m_cluster(cluster), m_clusterId(clusterId), m_spQueue(spQueue), m_debug(bDebug), m_iShardCount{ 0 } { }

void ToasterBot::onReady(const dpp::ready_t& event)
{
    if (dpp::run_once<struct clear_bot_commands>()) 
    {
        //ICustomCommand::UnregisterAll(m_cluster);
        //ICustomCommand::UnregisterGuildAll(&m_cluster, 1472034166869852287);
    }
    if (dpp::run_once<struct register_bot_commands>())
    {
        //ICustomCommand::UnregisterAll(m_cluster);
        ICustomCommand::RegisterGuildAll(&m_cluster, 1472034166869852287, Toaster::BotCommands);
    }
}

void ToasterBot::onMessage(const dpp::message_create_t& event)
{
    if (!event.msg.author.id) {
        m_cluster.log(dpp::ll_error, fmt::format("Message dropped, no author: {}.", event.msg.content));
        return;
    } 
    else if (event.msg.author.id == m_cluster.me.id) 
    {
        dpp::channel* channel = dpp::find_channel(event.msg.channel_id);
        if (channel && channel->get_type() == dpp::channel_type::DM)
        {
            for (auto& idUser : channel->recipients)
            {
                dpp::user* user = dpp::find_user(idUser);
                if (user) 
                {
                    m_cluster.log(dpp::ll_info,
                        fmt::format("Bot sending outgoing direct message to {}#{}.", user->username, user->discriminator));
                }
            }
        }
        else if (channel && 
            (channel->get_type() == dpp::channel_type::CHANNEL_PUBLIC_THREAD || 
             channel->get_type() == dpp::channel_type::CHANNEL_PRIVATE_THREAD))
        {
            // Guild channel
            m_cluster.log(dpp::ll_info,
                fmt::format("Bot sending outgoing message in channel {} - {}.", event.msg.channel_id, channel->name));
        }
    }
}

void ToasterBot::onSlashCommand(const dpp::slashcommand_t& event)
{
    std::chrono::time_point<std::chrono::steady_clock> start;
    if (m_debug)
    {
        start = std::chrono::steady_clock::now();
    }

    CommandContext ctx{ m_cluster, m_spQueue, m_debug, m_vWorkers };
    for (auto cmd : Toaster::BotCommands)
    {
        cmd->ExecuteCommand(ctx, event);
    }

    if (m_debug)
    {
        auto end = std::chrono::steady_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        m_cluster.log(
            dpp::ll_debug,
            fmt::format(
                "Slash '{}' executed in {} ms (user: {})",
                event.command.get_command_name(),
                duration,
                event.command.get_issuing_user().username
            )
        );
    }

    m_cluster.log(dpp::ll_info, fmt::format("Toaster process slash command {} for user {}", event.command.get_command_name(), event.command.get_issuing_user().username));
}

void ToasterBot::onInteractionCreate(const dpp::interaction_create_t& event)
{
    std::chrono::time_point<std::chrono::steady_clock> start;
    if (m_debug)
    {
        start = std::chrono::steady_clock::now();
    }

    CommandContext ctx{ m_cluster, m_spQueue, m_debug, m_vWorkers };
    for (auto cmd : Toaster::BotCommands)
    {
        cmd->ExecuteInteraction(ctx, event);
    }

    if (m_debug)
    {
        auto end = std::chrono::steady_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        m_cluster.log(
            dpp::ll_debug,
            fmt::format(
                "Interaction '{}' executed in {} ms (user: {})",
                event.command.get_command_name(),
                duration,
                event.command.get_issuing_user().username
            )
        );
    }

    m_cluster.log(dpp::ll_info, fmt::format("Toaster process interaction command {} for user {}", event.command.get_command_name(), event.command.get_issuing_user().username));
}

void ToasterBot::onFormSubmit(const dpp::form_submit_t& event)
{
    if (event.custom_id == CraftRequestDlg::modalID || 
        event.custom_id == BuildRequestDlg::modalID ||
        event.custom_id == ComponentRequestDlg::modalID ||
        event.custom_id == ResourceRequestDlg::modalID ||
        event.custom_id == RefineryRequestDlg::modalID)
    {
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
            jobDetails = jobCraft->PrintJobDetails(m_cluster);
            m_cluster.log(dpp::ll_info, fmt::format("{} added new request {}.", author.username, utils::GuidToString(jobCraft->GetID())));
            m_spQueue->AddToQueue(std::move(jobCraft));
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
            jobDetails = jobBuild->PrintJobDetails(m_cluster);
            m_cluster.log(dpp::ll_info, fmt::format("{} added new request {}.", author.username, utils::GuidToString(jobBuild->GetID())));
            m_spQueue->AddToQueue(std::move(jobBuild));
        }
        else if (event.custom_id == ComponentRequestDlg::modalID)
        {
            std::shared_ptr<ComponentJobRequest> jobComp = std::make_shared<ComponentJobRequest>();
            jobComp->SetCustomerID(author.id);
            jobComp->SetSCHandle(strSCHandle);
            jobComp->SetComponentList(strParam1);
            jobComp->SetPriority(JobRequest::StringToPriority(strParam2));
            jobDetails = jobComp->PrintJobDetails(m_cluster);
            m_cluster.log(dpp::ll_info, fmt::format("{} added new request {}.", author.username, utils::GuidToString(jobComp->GetID())));
            m_spQueue->AddToQueue(std::move(jobComp));
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
            jobDetails = jobRes->PrintJobDetails(m_cluster);
            m_cluster.log(dpp::ll_info, fmt::format("{} added new request {}.", author.username, utils::GuidToString(jobRes->GetID())));
            m_spQueue->AddToQueue(std::move(jobRes));
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
            jobDetails = jobRefine->PrintJobDetails(m_cluster);
            m_cluster.log(dpp::ll_info, fmt::format("{} added new request {}.", author.username, utils::GuidToString(jobRefine->GetID())));
            m_spQueue->AddToQueue(std::move(jobRefine));
        }

        dpp::embed embed;
        embed.set_title("Submitted job request:")
            .set_description(jobDetails)
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
    }
    else if (event.custom_id == EditRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strSCHandle = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
        // These parameters are input that depend on the dialog form's order as defined by the type of the job
        const std::string strParam1 = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";
        const std::string strParam2 = event.components.size() > 3 ? std::get<std::string>(event.components[3].value) : "";
        const std::string strParam3 = event.components.size() > 4 ? std::get<std::string>(event.components[4].value) : "";

        std::shared_ptr<JobRequest> job = m_spQueue->GetJobByGUID(strID);
        if (!job || (author.id != job->GetCustomerID() && !m_debug/*|| todo get permissions list */))
        {
            m_cluster.log(dpp::ll_error, fmt::format("{} attempted to edit {}", author.username, strID));
            return;
        }
        job->SetLastEditTime(utils::GetEpochTimestamp());

        const std::string strOldJobDetails = job->PrintJobDetails(m_cluster);
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

        const std::string strNewJobDetails = job->PrintJobDetails(m_cluster);
        dpp::embed embed;
        embed.set_title("Edited job:")
            .set_description(strNewJobDetails)
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        m_spQueue->SaveQueueToFile();

        m_cluster.log(dpp::ll_info, fmt::format("{} edited {}.", author.username, strID));

        if ((author.id != job->GetCustomerID() && strOldJobDetails != strNewJobDetails) || m_debug)
        {
            utils::NotifyIssuerMsg(m_cluster, job->GetCustomerID(), event,
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

        std::shared_ptr<JobRequest> job = m_spQueue->GetJobByGUID(strID);
        if (!job || (author.id != job->GetCustomerID() && !m_debug/*|| todo get permissions list */))
        {
            m_cluster.log(dpp::ll_error, fmt::format("{} attempted to assign {} to a worker.", author.username, strID));
            return;
        }

        job->SetLastEditTime(utils::GetEpochTimestamp());
        job->SetWorkerID(workerID);
        job->SetStatus(CraftingJobRequest::StringToStatus(strStatus));

        dpp::embed embed;
        embed.set_title("Assigned job:")
            .set_description(job->PrintJobDetails(m_cluster))
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        m_spQueue->SaveQueueToFile();

        const std::string strWorker = job->GetWorkerName(m_cluster);
        m_cluster.log(dpp::ll_info, fmt::format("{} assigned {} to {}", author.username, strID, strWorker));

        if (author.id != job->GetCustomerID() || m_debug)
        {
            utils::NotifyIssuerMsg(m_cluster, job->GetCustomerID(), event,
                fmt::format("Your request has been assigned to {} by {}:\n\n{}", strWorker, author.username, embed.description));
        }
    }
    else if (event.custom_id == StatusChangeRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strStatus = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";

        std::shared_ptr<JobRequest> job = m_spQueue->GetJobByGUID(strID);
        if (!job || (author.id != job->GetCustomerID() && !m_debug/*|| todo get permissions list */))
        {
            m_cluster.log(dpp::ll_error, fmt::format("{} attempted to change the status for {} to {}", author.username, strID, strStatus));
            return;
        }
        job->SetLastEditTime(utils::GetEpochTimestamp());

        const std::string strOldStatus = JobRequest::StatusToString(job->GetStatus());
        job->SetStatus(CraftingJobRequest::StringToStatus(strStatus));

        dpp::embed embed;
        embed.set_title("Status changed for job:")
            .set_description(job->PrintJobDetails(m_cluster))
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        m_spQueue->SaveQueueToFile();

        m_cluster.log(dpp::ll_info, fmt::format("{} updated the status for {} to {}", author.username, strID, strStatus));
        if ((author.id != job->GetCustomerID() && strOldStatus != strStatus) || m_debug)
        {
            utils::NotifyIssuerMsg(m_cluster, job->GetCustomerID(), event,
                fmt::format("Your request's status has moved from {} to {} by {}:\n\n{}", strOldStatus, strStatus, author.username, job->PrintJobDetails(m_cluster)));
        }
    }
    else if (event.custom_id == PriorityChangeRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strPriority = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";

        std::shared_ptr<JobRequest> job = m_spQueue->GetJobByGUID(strID);
        if (!job || (author.id != job->GetCustomerID() && !m_debug/*|| todo get permissions list */))
        {
            m_cluster.log(dpp::ll_error, fmt::format("{} attempted to change priority for {} to {}", author.username, strID, strPriority));
            return;
        }
        job->SetLastEditTime(utils::GetEpochTimestamp());

        const std::string strOldPriority = JobRequest::PriorityToString(job->GetPriority());
        job->SetPriority(JobRequest::StringToPriority(strPriority));

        dpp::embed embed;
        embed.set_title("Priority changed for job:")
            .set_description(job->PrintJobDetails(m_cluster))
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        m_spQueue->SaveQueueToFile();

        m_cluster.log(dpp::ll_info, fmt::format("{} updated the priority for {} to {}", author.username, strID, strPriority));
        if ((author.id != job->GetCustomerID() && strOldPriority != strPriority) || m_debug)
        {
            utils::NotifyIssuerMsg(m_cluster, job->GetCustomerID(), event,
                fmt::format("Your request's priority has moved from {} to {} by {}:\n\n{}", strOldPriority, strPriority, author.username, job->PrintJobDetails(m_cluster)));
        }
    }
}

void ToasterBot::onButtonClick(const dpp::button_click_t& event)
{
    if (event.custom_id == Button_Complete)
    {
        m_cluster.log(dpp::ll_info, Button_Complete);
    }
    else if (event.custom_id == Button_Note)
    {
        m_cluster.log(dpp::ll_info, Button_Note);
    }
    else if (event.custom_id == Button_Unassign)
    {
        m_cluster.log(dpp::ll_info, Button_Unassign);
    }
    else if (event.custom_id == Button_Delete)
    {
        m_cluster.log(dpp::ll_info, Button_Delete);
    }

    event.reply(dpp::message("Currently prototyping buttons and interactions - this button does not do anything right not.").set_flags(dpp::m_ephemeral));
}