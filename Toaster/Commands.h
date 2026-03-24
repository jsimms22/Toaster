//---------------------------------------------------------------------------------------------------------------------
/// \file
/// \brief Defines the custom slash command framework and concrete bot commands.
///
/// This file declares the ICustomCommand interface and all concrete command implementations used by the bot. 
/// Each command encapsulates its own slash command definition, interaction handling, form processing, and button
/// click logic.
//---------------------------------------------------------------------------------------------------------------------
#pragma once
#include "CommandContext.h"
#include "Resource.h"
// d++
#include <dpp/appcommand.h>
#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/snowflake.h>
// std library
#include <memory>
#include <string>
#include <vector>

class JobRequest;

//---------------------------------------------------------------------------------------------------------------------
/// \class ICustomCommand
/// \brief Abstract base class for all custom slash commands.
///
/// Extends dpp::slashcommand and enforces a common execution interface for slash commands, interactions, modal 
/// submissions, and button clicks.
///
/// All derived command classes must implement the execution handlers.
//---------------------------------------------------------------------------------------------------------------------
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
    virtual void ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) = 0;

    static void RegisterAll(dpp::cluster* cluster, const std::vector<ICustomCommand*>& vCommands);
    static void RegisterGuildAll(dpp::cluster* cluster, const dpp::snowflake idGuild, const std::vector<ICustomCommand*>& vCommands);
    static void UnregisterAll(dpp::cluster* cluster);
    static void UnregisterGuildAll(dpp::cluster* cluster, const dpp::snowflake idGuild);
};

//---------------------------------------------------------------------------------------------------------------------
/// \class AdminPanelCommand
/// \brief Displays administrative control panels.
///
/// Provides bot and queue management panels for administrators.
//---------------------------------------------------------------------------------------------------------------------
class AdminConfigChannelsCommand : public ICustomCommand
{
public:
    AdminConfigChannelsCommand()
        : ICustomCommand(Command_ConfigChannels, "Set channel ids for various request announcements.")
    {
        add_option(dpp::command_option(dpp::co_string, Parameter_Type, "Specify the channel announcement to set.", true)
            .add_choice(dpp::command_option_choice("New Requests", Option_NewRequest))
            .add_choice(dpp::command_option_choice("Request Edited", Option_EditRequest))
            .add_choice(dpp::command_option_choice("Request Deleted", Option_DeleteRequest))
            .add_choice(dpp::command_option_choice("Request Completed", Option_CompleteRequest)));
        add_option(dpp::command_option(dpp::co_string, Parameter_Channel, "Channel ID.", true));
    }

    virtual ~AdminConfigChannelsCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override;
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override;
    virtual void ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) override;
};

//---------------------------------------------------------------------------------------------------------------------
/// \class AdminPanelCommand
/// \brief Displays administrative control panels.
///
/// Provides bot and queue management panels for administrators.
//---------------------------------------------------------------------------------------------------------------------
class AdminResetChannelsCommand : public ICustomCommand
{
public:
    AdminResetChannelsCommand()
        : ICustomCommand(Command_ResetChannels, "Reset channel ids for various request announcements.")
    {
        add_option(dpp::command_option(dpp::co_string, Parameter_Type, "Specify the channel announcement to reset.", true)
            .add_choice(dpp::command_option_choice("New Requests", Option_NewRequest))
            .add_choice(dpp::command_option_choice("Request Edited", Option_EditRequest))
            .add_choice(dpp::command_option_choice("Request Deleted", Option_DeleteRequest))
            .add_choice(dpp::command_option_choice("Request Completed", Option_CompleteRequest)));
    }

    virtual ~AdminResetChannelsCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override;
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override {}
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override {}
    virtual void ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) override {}
};

//---------------------------------------------------------------------------------------------------------------------
/// \class AdminPanelCommand
/// \brief Displays administrative control panels.
///
/// Provides bot and queue management panels for administrators.
//---------------------------------------------------------------------------------------------------------------------
class AdminConfigPingCommand : public ICustomCommand
{
public:
    AdminConfigPingCommand()
        : ICustomCommand(Command_ConfigPing, "Set ping role and rules.")
    {
        add_option(dpp::command_option(dpp::co_string, Parameter_Type, "Specify the ping rule to change.", true)
            .add_choice(dpp::command_option_choice("New Requests", Option_NewRequest))
            .add_choice(dpp::command_option_choice("Request Edited", Option_EditRequest))
            .add_choice(dpp::command_option_choice("Request Deleted", Option_DeleteRequest))
            .add_choice(dpp::command_option_choice("Request Completed", Option_CompleteRequest)));
        add_option(dpp::command_option(dpp::co_boolean, Parameter_Bool, "True or False", true));
    }

