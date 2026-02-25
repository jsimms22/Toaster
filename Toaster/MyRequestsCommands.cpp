#include "BotUtility.h"
#include "Commands.h"
#include "JobQueue.h"
// fmt
#include <fmt/format.h>

void MyRequestsCommand::ExecuteCommand(CommandContext& ctx,  const dpp::slashcommand_t& event)
{
}

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

    std::size_t page = 0;
    const std::string result = ctx.queue->PrintQueuePageByUser(ctx.cluster, author.id, type, page, event.command.guild_id);
    const std::size_t size = ctx.queue->GetFilteredQueueSizeByUser(author.id, type);
    const std::size_t lastPage = size <= 1 ? 0 : (size - 1) / JobQueue::JOBS_PER_DETAIL_PAGE;
    const std::string header = fmt::format("Your Requests (Page {} of {}):", page + 1, lastPage + 1);

    dpp::embed embed;
    embed.set_title(header)
        .set_description(size != 0 ? result : "You have no job requests in queue.")
        .set_color(0x3498db);

    dpp::message msg;
    if (page != lastPage)
    {
        dpp::component row;
        row.add_component(
            dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Prev")
            .set_style(dpp::cos_primary)
            .set_id(fmt::format("myrequests:{}:{}", author.id, 0)));

        row.add_component(
            dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Next")
            .set_style(dpp::cos_primary)
            .set_id(fmt::format("myrequests:{}:{}", author.id, page + 1)));
        msg.add_component(row);
    }

    event.edit_original_response(msg.add_embed(embed).set_flags(dpp::m_ephemeral));
}

void MyRequestsCommand::ExecuteFormSubmit(CommandContext& ctx,  const dpp::form_submit_t& event)
{
}

void MyRequestsCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
    const std::string id = event.custom_id; // "myrequests:userid:page"

    if (id.starts_with("myrequests:"))
    {
        auto parts = utils::Split(id, ':');
        dpp::snowflake user = parts[1];
        std::size_t page = parts[2] != std::to_string(std::numeric_limits<std::size_t>::max()) ? std::stoul(parts[2]) : 0;

        const std::string result = ctx.queue->PrintQueuePageByUser(ctx.cluster, user, type, page, event.command.guild_id);
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
                .set_id(fmt::format("myrequests:{}:{}", user, prev_page)));

            row.add_component(
                dpp::component()
                .set_type(dpp::cot_button)
                .set_label("Next")
                .set_style(dpp::cos_primary)
                .set_id(fmt::format("myrequests:{}:{}", user, next_page)));
            msg.add_component(row);
        }

        // Edit the original message
        event.reply(dpp::ir_update_message, msg.add_embed(embed));
    }
}