#include "Toaster.h"

#include "BotUtility.h"
#include "Command.h"
#include "JobQueue.h"
#include "Resource.h"

#include "RequestDlg.h"
#include "CraftingJobRequest.h"
#include "BuildingJobRequest.h"
#include "ComponentJobRequest.h"
#include "ResourceJobRequest.h"

#include <dpp/dpp.h>
// Microsoft
#include <guiddef.h>

ToasterBot::ToasterBot(dpp::cluster* cluster, const uint32_t clusterId, const std::shared_ptr<JobQueue>& spQueue, const bool bDebug)
    : m_cluster{ cluster }, m_clusterId{ clusterId }, m_spQueue{ spQueue }, m_debug{ bDebug }, m_iShardCount{ 0 }
{

}

void ToasterBot::onReady(const dpp::ready_t& event)
{
    //if (dpp::run_once<struct clear_bot_commands>()) 
    {
        //m_cluster->guild_bulk_command_delete(1472034166869852287);
        //m_cluster->global_bulk_command_delete();
    }
    if (dpp::run_once<struct register_bot_commands>())
    {
        std::vector<dpp::slashcommand> vCmd;
        for (std::pair<std::string, dpp::slashcommand>& cmd : BotModule::commands)
        {
            cmd.second.application_id = m_cluster->me.id;
            vCmd.push_back(cmd.second);
        }
        m_cluster->guild_bulk_command_create(vCmd, 1472034166869852287);
        //m_cluster->global_bulk_command_create({ pingcommand, pongcommand, dingcommand, dongcommand, modalcommand, emphemcommand });

        m_cluster->log(dpp::ll_debug, "Registered " + std::to_string(vCmd.size()) + " commands with guild id: " + std::to_string(1472034166869852287));
    }
}

void ToasterBot::onMessage(const dpp::message_create_t& event)
{
    if (!event.msg.author.id) {
        m_cluster->log(dpp::ll_info, "Message dropped, no author: {}" + event.msg.content);
        return;
    } 
    else if (event.msg.author.id == m_cluster->me.id) 
    {
        m_cluster->log(dpp::ll_info, "Bot sending outgoing message.");
    }
}

void ToasterBot::onSlashCommand(const dpp::slashcommand_t& event)
{
    if (event.command.get_command_name() == Command_MyRequests)
    {
        const dpp::user author = event.command.get_issuing_user();
        const std::string header = "Your current requests (ordered by priority)";
        if (m_spQueue->GetQueueSize() == 0)
        {
            dpp::embed embed;
            embed.set_title(header)
                .set_description("You have no requests in queue.")
                .set_color(0x3498db);

            event.reply( dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        }
        else
        {
            const std::string result = m_spQueue->PrintQueueByUser(author.username);
            dpp::embed embed;
            embed.set_title(header)
                .set_description(!result.empty() ? result : "You have no requests in queue.")
                .set_color(0x3498db);

            event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        }

        m_cluster->log(dpp::ll_info, author.username + " used command " + event.command.get_command_name());
    }
    else if (event.command.get_command_name() == Command_ShowQueue)
    {
        const dpp::user author = event.command.get_issuing_user();
        const std::string header = "Here is the request queue (ordered by priority)";
        if (m_spQueue->GetQueueSize() == 0)
        {
            dpp::embed embed;
            embed.set_title(header)
                .set_description("Request queue is currently empty.")
                .set_color(0x3498db);

            event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        }
        else
        {
            dpp::embed embed;
            embed.set_title(header)
                .set_description(m_spQueue->PrintQueue())
                .set_color(0x3498db);

            event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        }

        m_cluster->log(dpp::ll_info, author.username + " used command " + event.command.get_command_name());
    }
    else if (event.command.get_command_name() == "hello")
    {
        const dpp::user author = event.command.get_issuing_user();
        // Reply to the user, but only let them see the response. 
        event.reply(dpp::message("Hello! How are you today?").set_flags(dpp::m_ephemeral));

        m_cluster->log(dpp::ll_info, author.username + " used command " + event.command.get_command_name());
    }
}

void ToasterBot::onInteractionCreate(const dpp::interaction_create_t& event)
{
    if (event.command.get_command_name() == Command_JobRequest)
    {
        const dpp::user author = event.command.get_issuing_user();
        const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Cmd));

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
            CraftRequestDlg modal;
            event.dialog(modal);
        }

        m_cluster->log(dpp::ll_info, author.username + " used command " + event.command.get_command_name());
    }
    else if (event.command.get_command_name() == Command_ModifyRequest)
    {
        const dpp::user author = event.command.get_issuing_user();
        const std::string strJobID = std::get<std::string>(event.get_parameter(Parameter_Id));
        const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Cmd));

        const std::shared_ptr<JobRequest> job = m_spQueue->GetJobByGUID(strJobID);
        if (!job)
        {
            return;
        }

        m_cluster->log(dpp::ll_info, author.username + " used command " + event.command.get_command_name());

        if (strCmdID == Option_Edit)
        {
            EditRequestDlg editModal(job);
            event.dialog(editModal);
        }
        else if (strCmdID == Option_Assign)
        {
            AssignRequestDlg assignModal(job);
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
            const dpp::user author = event.command.get_issuing_user();
            const bool result = m_spQueue->DeleteJobByGUID(strJobID);

            const std::string header = author.username + " deleted job request: \n";
            event.reply(dpp::message(header + strJobID).set_flags(dpp::m_ephemeral));

            m_spQueue->SaveQueueToFile();

            m_cluster->log(dpp::ll_warning, author.username + " deleted " + strJobID);
        }
    }
    else if (event.command.get_command_name() == Command_ShowRequest)
    {
        const dpp::user author = event.command.get_issuing_user();
        const std::string strJobID = std::get<std::string>(event.get_parameter(Parameter_Id));

        const std::shared_ptr<JobRequest> job = m_spQueue->GetJobByGUID(strJobID);
        if (job)
        {
            const std::string header = "Here is the request";
            dpp::embed embed;
            embed.set_title(header)
                .set_description(job->PrintJobDetails())
                .set_color(0x3498db);

            event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        }
        else
        {
            event.reply(dpp::message("This id does not exist in queue.").set_flags(dpp::m_ephemeral));
        }

        m_cluster->log(dpp::ll_info, author.username + " used command " + event.command.get_command_name());
    }
}

