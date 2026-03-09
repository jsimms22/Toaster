//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief Implementation of MyRequestsCommand.
///
/// Provides functionality for users to view and paginate through their own submitted job requests.
///
/// All results are scoped to the requesting user and validated
/// against the active JobQueue instance.
//---------------------------------------------------------------------------------------------------------------------
#include "Commands.h"

#include "BotUtility.h"
#include "JobQueue.h"
#include "JobRequest.h"
#include "PaginationPanel.h"
// fmt
#include <fmt/format.h>

//---------------------------------------------------------------------------------------------------------------------
/// \brief Handles the initial slash command interaction for viewing user requests.
///
/// Response is always ephemeral and scoped to the requesting user.
///
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Interaction create event.
//---------------------------------------------------------------------------------------------------------------------
void MyRequestsCommand::ExecuteInteraction(CommandContext& ctx,  const dpp::interaction_create_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    // Return early if queue is empty
    if (ctx.queue->GetQueueSize() == 0)
    {
        dpp::embed embed;
        embed.set_title("Your Requests:")
            .set_description("No requests in queue.")
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

        return;
    }

    // Acknowledge immediately to avoid timing out
    event.reply(dpp::message("...this could take a second. Please hold.").set_flags(dpp::m_ephemeral));

    // Construct the actual info panel
    const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Type));
    const std::size_t type = utils::CmdStringToJobType(strCmdID);
    const dpp::snowflake user = event.command.get_issuing_user().id;
    const std::size_t page = 0;
    const std::string result = ctx.queue->PrintPagedQueue(ctx.cluster,
                                                          event.command.guild_id,
                                                          page,
                                                          [user, type](const std::shared_ptr<const JobRequest> job) -> bool
                                                          { return job->GetCustomerID() == user && job->SupportsType(type); });
    const std::size_t size = ctx.queue->GetQueueSize([user, type](const std::shared_ptr<const JobRequest> job) -> bool
                                                    { return job->GetCustomerID() == user && job->SupportsType(type); });
    const std::string header = "Your Requests"; 

    PaginationPanel panel(ctx, this->name, type, page, size, JobQueue::JOBS_PER_DETAIL_PAGE, user);
    panel.AddEmbed(header, size != 0 ? result : "You have no job requests of this type in queue.");

    // Edit the original message
    event.edit_original_response(panel.set_flags(dpp::m_ephemeral));
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Handles pagination button clicks for user request listings. Updates the original ephemeral message 
/// using ir_update_message.
///
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Button click interaction event.
//---------------------------------------------------------------------------------------------------------------------
void MyRequestsCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
    const std::string id = event.custom_id;

    if (id.starts_with(fmt::format("{}:", this->name)))
    {
        // Decompose custom id for state information
        const auto parts = utils::Split(id, ':');
        const std::size_t type = std::stoul(parts[1]);
        const std::size_t page = parts[2] != std::to_string(std::numeric_limits<std::size_t>::max()) ? std::stoul(parts[2]) : 0;
        const dpp::snowflake user = parts[3];

        // Construct the actual info panel
        const std::string result = ctx.queue->PrintPagedQueue(ctx.cluster,
                                                              event.command.guild_id,
                                                              page,
                                                              [user, type](const std::shared_ptr<const JobRequest> job) -> bool
                                                              { return job->GetCustomerID() == user && job->SupportsType(type); });
        const std::size_t size = ctx.queue->GetQueueSize([user, type](const std::shared_ptr<const JobRequest> job) -> bool
                                                        { return job->GetCustomerID() == user && job->SupportsType(type); });
        const std::string header = "Your Requests";

        PaginationPanel panel(ctx, this->name, type, page, size, JobQueue::JOBS_PER_DETAIL_PAGE, user);
        panel.AddEmbed(header, size != 0 ? result : "You have no job requests of this type in queue.");

        // Edit the original message
        event.reply(dpp::ir_update_message, panel.set_flags(dpp::m_ephemeral));
    }
}