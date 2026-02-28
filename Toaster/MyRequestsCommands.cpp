//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief Implementation of MyRequestsCommand.
///
/// Provides functionality for users to view and paginate through their own submitted job requests.
///
/// All results are scoped to the requesting user and validated
/// against the active JobQueue instance.
//---------------------------------------------------------------------------------------------------------------------
#include "BotUtility.h"
#include "Commands.h"
#include "JobQueue.h"
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

    const dpp::user author = event.command.get_issuing_user();
    const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Type));
    const std::size_t type = utils::CmdStringToJobType(strCmdID);

    if (ctx.queue->GetQueueSize() == 0)
    {
        dpp::embed embed;
        embed.set_title("Your Requests:")
            .set_description("No requests in queue.")
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

        return;
    }

    // acknowledge immediately to avoid timing out
    event.reply(dpp::message("...this could take a second. Please hold.").set_flags(dpp::m_ephemeral));

    dpp::message msg = BuildUserQueuePage(ctx, author.id, type, 0, event.command.guild_id);

    event.edit_original_response(msg.set_flags(dpp::m_ephemeral));
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Handles pagination button clicks for user request listings.
///
/// Expected custom_id format:"myrequests:<type>:<userid>:<page>"
///
/// Updates the original ephemeral message using ir_update_message.
///
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Button click interaction event.
//---------------------------------------------------------------------------------------------------------------------
void MyRequestsCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
    const std::string id = event.custom_id; // "myrequests:userid:page"

    if (id.starts_with("myrequests:"))
    {
        auto parts = utils::Split(id, ':');
        std::size_t type = std::stoul(parts[1]);
        dpp::snowflake user = parts[2];
        std::size_t page = parts[3] != std::to_string(std::numeric_limits<std::size_t>::max()) ? std::stoul(parts[3]) : 0;

        dpp::message msg = BuildUserQueuePage(ctx, user, type, page, event.command.guild_id);

        // Edit the original message
        event.reply(dpp::ir_update_message, msg.set_flags(dpp::m_ephemeral));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Builds a paginated embed message displaying a user's job requests.
///
/// Page bounds are clamped to prevent out-of-range access caused by malformed or manipulated component IDs.
///
/// \param[in,out] ctx      Command execution context.
/// \param[in] user         User whose requests are being displayed.
/// \param[in] type         Job type filter.
/// \param[in] page         Requested page index (0-based).
/// \param[in] guildID      Guild context for job detail formatting.
///
/// \return Fully constructed dpp::message ready for reply/update.
//---------------------------------------------------------------------------------------------------------------------
dpp::message MyRequestsCommand::BuildUserQueuePage(CommandContext& ctx, 
                                                  dpp::snowflake user, 
                                                  std::size_t type, 
                                                  std::size_t page, 
                                                  dpp::snowflake guildID) const
{

    const std::string result = ctx.queue->PrintQueuePageByUser(ctx.cluster, user, type, page, guildID);
    const std::size_t size = ctx.queue->GetFilteredQueueSizeByUser(user, type);
    const std::size_t lastPage = size <= 1 ? 0 : (size - 1) / JobQueue::JOBS_PER_DETAIL_PAGE;
    const std::string header = fmt::format("Your Requests (Page {} of {}):", page + 1, lastPage + 1);

    dpp::embed embed;
    embed.set_title(header)
        .set_description(size != 0 ? result : "You have no job requests in queue.")
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
            .set_id(fmt::format("myrequests:{}:{}:{}", type, user, prev_page)));

        row.add_component(
            dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Next")
            .set_style(dpp::cos_primary)
            .set_id(fmt::format("myrequests:{}:{}:{}", type, user, next_page)));
        msg.add_component(row);
    }

    return msg.add_embed(embed);
}