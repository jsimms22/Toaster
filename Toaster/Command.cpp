#include "Command.h"
#include "Resource.h"


BotModule::CommandList BotModule::commands
{
    {
        Command_JobRequest,
        dpp::slashcommand(Command_JobRequest, "Submit a job request to the queue.", {})
            .add_option(dpp::command_option(dpp::co_string, Parameter_Type, "Select the type of job to submit.", true)
                .add_choice(dpp::command_option_choice("Crafting", Option_ItemCrafting))
                .add_choice(dpp::command_option_choice("Base Building", Option_BaseBuidling))
                .add_choice(dpp::command_option_choice("Component Request", Option_ComponentRequest))
                .add_choice(dpp::command_option_choice("Resource Collection", Option_ResourceCollect))
                .add_choice(dpp::command_option_choice("Refinery Job", Option_RefineryJob)))
    },
    {
        Command_MyRequests,
        dpp::slashcommand(Command_MyRequests, "Retrieve a list of your requests and status.", {})
            .add_option(dpp::command_option(dpp::co_string, Parameter_Type, "Specify whether to filter by a type.", true)
                .add_choice(dpp::command_option_choice("All", Option_All))
                .add_choice(dpp::command_option_choice("Crafting", Option_ItemCrafting))
                .add_choice(dpp::command_option_choice("Base Building", Option_BaseBuidling))
                .add_choice(dpp::command_option_choice("Component Request", Option_ComponentRequest))
                .add_choice(dpp::command_option_choice("Resource Collection", Option_ResourceCollect))
                .add_choice(dpp::command_option_choice("Refinery Job", Option_RefineryJob)))
    },
    {
        Command_ShowQueue,
        dpp::slashcommand(Command_ShowQueue, "Retrieve the current list of requests in the queue by a priority.", {})
            .add_option(dpp::command_option(dpp::co_string, Parameter_Type, "Specify whether to filter by a type.", true)
                .add_choice(dpp::command_option_choice("All", Option_All))
                .add_choice(dpp::command_option_choice("Crafting", Option_ItemCrafting))
                .add_choice(dpp::command_option_choice("Base Building", Option_BaseBuidling))
                .add_choice(dpp::command_option_choice("Component Request", Option_ComponentRequest))
                .add_choice(dpp::command_option_choice("Resource Collection", Option_ResourceCollect))
                .add_choice(dpp::command_option_choice("Refinery Job", Option_RefineryJob)))
    },
    {
        Command_ShowRequest,
        dpp::slashcommand(Command_ShowRequest, "Display the information about the request.", {})
            .add_option(dpp::command_option(dpp::co_string, Parameter_Id, "Provide the request id you want to modify.", true))
    },
    {
        Command_ModifyRequest,
        dpp::slashcommand(Command_ModifyRequest, "Edit a job request in queue by its id.", {})
            .add_option(dpp::command_option(dpp::co_string, Parameter_Cmd, "Select the action to take.", true)
                .add_choice(dpp::command_option_choice("Edit Job", Option_Edit))
                .add_choice(dpp::command_option_choice("Assign Worker", Option_Assign))
                .add_choice(dpp::command_option_choice("Update Status", Option_Status))
                .add_choice(dpp::command_option_choice("Change Priority", Option_Priority))
                .add_choice(dpp::command_option_choice("Delete Job", Option_Delete)))
            .add_option(dpp::command_option(dpp::co_string, Parameter_Id, "Provide the request id you want to modify.", true))
    },
    {
        Command_MyAssignments,
        dpp::slashcommand(Command_MyAssignments, "Show a list of my current assignments.", {})
    },
    {
        Command_MyTopAssignment,
        dpp::slashcommand(Command_MyTopAssignment, "Show my highest priority assignment.", {})
    },
    {
        Command_Hello,
        dpp::slashcommand(Command_Hello, "Hello there!", {})
    }
};