#include "Commands.h"
// fmt
#include <fmt/format.h>

void ICustomCommand::RegisterAll(dpp::cluster* cluster, const std::vector<ICustomCommand*>& vCommands)
{
    if (!cluster) return;

    for (auto* cmd : vCommands)
    {
        cmd->set_application_id(cluster->me.id);
        cluster->global_command_create(*cmd);
        Sleep(250); // Avoid rate limit
    }

    cluster->log(dpp::ll_debug, fmt::format("Created '{}' global commands.", vCommands.size()));
}

void ICustomCommand::RegisterGuildAll(dpp::cluster* cluster, const dpp::snowflake idGuild, const std::vector<ICustomCommand*>& vCommands)
{
    if (!cluster) return;

    for (auto* cmd : vCommands)
    {
        cmd->set_application_id(cluster->me.id);
        cluster->guild_command_create(*cmd, idGuild);
        Sleep(250); // Avoid rate limit
    }

    cluster->log(dpp::ll_debug, fmt::format("Created '{}' commands for guild id '{}'.", vCommands.size(), idGuild));
}

void ICustomCommand::UnregisterAll(dpp::cluster* cluster)
{
    if (!cluster) return;

    cluster->global_bulk_command_delete();
    cluster->log(dpp::ll_debug, "Deleted global commands.");
}

void ICustomCommand::UnregisterGuildAll(dpp::cluster* cluster, const dpp::snowflake idGuild)
{
    if (!cluster) return;

    cluster->guild_bulk_command_delete(idGuild);
    cluster->log(dpp::ll_debug, fmt::format("Deleted commands for guild id '{}'.", idGuild));
}