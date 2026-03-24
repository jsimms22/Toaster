#include "Commands.h"

#include "BotUtility.h"
#include "GuildSettings.h"
#include "PermissionsMgr.h"
#include "ShowWorkersPanel.h"
// d++
#include <dpp/guild.h>
#include <dpp/snowflake.h>
// fmt
#include <fmt/format.h>
// std library
#include <string>

void ShowWorkersCommand::ExecuteCommand(CommandContext& ctx, const dpp::slashcommand_t& event)
{
	if (event.command.get_command_name() != this->name)
	{
		return;
	}

	dpp::guild* guild = utils::FindGuildByID(ctx.cluster, event.command.guild_id);
	if (!guild) return;

	const auto pManager = PermissionsMgr::GetInstance();
	const auto& members = guild->members;
	const auto& role = ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Ping)];

	const std::string header = "### General Worker Role - ";
	const std::string roleStr = role.has_value() ? fmt::format("<@&{}>\n", role.value()) : "Role Not Set\n";
	std::string workerList;
	for (const auto& member : members)
	{
		if (role.has_value() && pManager->HasRole(member.second, role))
		{
			const std::string label = fmt::format("<@{}> ", member.first);
			workerList += fmt::format("- {} (`{}`)\n", label, member.first);
		}
	}

	ShowWorkersPanel workerlist{ this->name , ctx.guild, role };
	const std::string embedContent = header + roleStr +workerList;
	workerlist.AddEmbed("Assigned Worker Roles", embedContent);

	event.reply(workerlist.set_flags(dpp::m_ephemeral));
}

void ShowWorkersCommand::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
	const std::string id = event.custom_id;
	if (!id.starts_with(fmt::format("{}_", this->name)))
	{
		return;
	}

	auto parts = utils::Split(id, ':');

	dpp::guild* guild = utils::FindGuildByID(ctx.cluster, event.command.guild_id);
	if (!guild) return;

	const auto pManager = PermissionsMgr::GetInstance();
	const auto& members = guild->members;
	const auto& roleName = GuildSettings::RoleNames[std::stoull(parts[2])];
	const auto& role = ctx.guild->roles[std::stoull(parts[2])];

	const std::string header = fmt::format("### {} - ", roleName);
	const std::string roleStr = role.has_value() ? fmt::format("<@&{}>\n", role.value()) : "Role Not Set\n";
	std::string workerList;
	for (const auto& member : members)
	{
		if (pManager->HasRole(member.second, std::stoull(parts[1])))
		{
			const std::string label = fmt::format("<@{}>", member.first);
			workerList += fmt::format("- {} (`{}`)\n", label, member.first);
		}
	}

	ShowWorkersPanel workerlist{ Command_ShowWorkers, ctx.guild, std::stoull(parts[1]) };
	const std::string embedContent = fmt::format("{} {} {}", header, roleStr, workerList);
	workerlist.AddEmbed("Assigned Worker Roles", embedContent);

	event.reply(dpp::ir_update_message, workerlist.set_flags(dpp::m_ephemeral));
}