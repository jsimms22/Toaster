//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief Implementation of ShowQueueCommand.
///
/// Provides a paginated view of the global job request queue filtered by type.
//---------------------------------------------------------------------------------------------------------------------
#include "Commands.h"

#include "BotUtility.h"
#include "JobRequest.h"
#include "JobQueue.h"
#include "PaginationPanel.h"
#include "PermissionsMgr.h"
// fmt
#include <fmt/format.h>

//---------------------------------------------------------------------------------------------------------------------
/// \brief Handles the slash command interaction for viewing the request queue.
///
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Interaction create event.
//---------------------------------------------------------------------------------------------------------------------
void ShowQueueCommand::ExecuteInteraction(CommandContext& ctx,  const dpp::interaction_create_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    // Check Permissions
    const dpp::user author = event.command.get_issuing_user();
    const auto pManager = PermissionsMgr::GetInstance();
    if (!(pManager->IsWorker(author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) ||
          pManager->IsManager(author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) ||
          pManager->IsGuildAdmin(author.id, event) ||
          pManager->IsBotOwner(author.id)) && !ctx.debug)
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("USER '{}' was DENIED access to use '{}' command", author.id, event.command.get_command_name()));
        return;
    }

    // Return early if queue is empty
    if (ctx.queue->GetQueueSize() == 0)
    {
        dpp::embed embed;
        embed.set_title("Request Queue:")
            .set_description("Request queue is currently empty for this type.")
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

        return;
    }

    // Acknowledge immediately to avoid timing out
    event.reply(dpp::message("...this could take a second. Please hold.").set_flags(dpp::m_ephemeral));

    // Construct the actual info panel
    const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Type));
    const std::size_t type = utils::CmdStringToJobType(strCmdID);
    const std::size_t page = 0;
    const bool bShowComplete = true;

    auto compare = [type](const std::shared_ptr<const JobRequest> job) -> bool { return job->SupportsType(type); };
    const std::string result = ctx.queue->PrintPagedQueueCompact(ctx.cluster, event.command.guild_id, page, bShowComplete, compare);
    const std::size_t size = ctx.queue->GetQueueSize(bShowComplete, compare);

    const std::string header = "Request Queue";

    PaginationPanel panel(ctx, this->name, type, page, size, JobQueue::JOBS_PER_QUEUE_PAGE, bShowComplete);
    panel.AddEmbed(header, size != 0 ? result : "Request queue is currently empty for this type.");

    // Edit the original message
    event.edit_original_response(panel.set_flags(dpp::m_ephemeral));
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Handles pagination button clicks for the global request queue.
///
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Button click interaction event.
//---------------------------------------------------------------------------------------------------------------------
void ShowQueueCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
    const std::string id = event.custom_id;
    auto parts = utils::Split(id, ':');
    if (parts[0] == fmt::format("{}_pageshow", this->name))
    {
        parts[4] = parts[4] == "true" ? "false" : "true";
    }

    if (parts[0] == fmt::format("{}_pagenext", this->name) ||
        parts[0] == fmt::format("{}_pageprev", this->name) ||
        parts[0] == fmt::format("{}_pageshow", this->name))
    {
        // Deconstruct custom id for state information
        const std::size_t type = std::stoul(parts[1]);
        std::size_t page = parts[2] != std::to_string(std::numeric_limits<std::size_t>::max()) ? std::stoul(parts[2]) : 0;
        const bool bShowComplete = parts[4] == "true" ? true : false;

        // Construct the actual info panel
        auto compare = [type](const std::shared_ptr<const JobRequest> job) -> bool { return job->SupportsType(type); };
        const std::size_t size = ctx.queue->GetQueueSize(bShowComplete, compare);

        if (size != 0)
        {
            const std::size_t lastPage = (size - 1) / JobQueue::JOBS_PER_QUEUE_PAGE;
            if (page > lastPage)
                page = lastPage;
        }
        else
        {
            page = 0;
        }

        const std::string result = ctx.queue->PrintPagedQueueCompact(ctx.cluster, event.command.guild_id, page, bShowComplete, compare);
        const std::string header = "Request Queue";

        PaginationPanel panel(ctx, this->name, type, page, size, JobQueue::JOBS_PER_QUEUE_PAGE, bShowComplete);
        panel.AddEmbed(header, size != 0 ? result : "Request queue is currently empty for this type.");

        // Edit the original message
        event.reply(dpp::ir_update_message, panel.set_flags(dpp::m_ephemeral));
    }
}