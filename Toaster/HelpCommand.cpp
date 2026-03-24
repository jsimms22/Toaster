#include "Commands.h"
// fmt
#include <fmt/format.h>
// std library
#include <string>


namespace
{
//--------------------------------------------------------------
// Customer Help
//--------------------------------------------------------------
std::string CreateCustomerHelp()
{
    return fmt::format(
        "Customers can create and manage the job requests they generate."
        " Each customer is limited to a number of job requests determined"
        " by the discord admins. Job requests that are in completed"
        " or on hold status do not count against this capacity.\n"

        "### Available Commands:\n"

        "**/create_request**\n"
        "Create a new request and add it to the queue."
        " You will be asked to provide details about the job."
        " After a request has been generated you may subscribe to the request"
        " for notifications on changes made to the request by workers or managers.\n\n"

        "**/modify_request**\n"
        "Edit or delete one of your submitted requests.\n\n"

        "**/my_requests**\n"
        "View all requests you have submitted and check their status.\n\n"

        "**/show_request**\n"
        "Display detailed information about a specific request using its ID.\n"

        "### Job Status Levels:\n"
        "Requests move through several statuses as work progresses.\n"

        "- **Open**\n"
        "  The request has been created and is waiting for a worker.\n"

        "- **Assigned**\n"
        "  A worker has claimed or been assigned to the request.\n"

        "- **Active**\n"
        "  Work on the request is currently in progress.\n"

        "- **Stalled**\n"
        "  Progress is temporarily blocked (waiting for materials, information, etc.).\n"

        "- **On Hold**\n"
        "  The request has been intentionally paused.\n"

        "- **Complete**\n"
        "  The job has been finished and the request is closed.\n\n"

        "### Priority Levels:\n"
        "Priority helps workers understand how urgently a request is needed.\n"

        "- **Low**\n"
        "  When convenient.\n"
        "- **Medium**\n"
        "  Need the item soon.\n"
        "- **High**\n"
        "  Need the item today.\n"
        "- **Critical**\n"
        "  Need the item as soon as possible.\n\n"

        "You may request a priority when creating or modifying a job."
        " However, workers and managers reserve the right to adjust the priority"
        " based on workload, fairness, and resource availability."
    );
}

//--------------------------------------------------------------
// Worker Help
//--------------------------------------------------------------
std::string CreateWorkerHelp()
{
    return fmt::format(
        "Workers can view and manage assignments from the queue.\n\n"

        "**Customer Commands (also available):**\n"
        "/help\n"
        "/job_request\n"
        "/modify_request\n"
        "/my_requests\n"
        "/show_request\n"

        "### Worker Tools:\n"

        "**/show_queue**\n"
        "Display the current job queue by priority."
        " You can filter by job type.\n\n"

        "**/summary_queue**\n"
        "Shows a summarized overview of the current queue."
        " There are buttons to view currently unassigned or stalled jobs.\n\n"

        "**/show_workers**\n"
        "Displays lists of members who currently have worker roles assigned.\n\n"

        "**/add_worker**\n"
        "Adds a user as a worker to the given job request.\n\n"

        "**/remove_worker**\n"
        "Remove a user as a worker from the given job request.\n\n"

        "**/worker_panel**\n"
        "Displays your worker assignment portal. Panels:\n"
        "- Assignment Overview\n"
        "- All Assignments\n\n"

        "**/modify_request**\n"
        "Workers have full control over requests in the queue. Available actions:\n"
        "- Edit Job\n"
        "- Assign Worker\n"
        "- Update Status\n"
        "- Change Priority\n"
        "- Delete Job\n"

        "### Available Worker Roles You Can Sign Up For:\n"
        "Workers typically specialize in one or more job categories.\n"

        "- **Item Crafter**\n"
        "  Craft requested weapons, tools, gear, or other items.\n"

        "- **Base Builder**\n"
        "  Construct bases, structures, or defensive setups.\n"

        "- **Component Dealer**\n"
        "  Supply crafted or traded components required for builds.\n"

        "- **Resource Gatherer**\n"
        "  Collect raw materials like ore, wood, stone, etc.\n"

        "- **Refinery Worker**\n"
        "  Process raw resources into refined materials.\n"

        "- **Hazardous Materials Collector**\n"
        "  Retrieve dangerous or difficult-to-obtain materials.\n"
        "  These requests generally require combat or other hazards to procure.\n"

        "### Tip:\n"
        "Choose roles that match the type of jobs you want to receive from the queue."
        " Ask a manager or admin if you need a role assigned."
    );
}

//--------------------------------------------------------------
// Admin / Manager Help
//--------------------------------------------------------------
std::string CreateAdminHelp()
{
    return fmt::format(
        "Managers oversee the job queue, assign workers, and configure the system."
        " They have access to all customer and worker commands, plus administrative tools.\n"

        "### Customer Commands (also available):\n"
        "/help\n"
        "/job_request\n"
        "/modify_request\n"
        "/my_requests\n"
        "/show_request\n"

        "### Worker Commands (also available):\n"
        "/show_queue\n"
        "/summary_queue\n"
        "/show_workers\n"
        "/add_worker\n"
        "/remove_worker\n"
        "/worker_panel\n"
        "/modify_request\n"

        "### Manager Tools:\n"

        "**/manager_portal**\n"
        "Opens the administrative portal. Panels:\n"
        "- Bot Panel\n"
        "- Queue Panel\n\n"

        "**/admin_configroles**\n"
        "Assign or update the Discord roles used by the system. Roles include:\n"
        "- General Worker\n"
        "- Item Crafter\n"
        "- Base Builder\n"
        "- Component Dealer\n"
        "- Resource Gatherer\n"
        "- Refinery Worker\n"
        "- Hazardous Materials Collector\n"
        "- Manager\n\n"

        "**/admin_resetroles**\n"
        "Reset a Discord role used by the system.\n\n"

        "**/admin_configchannels**\n"
        "Configure which Discord channels receive job announcements.\n\n"

        "**/admin_resetchannels**\n"
        "Reset a Discord channel for receiving job announcements.\n\n"

        "**/admin_configping**\n"
        "Configure the worker ping role and notification rules for job announcements.\n"

        "### Queue Management Responsibilities:\n"
        "Managers help keep the system fair and running smoothly."
        " This may include:\n"
        "- Verifying requests are being assigned to workers\n"
        "- Updating job status when progress changes as needed\n"
        "- Adjusting job priorities when necessary\n"
        "- Determining when to archive old, and completed requests\n"

        "### Priority and Status Authority:\n"
        "Customers may request a priority when submitting a job."
        " However, managers and workers reserve the right to adjust"
        " priority or status based on workload, fairness, and available resources.\n"

        "### Tip:\n"
        "There are a number of input fields that accept raw id values for either roles,"
        " channels, or users. It may be beneficial for managers or admins to use Discord's"
        " dev mode option."
    );
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
/// \brief Executes the Help slash command.
///
/// Validates the command name and replies with an ephemeral message containing a dynamically generated list of 
/// available bot commands.
///
/// \param[in,out] ctx   Command execution context.
/// \param[in] event     Slash command invocation event.
//---------------------------------------------------------------------------------------------------------------------
void HelpCommand::ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Cmd));

    if (strCmdID == Option_HelpCustomer)
    {
        dpp::embed embed;
        embed.set_title("Customer Help Page")
            .set_description(CreateCustomerHelp())
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
    }
    else if (strCmdID == Option_HelpWorker)
    {
        dpp::embed embed;
        embed.set_title("Worker Help Page")
            .set_description(CreateWorkerHelp())
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
    }
    else if (strCmdID == Option_HelpAdmin)
    {
        dpp::embed embed;
        embed.set_title("Admin Help Page")
            .set_description(CreateAdminHelp())
            .set_color(0x3498db);

        event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
    }
}