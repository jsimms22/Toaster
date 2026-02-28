//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief Implementation of ShowQueueCommand.
///
/// Provides a paginated view of the global job request queue filtered by type.
//---------------------------------------------------------------------------------------------------------------------
#include "BotUtility.h"
#include "Commands.h"
#include "JobQueue.h"
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

    const dpp::user author = event.command.get_issuing_user();
    if (!(ctx.manager->IsActiveWorker(author.id, ctx.workers) ||
          ctx.manager->IsGuildAdmin(author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id)) ||
          ctx.manager->IsBotOwner(author.id)))
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("USER '{}' was DENIED access to use '{}' command", author.id, event.command.get_command_name()));
        return;
    }

    const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Type));
    const std::size_t type = utils::CmdStringToJobType(strCmdID);

    if (ctx.queue->GetQueueSize() == 0)
    {
        dpp::embed embed;
        embed.set_title("Request Queue:")
            .set_description("Request queue is currently empty for this type.")
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

        return;
    }

    // acknowledge immediately to avoid timing out
    event.reply(dpp::message("...this could take a second. Please hold.").set_flags(dpp::m_ephemeral));

    dpp::message msg = BuildQueuePage(ctx, type, 0, event.command.guild_id);

    event.edit_original_response(msg.set_flags(dpp::m_ephemeral));
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Handles pagination button clicks for the global request queue.
///
/// Expected custom_id format: "queue:<type>:<page>"
///
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Button click interaction event.
//---------------------------------------------------------------------------------------------------------------------
void ShowQueueCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
    const std::string id = event.custom_id; // "queue:type:page"

    if (id.starts_with("queue:"))
    {
        auto parts = utils::Split(id, ':');
        const std::size_t type = std::stoul(parts[1]);
        std::size_t page = parts[2] != std::to_string(std::numeric_limits<std::size_t>::max()) ? std::stoul(parts[2]) : 0;

        dpp::message msg = BuildQueuePage(ctx, type, page, event.command.guild_id);

        // Edit the original message
        event.reply(dpp::ir_update_message, msg.set_flags(dpp::m_ephemeral));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Builds a paginated embed message displaying a user's job requests.
/// 
/// \param[in,out] ctx      Command execution context.
/// \param[in] user         User whose requests are being displayed.
/// \param[in] type         Job type filter.
/// \param[in] page         Requested page index (0-based).
/// \param[in] guildID      Guild context for job detail formatting.
///
/// \return Fully constructed dpp::message ready for reply/update.
//---------------------------------------------------------------------------------------------------------------------
dpp::message ShowQueueCommand::BuildQueuePage(CommandContext& ctx,
                                              std::size_t type,
                                              std::size_t page,
                                              dpp::snowflake guildID) const
{
    const std::string result = ctx.queue->PrintQueuePageByType(ctx.cluster, type, page, guildID);
    const std::size_t size = ctx.queue->GetFilteredQueueSizeByType(type);
    const std::size_t lastPage = size <= 1 ? 0 : (size - 1) / JobQueue::JOBS_PER_QUEUE_PAGE;
    const std::string header = fmt::format("Request Queue (Page {} of {}):", page + 1, lastPage + 1);

    dpp::embed embed;
    embed.set_title(header)
        .set_description(size != 0 ? result : "Request queue is currently empty for this type.")
        .set_color(0x3498db);

    const std::size_t prev_page = page > 0 ? page - 1 : 0;
    const std::size_t next_page = page < lastPage ? page + 1 : lastPage;

    dpp::message msg;
    if (prev_page != next_page)
    {
        dpp::component row;
        row.add_component(
            dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Prev")
            .set_style(dpp::cos_primary)
            .set_id(fmt::format("queue:{}:{}", type, prev_page)));

        row.add_component(
            dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Next")
            .set_style(dpp::cos_primary)
            .set_id(fmt::format("queue:{}:{}", type, next_page)));
        msg.add_component(row);
    }

    return msg.add_embed(embed);
}