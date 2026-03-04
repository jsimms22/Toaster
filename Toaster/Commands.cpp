#include "Commands.h"
// fmt
#include <fmt/format.h>

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void ICustomCommand::RegisterAll(dpp::cluster* cluster, const std::vector<ICustomCommand*>& vCommands)
{
    if (!cluster) return;

    std::vector<dpp::slashcommand> slashCommands;
    for (auto* cmd : vCommands)
    {
        cmd->set_application_id(cluster->me.id);
        slashCommands.push_back(*cmd);
    }

    cluster->global_bulk_command_create(slashCommands);

    cluster->log(dpp::ll_debug, fmt::format("Created '{}' global commands.", vCommands.size()));
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void ICustomCommand::RegisterGuildAll(dpp::cluster* cluster, const dpp::snowflake idGuild, const std::vector<ICustomCommand*>& vCommands)
{
    if (!cluster) return;

    std::vector<dpp::slashcommand> slashCommands;
    for (auto* cmd : vCommands)
    {
        cmd->set_application_id(cluster->me.id);
        slashCommands.push_back(*cmd);
    }

    cluster->guild_bulk_command_create(slashCommands, idGuild);

    cluster->log(dpp::ll_debug, fmt::format("Created '{}' commands for guild id '{}'.", vCommands.size(), idGuild));
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void ICustomCommand::UnregisterAll(dpp::cluster* cluster)
{
    if (!cluster) return;

    cluster->global_bulk_command_delete();
    cluster->log(dpp::ll_debug, "Deleted global commands.");
}

//---------------------------------------------------------------------------------------------------------------------
// \brief 
//---------------------------------------------------------------------------------------------------------------------
void ICustomCommand::UnregisterGuildAll(dpp::cluster* cluster, const dpp::snowflake idGuild)
{
    if (!cluster) return;

    cluster->guild_bulk_command_delete(idGuild);
    cluster->log(dpp::ll_debug, fmt::format("Deleted commands for guild id '{}'.", idGuild));
}