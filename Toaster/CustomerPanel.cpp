#include "CustomerPanel.h"

#include "BotUtility.h"
#include "CommandContext.h"
#include "JobRequest.h"
#include "JobQueue.h"
#include "RequestDlg.h"
// fmt
#include <fmt/format.h>


CustomerPanel::CustomerPanel(
	CommandContext& ctx,
	const std::string& OwnerName,
	const dpp::snowflake& userID,
	const std::string& jobID, 
    const bool bSubscribe)
    : GeneralUserPanel(ctx, OwnerName, userID, jobID)
{
    m_btnSubscribe.set_type(dpp::cot_button)
        .set_label(bSubscribe ? "Subscribed" : "Unsubscribed")
        .set_style(bSubscribe ? dpp::cos_primary : dpp::cos_secondary)
        .set_id(fmt::format("{}_subscribe:{}:{}", OwnerName, userID, jobID));

    m_row.add_component(m_btnEdit)
        .add_component(m_btnNote)
        .add_component(m_btnSubscribe)
        .add_component(m_btnDelete);

    add_component(m_row);
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief 
///
/// \param[in] id        Button id string
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Modal form submission event.
//---------------------------------------------------------------------------------------------------------------------
void CustomerPanel::SubscribeButton(const std::string& id, CommandContext& ctx, const dpp::button_click_t& event)
{
    auto parts = utils::Split(id, ':');
    const dpp::snowflake user = parts[1];
    const std::string guid = parts[2];

    auto job = ctx.queue->GetJobByGUID(guid);
    if (!job)
    {
        event.reply(dpp::message("Could not find the job by its ID. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
        return;
    }

    const auto pManager = PermissionsMgr::GetInstance();
    if (pManager->IsRequestOwner(user, job))
    {
        // Grab job details now before we send an edit request
        const std::string strJobID = utils::GuidToStringNoBrackets(job->GetID());
        const bool bSubscribe = !job->IsCustomerSubscribed();

        ctx.queue->RequestModifyJob(job->GetID(), [bSubscribe](std::shared_ptr<JobRequest> job)
            {
                job->SubscribeCustomer(bSubscribe);
                job->SetLastEditTime(utils::GetEpochTimestamp());
            });

        ctx.cluster.log(dpp::ll_info, fmt::format("User changed subscribed status for '{}' to '{}'.", strJobID, bSubscribe ? "Subscribe" : "Unsubscribe"));
    }
    else if (!job)
    {
        event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
        return;
    }
    else
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_debug, fmt::format("USER '{}' was DENIED access to use '{}' button.", user, parts[0]));
        return;
    }
}