    virtual ~AdminConfigPingCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override;
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override;
    virtual void ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) override;
};

//---------------------------------------------------------------------------------------------------------------------
/// \class AdminPanelCommand
/// \brief Displays administrative control panels.
///
/// Provides bot and queue management panels for administrators.
//---------------------------------------------------------------------------------------------------------------------
class AdminConfigRolesCommand : public ICustomCommand
{
public:
    AdminConfigRolesCommand()
        : ICustomCommand(Command_ConfigRoles, "Set roles for various worker archetypes.")
    {
        add_option(dpp::command_option(dpp::co_string, Parameter_Type, "Specify the role to set for worker archetypes.", true)
            .add_choice(dpp::command_option_choice("General Worker", Option_GeneralRole))
            .add_choice(dpp::command_option_choice("Item Crafter", Option_CraftingRole))
            .add_choice(dpp::command_option_choice("Base Builder", Option_BuildingRole))
            .add_choice(dpp::command_option_choice("Component Dealer", Option_CompDealerRole))
            .add_choice(dpp::command_option_choice("Resource Gatherer", Option_ResourceRole))
            .add_choice(dpp::command_option_choice("Refinery Worker", Option_RefiningRole))
            .add_choice(dpp::command_option_choice("Hazardous Materials Collector", Option_HazmatRole))
            .add_choice(dpp::command_option_choice("Manager", Option_ManagerRole)));
        add_option(dpp::command_option(dpp::co_string, Parameter_Role, "Role ID.", true));
    }

    virtual ~AdminConfigRolesCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override;
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override {}
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override;
    virtual void ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) override {}
};

//---------------------------------------------------------------------------------------------------------------------
/// \class AdminPanelCommand
/// \brief Displays administrative control panels.
///
/// Provides bot and queue management panels for administrators.
//---------------------------------------------------------------------------------------------------------------------
class AdminResetRolesCommand : public ICustomCommand
{
public:
    AdminResetRolesCommand()
        : ICustomCommand(Command_ResetRoles, "Reset roles for the various worker archetypes.")
    {
        add_option(dpp::command_option(dpp::co_string, Parameter_Type, "Specify the role to reset.", true)
            .add_choice(dpp::command_option_choice("General Worker", Option_GeneralRole))
            .add_choice(dpp::command_option_choice("Item Crafter", Option_CraftingRole))
            .add_choice(dpp::command_option_choice("Base Builder", Option_BuildingRole))
            .add_choice(dpp::command_option_choice("Component Dealer", Option_CompDealerRole))
            .add_choice(dpp::command_option_choice("Resource Gatherer", Option_ResourceRole))
            .add_choice(dpp::command_option_choice("Refinery Worker", Option_RefiningRole))
            .add_choice(dpp::command_option_choice("Hazardous Materials Collector", Option_HazmatRole))
            .add_choice(dpp::command_option_choice("Manager", Option_ManagerRole)));
    }

    virtual ~AdminResetRolesCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override;
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override {}
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override {}
    virtual void ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) override {}
};

//---------------------------------------------------------------------------------------------------------------------
/// \class AdminPanelCommand
/// \brief Displays administrative control panels.
///
/// Provides bot and queue management panels for administrators.
//---------------------------------------------------------------------------------------------------------------------
class AdminPanelCommand : public ICustomCommand
{
public:
    AdminPanelCommand()
        : ICustomCommand(Command_Admin, "Show the administration panels.")
    {
        add_option(dpp::command_option(dpp::co_string, Parameter_Type, "Specify Admin Control Panel.", true)
            .add_choice(dpp::command_option_choice("Bot Panel", Option_Bot))
            .add_choice(dpp::command_option_choice("Queue Panel", Option_Queue)));
    }

    virtual ~AdminPanelCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override {}
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override {}
    virtual void ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) override;

