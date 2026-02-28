//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief Implementation of the ShowQueueSummaryCommand.
///
/// Contains the implementation of ShowQueueSummaryCommand declared in Commands.h.
//---------------------------------------------------------------------------------------------------------------------
#include "BotUtility.h"
#include "Commands.h"
#include "JobQueue.h"
// fmt
#include <fmt/format.h>
// std library
#include <algorithm>

//---------------------------------------------------------------------------------------------------------------------
/// \brief Event handler for create interaction processes.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
//---------------------------------------------------------------------------------------------------------------------
void ShowQueueSummaryCommand::ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event)
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

    const std::string header = "Queue Summary:";
    if (ctx.queue->GetQueueSize() == 0)
    {
        dpp::embed embed;
        embed.set_title(header)
            .set_description("Request queue is currently empty.")
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

        return;
    }

    const std::string result = ctx.queue->PrintQueueSummary(ctx.cluster);
    dpp::embed embed;
    embed.set_title(header)
        .set_description(result)
        .set_color(0x3498db);

    dpp::message msg;
    msg.add_embed(embed).set_flags(dpp::m_ephemeral);
    if (!result.empty())
    {
        dpp::component button1 = dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Show Stalled Jobs")
            .set_style(dpp::cos_primary)
            .set_id(Button_Stalled);

        dpp::component button2 = dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Show Unassigned Jobs")
            .set_style(dpp::cos_primary)
            .set_id(Button_Unassigned);

        dpp::component row = dpp::component()
            .set_type(dpp::cot_action_row)
            .add_component(button1)
            .add_component(button2);

        msg.add_component(row);
    }

    event.reply(msg);
}

