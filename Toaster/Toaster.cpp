#include "Toaster.h"

#include "BotUtility.h"
#include "Command.h"
#include "JobQueue.h"
#include "Resource.h"

#include "JobRequestFactory.h"
#include "RequestDlg.h"
#include "CraftingJobRequest.h"
#include "BuildingJobRequest.h"
#include "ComponentJobRequest.h"
#include "ResourceJobRequest.h"

#include <dpp/dpp.h>
// Microsoft
#include <guiddef.h>

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

ToasterBot::ToasterBot(dpp::cluster* cluster, const uint32_t clusterId, const std::shared_ptr<JobQueue>& spQueue, const bool bDebug)
    : m_cluster{ cluster }, m_clusterId{ clusterId }, m_spQueue{ spQueue }, m_debug{ bDebug }, m_iShardCount{ 0 } { }

void ToasterBot::onReady(const dpp::ready_t& event)
{
    if (dpp::run_once<struct clear_bot_commands>()) 
    {
        //m_cluster->guild_bulk_command_delete(1472034166869852287);
        //m_cluster->global_bulk_command_delete();

        //m_cluster->log(dpp::ll_debug, "Deleted commands for guild id: " + std::to_string(1472034166869852287));
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
        m_cluster->log(dpp::ll_error, "Message dropped, no author: {}" + event.msg.content);
        return;
    } 
    else if (event.msg.author.id == m_cluster->me.id) 
    {
        m_cluster->log(dpp::ll_info, "Bot sending outgoing message.");
    }
}

void ToasterBot::onSlashCommand(const dpp::slashcommand_t& event)
{   
    if (event.command.get_command_name() == Command_Hello)
    {
        const dpp::user author = event.command.get_issuing_user();
        // Reply to the user, but only let them see the response. 
        event.reply(dpp::message("Hello! How are you today?").set_flags(dpp::m_ephemeral));

        m_cluster->log(dpp::ll_info, author.username + " used command " + event.command.get_command_name());
    }
    else if (event.command.get_command_name() == Command_MyAssignments ||
             event.command.get_command_name() == Command_MyTopAssignment)
    {
        const dpp::user author = event.command.get_issuing_user();
        const std::string header = event.command.get_command_name() == Command_MyTopAssignment ? "Top Assigment (by priority):" : "Your Assignments:";
        if (m_spQueue->GetQueueSize() == 0)
        {
            dpp::embed embed;
            embed.set_title(header)
                .set_description("No requests or jobs currently assigned to you.")
                .set_color(0x3498db);

            event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        }
        else if (event.command.get_command_name() == Command_MyAssignments)
        {
            const std::string result = m_spQueue->PrintQueueByWorker(author.username);
            dpp::embed embed;
            embed.set_title(header)
                .set_description(!result.empty() ? result : "No requests or jobs currently assigned to you.")
                .set_color(0x3498db);

            event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        }
        else if (event.command.get_command_name() == Command_MyTopAssignment)
        {
            const std::string result = m_spQueue->PrintFirstAssignment(author.username);
            dpp::embed embed;
            embed.set_title(header)
                .set_description(!result.empty() ? result : "No requests or jobs currently assigned to you.")
                .set_color(0x3498db);

            dpp::component button1 = dpp::component()
                .set_type(dpp::cot_button)
                .set_label("Complete")
                .set_style(dpp::cos_success)
                .set_id(Button_Complete);

            dpp::component button2 = dpp::component()
                .set_type(dpp::cot_button)
                .set_label("Add Note")
                .set_style(dpp::cos_primary)
                .set_id(Button_Note);

            dpp::component button3 = dpp::component()
                .set_type(dpp::cot_button)
                .set_label("Unassign")
                .set_style(dpp::cos_primary)
                .set_id(Button_Unassign);

            dpp::component button4 = dpp::component()
                .set_type(dpp::cot_button)
                .set_label("Delete")
                .set_style(dpp::cos_danger)
                .set_id(Button_Delete);

            dpp::component row = dpp::component()
                .set_type(dpp::cot_action_row)
                .add_component(button1)
                .add_component(button2)
                .add_component(button3)
                .add_component(button4);

            event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral).add_component(row));
        }

        m_cluster->log(dpp::ll_info, author.username + " used command " + event.command.get_command_name());
    }
}