private:
    const std::string CreateBotEmbed(CommandContext& ctx, const dpp::interaction_create_t& event) const;
    dpp::message CreateBotPanel(CommandContext& ctx, const dpp::interaction_create_t& event) const;

    const std::string CreateQueueEmbed(CommandContext& ctx, const dpp::interaction_create_t& event) const;
    dpp::message CreateQueuePanel(CommandContext& ctx, const dpp::interaction_create_t& event, const std::size_t page) const;
};

class AdminSignUpBoard : public ICustomCommand
{
public:
    AdminSignUpBoard()
        : ICustomCommand(Command_WorkerSignUp, "Post a billboard in a channel to allow users to sign up for worker roles.")
    {
        add_option(dpp::command_option(dpp::co_string, Parameter_Channel, "Specify the channel id to post in.", true));
    }

    virtual ~AdminSignUpBoard() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override {}
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override {}
    virtual void ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) override;

private:

};

//---------------------------------------------------------------------------------------------------------------------
/// \class WorkerPanelCommand
/// \brief Displays assignment panels for workers.
//---------------------------------------------------------------------------------------------------------------------
class WorkerPanelCommand : public ICustomCommand
{
public:
    WorkerPanelCommand()
        : ICustomCommand(Command_Worker, "Show your assignment panels.")
    {
        add_option(dpp::command_option(dpp::co_string, Parameter_Type, "Specify a panel.", true)
            .add_choice(dpp::command_option_choice("Assignment Overview", Option_Overview))
            .add_choice(dpp::command_option_choice("All Assignments", Option_AllAssignments)));
    }

    virtual ~WorkerPanelCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override {}
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override {}
    virtual void ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) override;

private:
    dpp::message SendPanel(
        CommandContext& ctx, 
        const dpp::interaction_create_t& event, 
        const dpp::snowflake& user,
        const std::size_t page) const;
};

//---------------------------------------------------------------------------------------------------------------------
/// \class CreateRequestCommand
/// \brief Creates a new job request and adds it to the queue.
//---------------------------------------------------------------------------------------------------------------------
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
            .add_choice(dpp::command_option_choice("Refinery Job", Option_RefineryJob))
            .add_choice(dpp::command_option_choice("Hazardous Items Collection", Option_HazardousJob)));
    }

    virtual ~CreateRequestCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override {}
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override;
    virtual void ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) override;

private:
    dpp::message SendPanel(CommandContext& ctx, const dpp::interaction_create_t& event, const std::shared_ptr<const JobRequest>& job, const dpp::snowflake& user) const;

};

//---------------------------------------------------------------------------------------------------------------------
/// \class MyRequestsCommand
/// \brief Retrieves and displays requests submitted by the current user.
//---------------------------------------------------------------------------------------------------------------------
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

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override {}
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override {}
    virtual void ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) override;

};

//---------------------------------------------------------------------------------------------------------------------
/// \class ShowQueueCommand
/// \brief Displays the request queue with optional filtering and pagination.
//---------------------------------------------------------------------------------------------------------------------
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

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override {}
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override {}
    virtual void ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) override;
};

//---------------------------------------------------------------------------------------------------------------------
/// \class ShowQueueSummaryCommand
/// \brief Displays a summarized overview of the request queue.
//---------------------------------------------------------------------------------------------------------------------
class ShowQueueSummaryCommand : public ICustomCommand
{
public:
    ShowQueueSummaryCommand()
        : ICustomCommand(Command_SummaryQueue, "Display a summary for the queue.")
    {}

    virtual ~ShowQueueSummaryCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override {}
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override {}
    virtual void ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) override;
};

//---------------------------------------------------------------------------------------------------------------------
/// \class ShowRequestCommand
/// \brief Displays detailed information about a specific request.
//---------------------------------------------------------------------------------------------------------------------
class ShowRequestCommand : public ICustomCommand
{
public:
    ShowRequestCommand()
        : ICustomCommand(Command_ShowRequest, "Display the information about a request by its id.")
    {
        add_option(dpp::command_option(dpp::co_string, Parameter_Id, "Provide the request id you want to modify.", true));
    }

    virtual ~ShowRequestCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override {}
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override {}
    virtual void ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) override;

private:
    dpp::message SendPanel(CommandContext& ctx, const dpp::interaction_create_t& event, const std::shared_ptr<const JobRequest>& job, const dpp::snowflake& user) const;
};