void ToasterBot::onFormSubmit(const dpp::form_submit_t& event)
{
    if (event.custom_id == CraftRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strSCHandle = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strDesc = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
        const std::string strQuantity = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";
        const std::string strQuality = event.components.size() > 3 ? std::get<std::string>(event.components[3].value) : "";
        const std::string strPriority = event.components.size() > 4 ? std::get<std::string>(event.components[4].value) : "";

        auto jobCraft = std::make_shared<CraftingJobRequest>();
        jobCraft->SetAuthor(author.username);
        jobCraft->SetSCHandle(strSCHandle);
        jobCraft->SetItemDesc(strDesc);
        jobCraft->SetQuantity(std::stoull(strQuantity));
        jobCraft->SetQualityThres(strQuality);
        jobCraft->SetPriority(JobRequest::StringToPriority(strPriority));

        const std::string header = author.username + " submitted job request: \n";
        dpp::embed embed;
        embed.set_title(header)
            .set_description(jobCraft->PrintJobDetails())
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

        m_cluster->log(dpp::ll_info, author.username + " added new request " + utils::GuidToString(jobCraft->GetID()));

        m_spQueue->AddToQueue(std::move(jobCraft));
    }
    else if (event.custom_id == BuildRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strSCHandle = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strDesign = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
        const std::string strRequires = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";
        const std::string strZone = event.components.size() > 3 ? std::get<std::string>(event.components[3].value) : "";
        const std::string strPriority = event.components.size() > 4 ? std::get<std::string>(event.components[4].value) : "";

        auto jobBuild = std::make_shared<BuildingJobRequest>();
        jobBuild->SetAuthor(author.username);
        jobBuild->SetSCHandle(strSCHandle);
        jobBuild->SetBuildDesign(strDesign);
        jobBuild->SetBuildRequirments(strRequires);
        jobBuild->SetBuildZone(strZone);
        jobBuild->SetPriority(JobRequest::StringToPriority(strPriority));

        const std::string header = author.username + " submitted job request: \n";
        dpp::embed embed;
        embed.set_title(header)
            .set_description(jobBuild->PrintJobDetails())
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

        m_cluster->log(dpp::ll_info, author.username + " added new request " + utils::GuidToString(jobBuild->GetID()));

        m_spQueue->AddToQueue(std::move(jobBuild));
    }
    else if (event.custom_id == ComponentRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strSCHandle = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strCompList = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
        const std::string strPriority = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";

        auto jobComp = std::make_shared<ComponentJobRequest>();
        jobComp->SetAuthor(author.username);
        jobComp->SetSCHandle(strSCHandle);
        jobComp->SetComponentList(strCompList);
        jobComp->SetPriority(JobRequest::StringToPriority(strPriority));

        const std::string header = author.username + " submitted job request: \n";
        dpp::embed embed;
        embed.set_title(header)
            .set_description(jobComp->PrintJobDetails())
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

        m_cluster->log(dpp::ll_info, author.username + " added new request " + utils::GuidToString(jobComp->GetID()));

        m_spQueue->AddToQueue(std::move(jobComp));
    }
    else if (event.custom_id == ResourceRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strSCHandle = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strResState = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
        const std::string strResList = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";
        const std::string strQuality = event.components.size() > 3 ? std::get<std::string>(event.components[3].value) : "";
        const std::string strPriority = event.components.size() > 4 ? std::get<std::string>(event.components[4].value) : "";

        auto jobRes = std::make_shared<ResourceJobRequest>();
        jobRes->SetAuthor(author.username);
        jobRes->SetSCHandle(strSCHandle);
        jobRes->SetResourceState(ResourceJobRequest::StringToState(strResState));
        jobRes->SetResourcelist(strResList);
        jobRes->SetQualityThres(strQuality);
        jobRes->SetPriority(JobRequest::StringToPriority(strPriority));

        const std::string header = author.username + " submitted job request: \n";
        dpp::embed embed;
        embed.set_title(header)
            .set_description(jobRes->PrintJobDetails())
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

        m_cluster->log(dpp::ll_info, author.username + " added new request " + utils::GuidToString(jobRes->GetID()));

        m_spQueue->AddToQueue(std::move(jobRes));
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
        if (job->SupportsType(JOB_TYPE_CRAFTING))
        {
            std::shared_ptr<CraftingJobRequest> craft = std::dynamic_pointer_cast<CraftingJobRequest>(job);
            craft->SetSCHandle(strSCHandle);
            craft->SetItemDesc(strParam1);
            craft->SetQuantity(std::stoull(strParam2));
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
        }

        if (job)
        {
            const std::string header = "Edited job request : \n";
            dpp::embed embed;
            embed.set_title(header)
                .set_description(job->PrintJobDetails())
                .set_color(0x3498db);

            event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
            m_spQueue->SaveQueueToFile();

            m_cluster->log(dpp::ll_info, author.username + " edited " + strID);
        }
        else
        {
            m_cluster->log(dpp::ll_error, author.username + " attempted to edit " + strID);
        }
    }
    else if (event.custom_id == AssignRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strWorker = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
        const std::string strStatus = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";

        std::shared_ptr<JobRequest> job = m_spQueue->GetJobByGUID(strID);
        if (job)
        {
            job->SetWorker(strWorker);
            job->SetStatus(CraftingJobRequest::StringToStatus(strStatus));

            const std::string header = author.username + " assigned job request: \n";
            event.reply(dpp::message(header + job->PrintJobDetails()).set_flags(dpp::m_ephemeral));

            m_spQueue->SaveQueueToFile();

            m_cluster->log(dpp::ll_info, author.username + " assigned " + strID + " to " + strWorker);
        }
        else
        {
            m_cluster->log(dpp::ll_error, author.username + " attempted to assign " + strID + " to a worker");
        }
    }
    else if (event.custom_id == StatusChangeRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strStatus = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";

        std::shared_ptr<JobRequest> job = m_spQueue->GetJobByGUID(strID);
        if (job)
        {
            job->SetStatus(CraftingJobRequest::StringToStatus(strStatus));

            const std::string header = author.username + " updated job status: \n";
            event.reply(dpp::message(header + job->PrintJobDetails()).set_flags(dpp::m_ephemeral));

            m_spQueue->SaveQueueToFile();

            m_cluster->log(dpp::ll_info, author.username + " updated " + strID + " status to " + strStatus);
        }
        else
        {
            m_cluster->log(dpp::ll_error, author.username + " attempted to update " + strID + " to " + strStatus);
        }
    }
    else if (event.custom_id == PriorityChangeRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strPriority = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";

        std::shared_ptr<JobRequest> job = m_spQueue->GetJobByGUID(strID);
        if (job)
        {
            job->SetPriority(JobRequest::StringToPriority(strPriority));

            const std::string header = author.username + " updated job priority: \n";
            event.reply(dpp::message(header + job->PrintJobDetails()).set_flags(dpp::m_ephemeral));

            m_spQueue->SaveQueueToFile();

            m_cluster->log(dpp::ll_info, author.username + " changed priority for " + strID + " to " + strPriority);
        }
        else
        {
            m_cluster->log(dpp::ll_error, author.username + " attempted to change priority for " + strID + " to " + strPriority);
        }
    }
}