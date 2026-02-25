#include "Commands.h"
#include "JobQueue.h"
#include "RequestDlg.h"
#include "BotUtility.h"
// fmt
#include <fmt/format.h>

void MyTopAssignmentCommand::ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event)
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
        return;
    }

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

    const auto& job = ctx.queue->FirstAssignment(author.id);
    const std::string result = job ? job->PrintJobDetails(ctx.cluster, event.command.guild_id) : "";
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
            .set_id(fmt::format("{}:{}:{}",Button_Complete, author.id, utils::GuidToStringNoBrackets(job->GetID())));

        dpp::component button2 = dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Add Note")
            .set_style(dpp::cos_primary)
            .set_id(fmt::format("{}:{}:{}", Button_Note, author.id, utils::GuidToStringNoBrackets(job->GetID())));

        dpp::component button3 = dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Unassign Me")
            .set_style(dpp::cos_primary)
            .set_id(fmt::format("{}:{}:{}", Button_Unassign, author.id, utils::GuidToStringNoBrackets(job->GetID())));

        dpp::component button4 = dpp::component()
            .set_type(dpp::cot_button)
            .set_label("Delete")
            .set_style(dpp::cos_danger)
            .set_id(fmt::format("{}:{}:{}", Button_Delete, author.id, utils::GuidToStringNoBrackets(job->GetID())));

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

void MyTopAssignmentCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
    const std::string id = event.custom_id; // "buttonType:workerid:guid"

    if (id.starts_with(fmt::format("{}:", Button_Complete)))
    {
        auto parts = utils::Split(id, ':');
        const dpp::snowflake worker = parts[1];
        const std::string guid = parts[2];

        auto job = ctx.queue->GetJobByGUID(guid);
        if (job && job->GetWorkerID() == worker && job->GetStatus() < JobRequest::status::complete)
        {
            job->SetStatus(JobRequest::status::complete);

            const dpp::snowflake customer = job->GetCustomerID();
            if ((job->IsCustomerSubscribed() && customer != worker) || ctx.debug)
            {
                utils::NotifyIssuerMsg(ctx.cluster, job->GetCustomerID(), event,
                    fmt::format("Request {} has been completed by {}.", guid, event.command.get_issuing_user().global_name));
            }

            ctx.cluster.log(dpp::ll_info, fmt::format("Request {} has been set to completed by {}.", utils::GuidToStringNoBrackets(job->GetID()), event.command.get_issuing_user().global_name));
            event.reply(dpp::message(fmt::format("Request {} has been set to completed.", utils::GuidToStringNoBrackets(job->GetID()))).set_flags(dpp::m_ephemeral));
        }
        else
        {
            event.reply(dpp::message("Could not find perform this action.").set_flags(dpp::m_ephemeral));
        }
    }
    else if (id.starts_with(fmt::format("{}:", Button_Note)))
    {
        auto parts = utils::Split(id, ':');
        dpp::snowflake worker = parts[1];
        const std::string guid = parts[2];

        auto job = ctx.queue->GetJobByGUID(guid);
        if (job && job->GetWorkerID() == worker)
        {
            /* todo add note functionality to job class */
            event.reply(dpp::message("Functionality currently not supported.").set_flags(dpp::m_ephemeral));
            return;

            ctx.queue->SaveQueueToFile();
            // todo note modal dlg
        }
        else
        {
            event.reply(dpp::message("Could not find perform this action.").set_flags(dpp::m_ephemeral));
        }
    }
    else if (id.starts_with(fmt::format("{}:", Button_Unassign)))
    {
        auto parts = utils::Split(id, ':');
        dpp::snowflake worker = parts[1];
        const std::string guid = parts[2];

        auto job = ctx.queue->GetJobByGUID(guid);
        if (job && job->GetWorkerID() == worker && job->GetStatus() < JobRequest::status::complete)
        {
            job->SetWorkerID(0);
            if (job->GetStatus() == JobRequest::status::active ||
                job->GetStatus() == JobRequest::status::assigned)
            {
                job->SetStatus(JobRequest::status::open);
            }

            const dpp::snowflake customer = job->GetCustomerID();
            if ((job->IsCustomerSubscribed() && customer != worker) || ctx.debug)
            {
                utils::NotifyIssuerMsg(ctx.cluster, job->GetCustomerID(), event,
                    fmt::format("Request {} has been unassigned by {}.", guid, event.command.get_issuing_user().global_name));
            }

            ctx.cluster.log(dpp::ll_info, fmt::format("Request {} has been set to unassigned by {}.", utils::GuidToStringNoBrackets(job->GetID()), event.command.get_issuing_user().global_name));
            event.reply(dpp::message(fmt::format("Request {} has been unassigned.", utils::GuidToStringNoBrackets(job->GetID()))).set_flags(dpp::m_ephemeral));
        }
        else
        {
            event.reply(dpp::message("Could not find perform this action.").set_flags(dpp::m_ephemeral));
        }
    }
    else if (id.starts_with(fmt::format("{}:", Button_Delete)))
    {
        auto parts = utils::Split(id, ':');
        dpp::snowflake worker = parts[1];
        const std::string guid = parts[2];

        auto job = ctx.queue->GetJobByGUID(guid);
        if (job && job->GetWorkerID() == worker && job->GetStatus() < JobRequest::status::complete)
        {
            DeleteRequestDlg modal(job, job->PrintJobDetails(ctx.cluster, event.command.guild_id));
            event.dialog(modal);
        }
        else
        {
            event.reply(dpp::message("Could not find perform this action.").set_flags(dpp::m_ephemeral));
        }
    }
}


