#include "Commands.h"
#include "JobQueue.h"

#include "RequestDlg.h"
// fmt
#include <fmt/format.h>

void CreateRequestCommand::ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event)
{
}

void CreateRequestCommand::ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    const std::string strCmdID = std::get<std::string>(event.get_parameter(Parameter_Type));

    if (strCmdID == Option_ItemCrafting)
    {
        CraftRequestDlg modal;
        event.dialog(modal);
    }
    else if (strCmdID == Option_ComponentRequest)
    {
        ComponentRequestDlg modal;
        event.dialog(modal);
    }
    else if (strCmdID == Option_BaseBuidling)
    {
        BuildRequestDlg modal;
        event.dialog(modal);
    }
    else if (strCmdID == Option_ResourceCollect)
    {
        ResourceRequestDlg modal;
        event.dialog(modal);
    }
    else if (strCmdID == Option_RefineryJob)
    {
        RefineryRequestDlg modal;
        event.dialog(modal);
    }

    ctx.cluster.log(dpp::ll_info, fmt::format("{} used command {} with cmd option {}", author.username, event.command.get_command_name(), strCmdID));
}

void CreateRequestCommand::ExecuteFormSubmit(CommandContext& ctx, const dpp::form_submit_t& event)
{
}