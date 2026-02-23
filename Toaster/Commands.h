#pragma once
#include "CommandContext.h"
#include "Resource.h"
// d++
#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/snowflake.h>
// std library
#include <string>
#include <vector>

class ICustomCommand : public dpp::slashcommand
{
public:
    ICustomCommand(const std::string& name, const std::string& description)
        : dpp::slashcommand(name, description, {}) {}

    virtual ~ICustomCommand() = default;

    // Each command class must implement its execute logic
    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) = 0;
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) = 0;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) = 0;

    static void RegisterAll(dpp::cluster* cluster, const std::vector<ICustomCommand*>& vCommands);
    static void RegisterGuildAll(dpp::cluster* cluster, const dpp::snowflake idGuild, const std::vector<ICustomCommand*>& vCommands);
    static void UnregisterAll(dpp::cluster* cluster);
    static void UnregisterGuildAll(dpp::cluster* cluster, const dpp::snowflake idGuild);
};

class CreateRequestCommand : public ICustomCommand
{
public:
    CreateRequestCommand()
        : ICustomCommand(Command_JobRequest, "Create a request and add it to the queue.") 
    {
        add_option(dpp::command_option(dpp::co_string, Parameter_Type, "Select the type of job to submit.", true)
            .add_choice(dpp::command_option_choice("Crafting", Option_ItemCrafting))
            .add_choice(dpp::command_option_choice("Base Building", Option_BaseBuidling))
            .add_choice(dpp::command_option_choice("Component Request", Option_ComponentRequest))
            .add_choice(dpp::command_option_choice("Resource Collection", Option_ResourceCollect))
            .add_choice(dpp::command_option_choice("Refinery Job", Option_RefineryJob)));
    }

    virtual ~CreateRequestCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override;
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override;
};

class MyRequestsCommand : public ICustomCommand
{
public:
    MyRequestsCommand()
        : ICustomCommand(Command_MyRequests, "Retrieve a list of your requests and status.") 
    {
        add_option(dpp::command_option(dpp::co_string, Parameter_Type, "Specify whether to filter by a type.", true)
            .add_choice(dpp::command_option_choice("All", Option_All))
            .add_choice(dpp::command_option_choice("Crafting", Option_ItemCrafting))
            .add_choice(dpp::command_option_choice("Base Building", Option_BaseBuidling))
            .add_choice(dpp::command_option_choice("Component Request", Option_ComponentRequest))
            .add_choice(dpp::command_option_choice("Resource Collection", Option_ResourceCollect))
            .add_choice(dpp::command_option_choice("Refinery Job", Option_RefineryJob)));
    }

    virtual ~MyRequestsCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override;
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override;
};

class ShowQueueCommand : public ICustomCommand
{
public:
    ShowQueueCommand()
        : ICustomCommand(Command_ShowQueue, "Retrieve the current list of requests in the queue by a priority.")
    {
        add_option(dpp::command_option(dpp::co_string, Parameter_Type, "Specify whether to filter by a type.", true)
            .add_choice(dpp::command_option_choice("All", Option_All))
            .add_choice(dpp::command_option_choice("Crafting", Option_ItemCrafting))
            .add_choice(dpp::command_option_choice("Base Building", Option_BaseBuidling))
            .add_choice(dpp::command_option_choice("Component Request", Option_ComponentRequest))
            .add_choice(dpp::command_option_choice("Resource Collection", Option_ResourceCollect))
            .add_choice(dpp::command_option_choice("Refinery Job", Option_RefineryJob)));
    }

    virtual ~ShowQueueCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override;
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override;
};

class ShowQueueSummaryCommand : public ICustomCommand
{
public:
    ShowQueueSummaryCommand()
        : ICustomCommand(Command_ShowQueue, "Retrieve a summary for the state of the request in queue.")
    {}

    virtual ~ShowQueueSummaryCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override;
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override;
};

class ShowRequestCommand : public ICustomCommand
{
public:
    ShowRequestCommand()
        : ICustomCommand(Command_ShowRequest, "Display the information about the request.")
    {
        add_option(dpp::command_option(dpp::co_string, Parameter_Id, "Provide the request id you want to modify.", true));
    }

    virtual ~ShowRequestCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override;
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override;
};

class ModifyRequestCommand : public ICustomCommand
{
public:
    ModifyRequestCommand()
        : ICustomCommand(Command_ModifyRequest, "Edit a job request in queue by its id.") 
    {
        add_option(dpp::command_option(dpp::co_string, Parameter_Cmd, "Select the action to take.", true)
            .add_choice(dpp::command_option_choice("Edit Job", Option_Edit))
            .add_choice(dpp::command_option_choice("Assign Worker", Option_Assign))
            .add_choice(dpp::command_option_choice("Update Status", Option_Status))
            .add_choice(dpp::command_option_choice("Change Priority", Option_Priority))
            .add_choice(dpp::command_option_choice("Delete Job", Option_Delete)));
        add_option(dpp::command_option(dpp::co_string, Parameter_Id, "Provide the request id you want to modify.", true));
    }

    virtual ~ModifyRequestCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override;
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override;
};

class MyAssignmentsCommand : public ICustomCommand
{
public:
    MyAssignmentsCommand()
        : ICustomCommand(Command_MyAssignments, "Show a list of my current assignments.") {
    }

    virtual ~MyAssignmentsCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override;
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override;
};

class MyTopAssignmentCommand : public ICustomCommand
{
public:
    MyTopAssignmentCommand()
        : ICustomCommand(Command_MyTopAssignment, "Show my highest priority job assignment.") {}

    virtual ~MyTopAssignmentCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override;
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override;
};

class HelloCommand : public ICustomCommand
{
public:
    HelloCommand()
        : ICustomCommand(Command_Hello, "Hello there!") {}
    

    virtual ~HelloCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override;
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override;
};

namespace Toaster
{
    using CommandList = std::vector<ICustomCommand*>;
    static inline CommandList BotCommands
    {
        new CreateRequestCommand(),
        new MyRequestsCommand(),
        new ShowQueueCommand(),
        new ShowRequestCommand(),
        new ModifyRequestCommand(),
        new MyAssignmentsCommand(),
        new MyTopAssignmentCommand(),
        new HelloCommand()
    };
};