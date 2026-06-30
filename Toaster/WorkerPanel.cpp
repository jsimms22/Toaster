#include "WorkerPanel.h"

#include "CommandContext.h"
#include "GuildSettings.h"
#include "PermissionsMgr.h"
#include "JobRequest.h"
#include "JobQueue.h"
#include "BotUtility.h"
#include "RequestDlg.h"
#include "RequestID.h"
// fmt
#include <fmt/format.h>

WorkerPanel::WorkerPanel(
	const std::string& OwnerName,
	const dpp::snowflake& userID,
	const std::string& jobID,
    const bool bIsWorker)
	: GeneralUserPanel(OwnerName, userID, jobID)
{
    m_btnRefreshPanel.set_type(dpp::cot_button)
        .set_label("Refresh Panel")
        .set_style(dpp::cos_primary)
        .set_id(fmt::format("{}_refresh:{}:{}", m_ownerName, m_userID, m_jobID));

	m_btnComplete.set_type(dpp::cot_button)
		.set_label("Complete")
		.set_style(dpp::cos_success)
		.set_id(fmt::format("{}_complete:{}:{}", m_ownerName, m_userID, m_jobID));

	m_btnAssign.set_type(dpp::cot_button)
		.set_label(bIsWorker ? "Unassign Me" : "Assign Me")
		.set_style(bIsWorker ? dpp::cos_primary : dpp::cos_success)
		.set_id(fmt::format("{}_unassign:{}:{}", m_ownerName, m_userID, m_jobID));

	m_row.add_component(m_btnRefreshPanel)
        .add_component(m_btnComplete)
		.add_component(m_btnEdit)
		.add_component(m_btnAssign)
		.add_component(m_btnDelete);

    m_row2.add_component(m_btnNote)
        .add_component(m_btnShowNotes);

    add_component(m_row);
    add_component(m_row2);
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief 
///
/// \param[in] id        Button id string
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Modal form submission event.
//---------------------------------------------------------------------------------------------------------------------
void WorkerPanel::CompleteButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
    auto parts = utils::Split(id, ':');
    const dpp::snowflake worker = parts[1];
    const std::string rID = parts[2];

    const auto job = ctx.queue->GetJobByID(rID);
    if (!job)
    {
        event.reply(dpp::message("Could not find the job by its ID. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
        return;
    }

    const auto pManager = PermissionsMgr::GetInstance();
    if ((pManager->CanAssignJob(event, worker,job,utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) && 
         job->GetStatus() < JobRequest::status::complete) || ctx.debug)
    {
        ctx.queue->RequestModify(job->GetID(), [](std::shared_ptr<JobRequest> job)
            {
                job->SetStatus(JobRequest::status::complete);
            });

        const dpp::snowflake customer = job->GetCustomerID();
        // Temporarily forcing a message to send on complete per request by Leaf
        if ((/*job->IsCustomerSubscribed() &&*/ customer != worker) || ctx.debug)
        {
            utils::NotifyIssuerMsgWithEmbed(
                ctx.cluster,
                event,
                customer,
                fmt::format("Request {} has been completed by {}.", rID, fmt::format("<@{}>", event.command.get_issuing_user().id)),
                "Request Details",
                job->PrintJobDetails(ctx.cluster, event.command.guild_id)
            );
        }

        ctx.cluster.log(dpp::ll_info, fmt::format("Request '{}' has been set to completed by USER '{}'.", ToString(job->GetID()), event.command.get_issuing_user().global_name));

        GuildSettings::AnnounceOnComplete(ctx, job, worker);
    }
    else
    {
        event.reply(dpp::message("Could not perform this action.").set_flags(dpp::m_ephemeral));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief 
///
/// \param[in] id        Button id string
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Modal form submission event.
//---------------------------------------------------------------------------------------------------------------------
void WorkerPanel::UnassignButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
    auto parts = utils::Split(id, ':');
    dpp::snowflake user = parts[1];
    const std::string rID = parts[2];

    const auto job = ctx.queue->GetJobByID(rID);
    if (!job)
    {
        event.reply(dpp::message("Could not find the job by its ID. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
        return;
    }

    const auto pManager = PermissionsMgr::GetInstance();
    if (!pManager->CanAssignJob(event, user, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) && !ctx.debug)
    {
        // Permission denied
        event.reply(dpp::message("You do not have permissions to change assignments for this job.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("USER '{}' attempted to reassign a job with a button.", event.command.get_issuing_user().global_name));
        return;
    }

    if (job->GetStatus() < JobRequest::status::complete)
    {
        if (job->IsWorker(user))
        {
            ctx.queue->RequestModify(job->GetID(), [user](std::shared_ptr<JobRequest> job)
                {
                    job->RemoveWorkerID(user);
                    if (job->GetStatus() == JobRequest::status::active ||
                        job->GetStatus() == JobRequest::status::assigned)
                    {
                        job->SetStatus(JobRequest::status::open);
                    }
                });

            // Send notification to the customer
            const dpp::snowflake customer = job->GetCustomerID();
            if ((job->IsCustomerSubscribed() && customer != user) || ctx.debug)
            {
                utils::NotifyIssuerMsgWithEmbed(
                    ctx.cluster,
                    event,
                    customer,
                    fmt::format("{} has been unassigned from your request {}.", fmt::format("<@{}>", user), rID),
                    "Request Details",
                    job->PrintJobDetails(ctx.cluster, event.command.guild_id)
                );
            }

            ctx.cluster.log(dpp::ll_info, fmt::format("Request '{}' has been set to unassigned by USER '{}'.", ToString(job->GetID()), event.command.get_issuing_user().global_name));
        }
        else if (!job->IsWorker(user))
        {
            ctx.queue->RequestModify(job->GetID(), [user](std::shared_ptr<JobRequest> job)
                {
                    job->AddWorkerID(user);
                    if (job->GetStatus() != JobRequest::status::assigned ||
                        job->GetStatus() != JobRequest::status::active)
                    {
                        job->SetStatus(JobRequest::status::assigned);
                    }
                });

            // Send notification to the customer
            const dpp::snowflake customer = job->GetCustomerID();
            if ((job->IsCustomerSubscribed() && customer != user) || ctx.debug)
            {
                utils::NotifyIssuerMsgWithEmbed(
                    ctx.cluster,
                    event,
                    customer,
                    fmt::format("{} has been assigned to your request {}.", fmt::format("<@{}>", user), rID),
                    "Request Details",
                    job->PrintJobDetails(ctx.cluster, event.command.guild_id)
                );
            }

            ctx.cluster.log(dpp::ll_info, fmt::format("Request '{}' has been set to assigned by USER '{}'.", ToString(job->GetID()), event.command.get_issuing_user().global_name));
        }
        else
        {
            event.reply(dpp::message("Could not find perform this action.").set_flags(dpp::m_ephemeral));
        }
    }
    else
    {
        event.reply(dpp::message("This job has already been completed. To change assignment do so through the modify command.").set_flags(dpp::m_ephemeral));
    }
}