void ToasterBot::onInteractionCreate(const dpp::interaction_create_t& event)
{
    if (event.command.get_command_name() == Command_JobRequest)
    {
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

        m_cluster->log(dpp::ll_info, author.username + " used command " + event.command.get_command_name() + " with cmd option " + strCmdID);
    }
    else if (event.command.get_command_name() == Command_MyRequests ||
             event.command.get_command_name() == Command_ShowQueue)
    {
        const dpp::user author = event.command.get_issuing_user();
        const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Type));
        
        const std::size_t filterType = CmdStringToJobType(strCmdID);
        const std::string header = event.command.get_command_name() == Command_MyRequests ? "Your Requests:" : "Request Queue:";
        if (m_spQueue->GetQueueSize() == 0)
        {
            dpp::embed embed;
            embed.set_title(header)
                .set_description("Request queue is currently empty.")
                .set_color(0x3498db);

            event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        }
        else if (event.command.get_command_name() == Command_MyRequests)
        {
            const std::string result = m_spQueue->PrintQueueByUser(author.username, filterType);
            dpp::embed embed;
            embed.set_title(header)
                .set_description(!result.empty() ? result : "You have no requests of this type in queue.")
                .set_color(0x3498db);

            event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        }
        else if (event.command.get_command_name() == Command_ShowQueue)
        {
            const std::string result = m_spQueue->PrintQueueByType(filterType);
            dpp::embed embed;
            embed.set_title(header)
                .set_description(!result.empty() ? result : "No requests of this type in queue.")
                .set_color(0x3498db);

            event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        }

        m_cluster->log(dpp::ll_info, author.username + " used command " + event.command.get_command_name());
    }
    else if (event.command.get_command_name() == Command_ModifyRequest)
    {
        const dpp::snowflake guild = event.command.guild_id;
        const dpp::user author = event.command.get_issuing_user();
        const std::string strJobID = std::get<std::string>(event.get_parameter(Parameter_Id));
        const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Cmd));

        const std::shared_ptr<JobRequest> job = m_spQueue->GetJobByGUID(strJobID);
        if (!job)
        {
            event.reply(dpp::message("Could not find id: " + strJobID).set_flags(dpp::m_ephemeral));
            m_cluster->log(dpp::ll_warning, author.username + " could not find " + strJobID + " to handle cmd " + strCmdID);
            return;
        }
        // Check for permissions to edit a job request
        else if (author.username != job->GetAuthor() /*|| todo get permissions list */)
        {
            event.reply(dpp::message("You do not have permissions to modify: " + strJobID).set_flags(dpp::m_ephemeral));
            m_cluster->log(dpp::ll_warning, author.username + " attempted to " + strCmdID  + " job " + strJobID);
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
            job->SetLastEditTime(utils::GetEpochTimestamp());
            const std::string jobDetails = job->PrintJobDetails();
            const bool result = m_spQueue->DeleteJobByGUID(strJobID);
            if (!result)
            {
                event.reply(dpp::message("Failed to delete id: " + strJobID).set_flags(dpp::m_ephemeral));
                m_cluster->log(dpp::ll_warning, author.username + " failed to delete " + strJobID);
            }

            event.reply(dpp::message("Deleted job request:\n\n" + jobDetails).set_flags(dpp::m_ephemeral));
            m_spQueue->SaveQueueToFile();
            m_cluster->log(dpp::ll_warning, author.username + " deleted " + strJobID);

            if (author.username != job->GetAuthor() || m_debug)
            {
                const std::string strNotifyUser = "Request **" + strJobID + "** has been deleted by " + author.username + ":\n\n";
                NotifyIssuerMsg(author, event, strNotifyUser + jobDetails);
            }
        }
    }
    else if (event.command.get_command_name() == Command_ShowRequest)
    {
        const dpp::user author = event.command.get_issuing_user();
        const std::string strJobID = std::get<std::string>(event.get_parameter(Parameter_Id));

        const std::shared_ptr<JobRequest> job = m_spQueue->GetJobByGUID(strJobID);
        m_cluster->log(dpp::ll_info, author.username + " used command " + event.command.get_command_name());

        if (!job)
        {
            event.reply(dpp::message("This id does not exist in queue.").set_flags(dpp::m_ephemeral));
            return;
        }

        dpp::embed embed;
        embed.set_title("Here is the request:")
            .set_description(job->PrintJobDetails())
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
    }
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
            jobCraft->SetAuthor(author.username);
            jobCraft->SetSCHandle(strSCHandle);
            jobCraft->SetItemDesc(strParam1);
            jobCraft->SetQuantity(strParam2);
            jobCraft->SetQualityThres(strParam3);
            jobCraft->SetPriority(JobRequest::StringToPriority(strParam4));
            jobDetails = jobCraft->PrintJobDetails();
            m_cluster->log(dpp::ll_info, author.username + " added new request " + utils::GuidToString(jobCraft->GetID()));
            m_spQueue->AddToQueue(std::move(jobCraft));
        }
        else if (event.custom_id == BuildRequestDlg::modalID)
        {
            std::shared_ptr<BuildingJobRequest> jobBuild = std::make_shared<BuildingJobRequest>();
            jobBuild->SetAuthor(author.username);
            jobBuild->SetSCHandle(strSCHandle);
            jobBuild->SetBuildDesign(strParam1);
            jobBuild->SetBuildRequirments(strParam2);
            jobBuild->SetBuildZone(strParam3);
            jobBuild->SetPriority(JobRequest::StringToPriority(strParam4));
            jobDetails = jobBuild->PrintJobDetails();
            m_cluster->log(dpp::ll_info, author.username + " added new request " + utils::GuidToString(jobBuild->GetID()));
            m_spQueue->AddToQueue(std::move(jobBuild));
        }
        else if (event.custom_id == ComponentRequestDlg::modalID)
        {
            std::shared_ptr<ComponentJobRequest> jobComp = std::make_shared<ComponentJobRequest>();
            jobComp->SetAuthor(author.username);
            jobComp->SetSCHandle(strSCHandle);
            jobComp->SetComponentList(strParam1);
            jobComp->SetPriority(JobRequest::StringToPriority(strParam2));
            jobDetails = jobComp->PrintJobDetails();
            m_cluster->log(dpp::ll_info, author.username + " added new request " + utils::GuidToString(jobComp->GetID()));
            m_spQueue->AddToQueue(std::move(jobComp));
        }
        else if (event.custom_id == ResourceRequestDlg::modalID)
        {
            std::shared_ptr<ResourceJobRequest> jobRes = std::make_shared<ResourceJobRequest>();
            jobRes->SetAuthor(author.username);
            jobRes->SetSCHandle(strSCHandle);
            jobRes->SetResourceState(ResourceJobRequest::StringToState(strParam1));
            jobRes->SetResourcelist(strParam2);
            jobRes->SetQualityThres(strParam3);
            jobRes->SetPriority(JobRequest::StringToPriority(strParam4));
            jobDetails = jobRes->PrintJobDetails();
            m_cluster->log(dpp::ll_info, author.username + " added new request " + utils::GuidToString(jobRes->GetID()));
            m_spQueue->AddToQueue(std::move(jobRes));
        }
        else if (event.custom_id == RefineryRequestDlg::modalID)
        {
            std::shared_ptr<RefineryJobRequest> jobRefine = std::make_shared<RefineryJobRequest>();
            jobRefine->SetAuthor(author.username);
            jobRefine->SetSCHandle(strSCHandle);
            jobRefine->SetResourceState(RefineryJobRequest::StringToState(strParam1));
            jobRefine->SetResourcelist(strParam2);
            jobRefine->SetRefinery(strParam3);
            jobRefine->SetPriority(JobRequest::StringToPriority(strParam4));
            jobDetails = jobRefine->PrintJobDetails();
            m_cluster->log(dpp::ll_info, author.username + " added new request " + utils::GuidToString(jobRefine->GetID()));
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
        if (!job || (job->GetAuthor() != author.username && !m_debug))
        {
            m_cluster->log(dpp::ll_error, author.username + " attempted to edit " + strID);
            return;
        }
        job->SetLastEditTime(utils::GetEpochTimestamp());

        const std::string strOldJobDetails = job->PrintJobDetails();
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

        const std::string strNewJobDetails = job->PrintJobDetails();
        dpp::embed embed;
        embed.set_title("Edited job:")
            .set_description(strNewJobDetails)
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        m_spQueue->SaveQueueToFile();

        m_cluster->log(dpp::ll_info, author.username + " edited " + strID);

        if ((author.username != job->GetAuthor() && strOldJobDetails != strNewJobDetails) || m_debug)
        {
            const std::string strNotifyUser = "Request **" + utils::GuidToStringNoBrackets(job->GetID()) + "** has been edited by " + author.username + ":\n\n";
            NotifyIssuerMsg(author, event, strNotifyUser + "New Job Details:\n" + strNewJobDetails);
        }
    }
    else if (event.custom_id == AssignRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strWorker = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";
        const std::string strStatus = event.components.size() > 2 ? std::get<std::string>(event.components[2].value) : "";

        std::shared_ptr<JobRequest> job = m_spQueue->GetJobByGUID(strID);
        if (!job || (job->GetAuthor() != author.username && !m_debug))
        {
            m_cluster->log(dpp::ll_error, author.username + " attempted to assign " + strID + " to a worker");
            return;
        }
        job->SetLastEditTime(utils::GetEpochTimestamp());

        job->SetWorker(strWorker);
        job->SetStatus(CraftingJobRequest::StringToStatus(strStatus));

        dpp::embed embed;
        embed.set_title("Assigned job:")
            .set_description(job->PrintJobDetails())
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        m_spQueue->SaveQueueToFile();

        m_cluster->log(dpp::ll_info, author.username + " assigned " + strID + " to " + strWorker);

        if (author.username != job->GetAuthor() || m_debug)
        {
            const std::string strNotifyUser = "Your request has been assigned to **" + strWorker + "** to by " + author.username + ":\n\n";
            NotifyIssuerMsg(author, event, strNotifyUser + job->PrintJobDetails());
        }
    }
    else if (event.custom_id == StatusChangeRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strStatus = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";

        std::shared_ptr<JobRequest> job = m_spQueue->GetJobByGUID(strID);
        if (!job || (job->GetAuthor() != author.username && !m_debug))
        {
            m_cluster->log(dpp::ll_error, author.username + " attempted to update " + strID + " to " + strStatus);
        }
        job->SetLastEditTime(utils::GetEpochTimestamp());

        const std::string strOldStatus = JobRequest::StatusToString(job->GetStatus());
        job->SetStatus(CraftingJobRequest::StringToStatus(strStatus));

        dpp::embed embed;
        embed.set_title("Status changed for job:")
            .set_description(job->PrintJobDetails())
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        m_spQueue->SaveQueueToFile();

        m_cluster->log(dpp::ll_info, author.username + " updated " + strID + " status to " + strStatus);

        if ((author.username != job->GetAuthor() && strOldStatus != strStatus) || m_debug)
        {
            const std::string strNotifyUser = "Your request's status has moved from **" + strOldStatus + "** to **" + strStatus + "** by " + author.username + ":\n\n";
            NotifyIssuerMsg(author, event, strNotifyUser + job->PrintJobDetails());
        }
    }
    else if (event.custom_id == PriorityChangeRequestDlg::modalID)
    {
        const dpp::user author = event.command.get_issuing_user();

        const std::string strID = !event.components.empty() ? std::get<std::string>(event.components[0].value) : "";
        const std::string strPriority = event.components.size() > 1 ? std::get<std::string>(event.components[1].value) : "";

        std::shared_ptr<JobRequest> job = m_spQueue->GetJobByGUID(strID);
        if (!job || (job->GetAuthor() != author.username && !m_debug))
        {
            m_cluster->log(dpp::ll_error, author.username + " attempted to change priority for " + strID + " to " + strPriority);
            return;
        }
        job->SetLastEditTime(utils::GetEpochTimestamp());

        const std::string strOldPriority = JobRequest::PriorityToString(job->GetPriority());
        job->SetPriority(JobRequest::StringToPriority(strPriority));

        dpp::embed embed;
        embed.set_title("Priority changed for job:")
            .set_description(job->PrintJobDetails())
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
        m_spQueue->SaveQueueToFile();

        m_cluster->log(dpp::ll_info, author.username + " changed priority for " + strID + " to " + strPriority);

        if ((author.username != job->GetAuthor() && strOldPriority != strPriority) || m_debug)
        {
            const std::string strNotifyUser = "Your request's priority has moved from **" + strOldPriority + "** to **" + strPriority + "** by " + author.username + ":\n\n";
            NotifyIssuerMsg(author, event, strNotifyUser + job->PrintJobDetails());
        }
    }
}

void ToasterBot::onButtonClick(const dpp::button_click_t& event)
{
    if (event.custom_id == Button_Complete)
    {
        m_cluster->log(dpp::ll_info, Button_Complete);
    }
    else if (event.custom_id == Button_Note)
    {
        m_cluster->log(dpp::ll_info, Button_Note);
    }
    else if (event.custom_id == Button_Unassign)
    {
        m_cluster->log(dpp::ll_info, Button_Unassign);
    }
    else if (event.custom_id == Button_Delete)
    {
        m_cluster->log(dpp::ll_info, Button_Delete);
    }
}

void ToasterBot::NotifyIssuerMsg(const dpp::user& user, const dpp::event_dispatch_t& event, const std::string& msg)
{
    m_cluster->direct_message_create(user.id, dpp::message(msg), [user, cluster = m_cluster](const dpp::confirmation_callback_t& callback) {
        if (callback.is_error())
            cluster->log(dpp::ll_error, "Error sending private message to user: " + user.username);
        else
            cluster->log(dpp::ll_info, "Sent private message to user: " + user.username);
        });
}