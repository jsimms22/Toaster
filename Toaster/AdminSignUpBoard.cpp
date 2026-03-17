#include "Commands.h"

#include "GuildSettings.h"
#include "PermissionsMgr.h"
#include "BotUtility.h"
// d++
#include <dpp/guild.h>
// fmt
#include <fmt/format.h>
// std library
#include <string>
#include <unordered_set>
#include <vector>

namespace
{
struct RoleButton
{
    std::string label;
    dpp::snowflake role_id;
    dpp::component_style style = dpp::cos_primary;
};

std::string CreateWorkerRoleOverview(const std::vector<RoleButton>& buttons)
{
    auto findRoleId = [](const std::string& name, const std::vector<RoleButton>& btns) -> std::string {
        for (const auto& b : btns)
        {
            if (b.label == name && b.role_id != 0)
                return "<@&" + std::to_string(b.role_id) + ">";
        }
        return "Not Set";
        };

    return fmt::format(
        "### Worker Roles Overview\n"
        "Workers help fulfill job requests by specializing in specific tasks. "
        "Most roles focus on a particular skill set and will only be pinged for the relevant requests:\n\n"

        "- **General Worker** - {} \nCan take on a variety of tasks without a specialty.\n"
        "- **Item Crafter** - {} \nCreate weapons, ships, gear, or other craftable items requested.\n"
        "- **Base Builder** - {} \nConstruct structures, outposts, and bases. Support infrastructure type requests.\n"
        "- **Component Supplier** - {} \nSupply crafted or traded components for vehicle customization.\n"
        "- **Resource Harvester** - {} \nCollect raw materials such as salvage material, ore, or organic harvestables.\n"
        "- **Refinery Worker** - {} \nProcess raw resources into usable materials.\n\n"

        "Choose roles that match the type of jobs you want to receive. "
        "You can select multiple roles. Click the button again to toggle the ping role when off-duty.",
        findRoleId("General Worker", buttons),
        findRoleId("Item Crafter", buttons),
        findRoleId("Base Builder", buttons),
        findRoleId("Component Supplier", buttons),
        findRoleId("Resouce Harvester", buttons),
        findRoleId("Refinery Worker", buttons)
    );
}

void FilterRoleButtons(std::vector<RoleButton>& buttons)
{
    std::unordered_set<dpp::snowflake> seen;

    buttons.erase(
        std::remove_if(
            buttons.begin(),
            buttons.end(),
            [&](const RoleButton& btn)
            {
                if (btn.role_id == 0)
                    return true;

                if (!seen.insert(btn.role_id).second)
                    return true; // duplicate

                return false;
            }
        ),
        buttons.end()
    );
}

dpp::message BuildRoleBoard(
    dpp::snowflake channel_id,
    const std::string& title,
    std::vector<RoleButton>& buttons
)
{
    dpp::embed embed;
    embed.set_title(title)
        .set_description(CreateWorkerRoleOverview(buttons))
        .set_color(0x3498db);

    dpp::message msg(channel_id, "");
    msg.add_embed(embed);

    FilterRoleButtons(buttons);

    const size_t buttons_per_row = 3;

    for (size_t i = 0; i < buttons.size(); i += buttons_per_row)
    {
        dpp::component row;
        row.set_type(dpp::cot_action_row);

        for (size_t j = i; j < i + buttons_per_row && j < buttons.size(); j++)
        {
            const RoleButton& b = buttons[j];

            row.add_component(
                dpp::component()
                .set_type(dpp::cot_button)
                .set_label(b.label)
                .set_style(b.style)
                .set_id("rolebtn:" + std::to_string(b.role_id))
            );
        }

        msg.add_component(row);
    }

    return msg;
}
} // namespace

void AdminSignUpBoard::ExecuteInteraction(CommandContext& ctx, const dpp::interaction_create_t& event)
{
    if (!ctx.queue || event.command.get_command_name() != this->name)
    {
        return;
    }

    const dpp::user author = event.command.get_issuing_user();
    std::string channelID = std::get<std::string>(event.get_parameter(Parameter_Id));

    const auto pManager = PermissionsMgr::GetInstance();
    if (!pManager->CanAccessAdminPanel(event, author.id, utils::FindGuildByID(ctx.cluster, event.command.guild_id), ctx.guild) && !ctx.debug)
    {
        event.reply(dpp::message("You do not have sufficient permissions to perform this action.").set_flags(dpp::m_ephemeral));
        ctx.cluster.log(dpp::ll_warning, fmt::format("USER '{}' was DENIED access to use '{}' command", author.id, event.command.get_command_name()));
        return;
    }

    auto is_all_numbers = [](const std::string& s) -> bool {
        return !s.empty() && std::all_of(s.begin(), s.end(), [](unsigned char c) {
            return std::isdigit(c);
            });
        };

    utils::FilterWhiteSpace(channelID);

    if (!is_all_numbers(channelID) && !channelID.empty())
    {
        event.reply(dpp::message("Invalid input for the channel id. Must be all numeric characters.").set_flags(dpp::m_ephemeral));
        return;
    }

    std::vector<RoleButton> buttons =
    {
        {"General Worker", ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Ping)].value_or(0)},
        {"Item Crafter", ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Crafter)].value_or(0)},
        {"Base Builder", ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Builder)].value_or(0)},
        {"Component Supplier", ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Comp)].value_or(0)},
        {"Resouce Harvester", ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Gatherer)].value_or(0)},
        {"Refinery Worker", ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Refiner)].value_or(0)}
        /* purposely left hazmat off of here */
    };

    dpp::message msg = BuildRoleBoard(
        channelID,
        "Logistics - Job Board Ping Roles",
        buttons
    );

    ctx.cluster.message_create(msg);

    event.reply(dpp::message("Posting sign-up board in <#" + channelID + ">. This Discord bot needs to have a role assigned that is higher in order than roles being assigned to users.").set_flags(dpp::m_ephemeral));
}

void AdminSignUpBoard::ExecuteButtonClick(CommandContext& ctx, const dpp::button_click_t& event)
{
    const std::string& id = event.custom_id;

    const std::string prefix = "rolebtn:";
    if (!id.starts_with("rolebtn:"))
        return;

    dpp::snowflake role_id = std::stoull(id.substr(prefix.length()));
    dpp::snowflake guild_id = event.command.guild_id;
    dpp::snowflake user_id = event.command.get_issuing_user().id;
    dpp::guild_member member = event.command.member;

    const auto& roles = member.get_roles();

    bool has_role = std::find(roles.begin(), roles.end(), role_id) != roles.end();

    std::string msg;
    if (has_role)
    {
        ctx.cluster.guild_member_remove_role(guild_id, user_id, role_id);
        msg = "Removed role <@&" + std::to_string(role_id) + ">.";
    }
    else
    {
        ctx.cluster.guild_member_add_role(guild_id, user_id, role_id);
        msg = "Assigned role <@&" + std::to_string(role_id) + ">.";
    }

    event.reply(dpp::message(msg).set_flags(dpp::m_ephemeral));
}
