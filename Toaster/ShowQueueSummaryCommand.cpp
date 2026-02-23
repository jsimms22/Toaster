#include "BotUtility.h"
#include "Commands.h"
#include "JobQueue.h"
// fmt
#include <fmt/format.h>

void ShowQueueSummaryCommand::ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event)
{
}

void ShowQueueSummaryCommand::ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
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
        .set_description(!result.empty() ? result : "No requests of this type in queue.")
        .set_color(0x3498db);

    dpp::message msg;
    msg.add_embed(embed).set_flags(dpp::m_ephemeral);
    if (!result.empty())
    {
        dpp::component button1 = dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Show Stalled Jobs")
            .set_style(dpp::cos_success)
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

void ShowQueueSummaryCommand::ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event)
{
}

void ShowQueueSummaryCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
    if (!ctx.queue)
    {
        return;
    }

    if (event.custom_id == Button_Stalled)
    {
        const dpp::user author = event.command.get_issuing_user();
        const std::string header = "Stalled Jobs Report:";
        if (ctx.queue->GetQueueSize() == 0)
        {
            dpp::embed embed;
            embed.set_title(header)
                .set_description("Request queue is currently empty.")
                .set_color(0x3498db);

            event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

            return;
        }

        const std::string result = ctx.queue->PrintQueueByStatus(ctx.cluster, JobRequest::status::stalled);
        dpp::embed embed;
        embed.set_title(header)
            .set_description(!result.empty() ? result : "No requests of this type in queue.")
            .set_color(0x3498db);

        dpp::message msg;
        msg.add_embed(embed).set_flags(dpp::m_ephemeral);
        event.reply(msg);
    }
    else if (event.custom_id == Button_Unassigned)
    {
        const dpp::user author = event.command.get_issuing_user();
        const std::string header = "Unassigned Jobs Report:";
        if (ctx.queue->GetQueueSize() == 0)
        {
            dpp::embed embed;
            embed.set_title(header)
                .set_description("Request queue is currently empty.")
                .set_color(0x3498db);

            event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

            return;
        }

        const std::string result = ctx.queue->PrintQueueByWorker(ctx.cluster, 0);
        dpp::embed embed;
        embed.set_title(header)
            .set_description(!result.empty() ? result : "No requests of this type in queue.")
            .set_color(0x3498db);

        dpp::message msg;
        msg.add_embed(embed).set_flags(dpp::m_ephemeral);
        event.reply(msg);
    }
}