//---------------------------------------------------------------------------------------------------------------------
/// \class ShowRequestCommand
/// \brief Displays detailed information about a specific request.
//---------------------------------------------------------------------------------------------------------------------
class ShowWorkersCommand : public ICustomCommand
{
public:
    ShowWorkersCommand()
        : ICustomCommand(Command_ShowWorkers, "Displays workers with active roles.")
    {
    }

    virtual ~ShowWorkersCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override;
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override {}
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override {}
    virtual void ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) override;

private:
};

//---------------------------------------------------------------------------------------------------------------------
/// \class ModifyRequestCommand
/// \brief Modifies an existing job request (edit, assign, update status, etc.).
//---------------------------------------------------------------------------------------------------------------------
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

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override {}
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override;
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override;
    virtual void ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) override;

private:
    dpp::message SendPanel(CommandContext& ctx, const dpp::interaction_create_t& event, const std::shared_ptr<const JobRequest>& job, const dpp::snowflake& user) const;
};

//---------------------------------------------------------------------------------------------------------------------
/// \class HelpCommand
/// \brief Lists all available bot commands.
//---------------------------------------------------------------------------------------------------------------------
class HelpCommand : public ICustomCommand
{
public:
    HelpCommand()
        : ICustomCommand(Command_Help, "Displays help documentation for customers, workers, or managers.")
    {
        add_option(dpp::command_option(dpp::co_string, Parameter_Cmd, "Select the help doc to view.", true)
            .add_choice(dpp::command_option_choice("Customer Help Page", Option_HelpCustomer))
            .add_choice(dpp::command_option_choice("Worker Help Page", Option_HelpWorker))
            .add_choice(dpp::command_option_choice("Manager Help Page", Option_HelpAdmin)));
    }

    virtual ~HelpCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override;
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override {}
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override {}
    virtual void ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) override {}
};

//---------------------------------------------------------------------------------------------------------------------
/// \class AddWorkerCommand
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class AddWorkerCommand : public ICustomCommand
{
public:
    AddWorkerCommand()
        : ICustomCommand(Command_AddWorker, "Assign a worker to a job request with either a ping mention or their id.")
    {
        add_option(dpp::command_option(dpp::co_string, Parameter_Id, "Specify the id of the job request.", true));
        add_option(dpp::command_option(dpp::co_string, Parameter_User, "Specify the user to assign", true));
    }

    virtual ~AddWorkerCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override;
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override {}
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override {}
    virtual void ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) override {}
};

//---------------------------------------------------------------------------------------------------------------------
/// \class RemoveWorkerCommand
/// \brief 
//---------------------------------------------------------------------------------------------------------------------
class RemoveWorkerCommand : public ICustomCommand
{
public:
    RemoveWorkerCommand()
        : ICustomCommand(Command_RemoveWorker, "Unassign a worker from a job request with either a ping mention or their id.")
    {
        add_option(dpp::command_option(dpp::co_string, Parameter_Id, "Specify the id of the job request.", true));
        add_option(dpp::command_option(dpp::co_string, Parameter_User, "Specify the user to unassign", true));
    }

    virtual ~RemoveWorkerCommand() = default;

    virtual void ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event) override;
    virtual void ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event) override {}
    virtual void ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event) override {}
    virtual void ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event) override {}
};

//---------------------------------------------------------------------------------------------------------------------
/// \namespace Toaster
/// \brief Contains global command registrations for the bot.
///
/// Holds the static list of instantiated bot commands used during command registration.
//---------------------------------------------------------------------------------------------------------------------
namespace Toaster
{
    using CommandList = std::vector<ICustomCommand*>;
    static inline CommandList BotCommands
    {
        new AdminConfigChannelsCommand(),
        new AdminResetChannelsCommand(),
        new AdminConfigPingCommand(),
        new AdminConfigRolesCommand(),
        new AdminResetRolesCommand(),
        new AdminSignUpBoard(),
        new AdminPanelCommand(),
        new WorkerPanelCommand(),
        new ShowQueueCommand(),
        new ShowWorkersCommand(),
        new ShowQueueSummaryCommand(),
        new HelpCommand(),
        new CreateRequestCommand(),
        new ModifyRequestCommand(),
        new AddWorkerCommand(),
        new RemoveWorkerCommand(),
        new MyRequestsCommand(),
        new ShowRequestCommand()
    };
};