#include "Commands.h"
#include "JobQueue.h"
#include "BotUtility.h"

void MyTopAssignmentCommand::ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    const std::string header = "Top Assigment (by priority):";
    if (ctx.queue->GetQueueSize() == 0)
    {
        dpp::embed embed;
        embed.set_title(header)
            .set_description("No requests or jobs currently assigned to you.")
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));

        return;
    }

    const auto& job = ctx.queue->FirstAssignment(author.username);
    const std::string result = job->PrintJobDetails(ctx.cluster);
    dpp::embed embed;
    embed.set_title(header)
        .set_description(!result.empty() ? result : "No requests or jobs currently assigned to you.")
        .set_color(0x3498db);

    dpp::message msg; 
    msg.add_embed(embed).set_flags(dpp::m_ephemeral);
    if (!result.empty())
    {
        dpp::component button1 = dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Complete")
            .set_style(dpp::cos_success)
            .set_id(Button_Complete)
            .set_content(utils::GuidToStringNoBrackets(job->GetID()));
        button1.value = utils::GuidToStringNoBrackets(job->GetID());

        dpp::component button2 = dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Add Note")
            .set_style(dpp::cos_primary)
            .set_id(Button_Note)
            .set_content(utils::GuidToStringNoBrackets(job->GetID()));

        dpp::component button3 = dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Unassign")
            .set_style(dpp::cos_primary)
            .set_id(Button_Unassign)
            .set_content(utils::GuidToStringNoBrackets(job->GetID()));

        dpp::component button4 = dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Delete")
            .set_style(dpp::cos_danger)
            .set_id(Button_Delete)
            .set_content(utils::GuidToStringNoBrackets(job->GetID()));

        dpp::component row = dpp::component()
            .set_type(dpp::cot_action_row)
            .add_component(button1)
            .add_component(button2)
            .add_component(button3)
            .add_component(button4);

        msg.add_component(row);
    }

    event.reply(msg);
}

void MyTopAssignmentCommand::ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event)
{
}

void MyTopAssignmentCommand::ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event)
{
}