//---------------------------------------------------------------------------------------------------------------------
/// \brief Event handler for button interactions.
///
/// \param[out] ctx     Command context needed to handle various tasks.
/// \param[out] event   The event being processed for a response to the user.
//---------------------------------------------------------------------------------------------------------------------
void ShowQueueSummaryCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
    if (!ctx.queue)
    {
        return;
    }

    if (event.custom_id == Button_Stalled)
    {
        const dpp::user author = event.command.get_issuing_user();
        if (ctx.queue->GetQueueSize() == 0)
        {
            dpp::embed embed;
            embed.set_title("Stalled Jobs Report:")
                .set_description("Request queue is currently empty.")
                .set_color(0x3498db);

            event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

            return;
        }

        // acknowledge immediately to avoid timing out
        event.reply(dpp::message("...this could take a second. Please hold.").set_flags(dpp::m_ephemeral));

        std::size_t page = 0;
        const std::string result = ctx.queue->PrintQueuePageByStatus(ctx.cluster, JobRequest::status::stalled, page, event.command.guild_id);
        const std::size_t size = ctx.queue->GetFilteredQueueSizeByStatus(JobRequest::status::stalled);
        const std::size_t lastPage = size <= 1 ? 0 : (size - 1) / JobQueue::JOBS_PER_DETAIL_PAGE;
        const std::string header = fmt::format("Stalled Job Report (Page {} of {}):", page + 1, lastPage + 1);

        dpp::embed embed;
        embed.set_title(header)
            .set_description(size != 0 ? result : "No stalled jobs in queue.")
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
                .set_id(fmt::format("stalled:{}:{}", static_cast<int>(JobRequest::status::stalled), 0)));

            row.add_component(
                dpp::component()
                .set_type(dpp::cot_button)
                .set_label("Next")
                .set_style(dpp::cos_primary)
                .set_id(fmt::format("stalled:{}:{}", static_cast<int>(JobRequest::status::stalled), page + 1)));
            msg.add_component(row);
        }

        event.edit_original_response(msg.add_embed(embed).set_flags(dpp::m_ephemeral));

        return;
    }
    else if (event.custom_id == Button_Unassigned)
    {
        const dpp::user author = event.command.get_issuing_user();
        if (ctx.queue->GetQueueSize() == 0)
        {
            dpp::embed embed;
            embed.set_title("Unassigned Jobs Report:")
                .set_description("Request queue is currently empty.")
                .set_color(0x3498db);

            event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

            return;
        }

        // acknowledge immediately to avoid timing out
        event.reply(dpp::message("...this could take a second. Please hold.").set_flags(dpp::m_ephemeral));

        std::size_t page = 0;
        const std::string result = ctx.queue->PrintQueuePageByWorker(ctx.cluster, 0, page, event.command.guild_id);
        const std::size_t size = ctx.queue->GetFilteredQueueSizeByWorker(dpp::snowflake(0));
        const std::size_t lastPage = size <= 1 ? 0 : (size - 1) / JobQueue::JOBS_PER_DETAIL_PAGE;
        const std::string header = fmt::format("Unassigned Job Report (Page {} of {}):", page + 1, lastPage + 1);

        dpp::embed embed;
        embed.set_title(header)
            .set_description(size != 0 ? result : "No unassigned jobs in queue.")
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
                .set_id(fmt::format("unassigned:{}:{}", 0, 0)));

            row.add_component(
                dpp::component()
                .set_type(dpp::cot_button)
                .set_label("Next")
                .set_style(dpp::cos_primary)
                .set_id(fmt::format("unassigned:{}:{}", 0, page + 1)));
            msg.add_component(row);
        }

        event.edit_original_response(msg.add_embed(embed).set_flags(dpp::m_ephemeral));

        return;
    }

    const std::string id = event.custom_id; // "stalled:JobRequest::status:page"

    if (id.starts_with("stalled:"))
    {
        auto parts = utils::Split(id, ':');
        const JobRequest::status type = static_cast<JobRequest::status>(std::stoi(parts[1]));
        if (type != JobRequest::status::stalled) return;
        std::size_t page = parts[2] != std::to_string(std::numeric_limits<std::size_t>::max()) ? std::stoul(parts[2]) : 0;

        const std::string result = ctx.queue->PrintQueuePageByStatus(ctx.cluster, JobRequest::status::stalled, page, event.command.guild_id);
        const std::size_t size = ctx.queue->GetFilteredQueueSizeByStatus(JobRequest::status::stalled);
        const std::size_t lastPage = size <= 1 ? 0 : (size - 1) / JobQueue::JOBS_PER_DETAIL_PAGE;
        const std::string header = fmt::format("Stalled Job Report (Page {} of {}):", page + 1, lastPage + 1);

        dpp::embed embed;
        embed.set_title(header)
            .set_description(size != 0 ? result : "No stalled jobs in queue.")
            .set_color(0x3498db);

        dpp::message msg;
        if (size != 0)
        {
            const std::size_t prev_page = page > 0 ? page - 1 : 0;
            const std::size_t next_page = page < lastPage ? page + 1 : lastPage;
            if (prev_page != next_page)
            {
                dpp::component row;
                row.add_component(
                    dpp::component()
                    .set_type(dpp::cot_button)
                    .set_label("Prev")
                    .set_style(dpp::cos_primary)
                    .set_id(fmt::format("stalled:{}:{}", static_cast<int>(JobRequest::status::stalled), prev_page)));

                row.add_component(
                    dpp::component()
                    .set_type(dpp::cot_button)
                    .set_label("Next")
                    .set_style(dpp::cos_primary)
                    .set_id(fmt::format("stalled:{}:{}", static_cast<int>(JobRequest::status::stalled), next_page)));
                msg.add_component(row);
            }
        }

        // Edit the original message
        event.reply(dpp::ir_update_message, msg.add_embed(embed));

        return;
    }
    else if (id.starts_with("unassigned:"))
    {
        auto parts = utils::Split(id, ':');
        const dpp::snowflake worker = 0;
        if (worker != 0) return;
        std::size_t page = parts[2] != std::to_string(std::numeric_limits<std::size_t>::max()) ? std::stoul(parts[2]) : 0;

        const std::string result = ctx.queue->PrintQueuePageByWorker(ctx.cluster, 0, page, event.command.guild_id);
        const std::size_t size = ctx.queue->GetFilteredQueueSizeByWorker(dpp::snowflake(0));
        const std::size_t lastPage = size <= 1 ? 0 : (size - 1) / JobQueue::JOBS_PER_DETAIL_PAGE;
        const std::string header = fmt::format("Unassigned Job Report (Page {} of {}):", page + 1, lastPage + 1);

        dpp::embed embed;
        embed.set_title(header)
            .set_description(size != 0 ? result : "No unassigned jobs in queue.")
            .set_color(0x3498db);

        dpp::message msg;
        if (size != 0)
        {
            const std::size_t prev_page = page > 0 ? page - 1 : 0;
            const std::size_t next_page = page < lastPage ? page + 1 : lastPage;
            if (prev_page != next_page)
            {
                dpp::component row;
                row.add_component(
                    dpp::component()
                    .set_type(dpp::cot_button)
                    .set_label("Prev")
                    .set_style(dpp::cos_primary)
                    .set_id(fmt::format("unassigned:{}:{}", 0, prev_page)));

                row.add_component(
                    dpp::component()
                    .set_type(dpp::cot_button)
                    .set_label("Next")
                    .set_style(dpp::cos_primary)
                    .set_id(fmt::format("unassigned:{}:{}", 0, next_page)));
                msg.add_component(row);
            }
        }

        // Edit the original message
        event.reply(dpp::ir_update_message, msg.add_embed(embed));

        return;
    }
}