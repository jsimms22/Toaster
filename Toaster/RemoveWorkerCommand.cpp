#include "Commands.h"

#include "GuildSettings.h"
#include "PermissionsMgr.h"
#include "BotUtility.h"

#include "JobRequest.h"
#include "JobQueue.h"
// fmt
#include <fmt/format.h>
// std library
#include <string>

void RemoveWorkerCommand::ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    std::string rID = std::get<std::string>(event.get_parameter(Parameter_Id));
    std::string strUserID = std::get<std::string>(event.get_parameter(Parameter_User));

    utils::FilterWhiteSpace(rID);
    const auto job = ctx.queue->GetJobByID(rID);
    if (!job)
    {
        event.reply(dpp::message("This job was not found in the queue. It may have been deleted or archived.").set_flags(dpp::m_ephemeral));
        return;
    }

    const auto pManager = PermissionsMgr::GetInstance();
    if (!pManager->CanAssignJob(event, author.id, job, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) && !ctx.debug)
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("USER '{}' was DENIED access to use '{}' command", author.global_name, event.command.get_command_name()));
        return;
    }

    auto is_all_numbers = [](const std::string& s) -> bool {
        return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) {
            return std::isdigit(c);
            });
        };

    utils::FilterWhiteSpace(strUserID);
    utils::FilterCharacters(strUserID);

    if (!is_all_numbers(strUserID) || strUserID.empty())
    {
        event.reply(dpp::message("Invalid input for the user id. Must be all numeric characters or a user ping.").set_flags(dpp::m_ephemeral));
        return;
    }

    const dpp::snowflake worker{ strUserID };
    auto* pGuild = utils::FindGuildByID(ctx.cluster, event.command.guild_id);

    if (!(pManager->IsWorker(worker, pGuild, ctx.guild) || pManager->IsRequestWorker(worker, job)))
    {
        event.reply(dpp::message(fmt::format("User <@{}> does not have a worker role.\n", worker)).set_flags(dpp::m_ephemeral));
        return;
    }

    fmt::memory_buffer workerbuf;
    fmt::format_to(std::back_inserter(workerbuf), "<@{}>", worker);

    bool bResult = false;
    ctx.queue->RequestModify(job->GetID(),
        [worker, &bResult](std::shared_ptr<JobRequest> job)
        {
            bResult = job->RemoveWorkerID(worker);
        });

    fmt::memory_buffer msg;
    if (bResult)
    {
        fmt::format_to(std::back_inserter(msg), "Workers updated: {}\n", fmt::to_string(workerbuf));
    }
    else
    {
        event.reply(dpp::message(fmt::format("User <@{}> is not assigned to this job request.\n", worker)).set_flags(dpp::m_ephemeral));
        return;
    }

    if (msg.size() != 0)
    {
        const std::string jobDetails = job->PrintJobDetails(ctx.cluster, event.command.guild_id);

        const auto newJobStatus = job->GetStatus();

        dpp::embed embed;
        embed.set_title("Updated Assignments")
            .set_description(jobDetails)
            .set_color(0x3498db);

        event.reply(
            dpp::message(fmt::to_string(msg))
            .add_embed(embed)
            .set_flags(dpp::m_ephemeral));

        ctx.cluster.log(dpp::ll_info, fmt::format("USER '{}' updated '{}' with '{}'", author.global_name, rID, fmt::to_string(workerbuf)));

        const dpp::snowflake customer = job->GetCustomerID();
        if ((job->IsCustomerSubscribed() && author.id != customer) || ctx.debug)
        {
            utils::NotifyIssuerMsgWithEmbed(
                ctx.cluster,
                event,
                customer,
                fmt::format("{} were updated on your request by <@{}>.", fmt::to_string(workerbuf), author.id),
                "Request Details",
                jobDetails
            );
        }

        const std::string customMessage = "Worker list was updated on the request.";
        GuildSettings::AnnounceOnUpdate(ctx, job, author.id, customMessage);
    }
    else
    {
        event.reply(dpp::message("No worker ids added.").set_flags(dpp::m_ephemeral));
    }
}