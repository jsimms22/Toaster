#include "AdminConfigDialog.h"

#include "Commands.h"
#include "GuildSettings.h"
// std library
#include <string>

const std::string AdminConfigDialog::modalID = "AdminConfigModal";
const std::string AdminConfigDialog::modalDesc = "Bot Per Server Config Settings";

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void AdminConfigDialog::InitializeControls()
{
	// For ping rules
	if (m_strCommand == Command_ConfigPing)
	{
		PingRoleEdit.set_label("General Ping Role")
			.set_type(dpp::cot_text)
			.set_placeholder("role id")
			.set_min_length(0)
			.set_max_length(40)
			.set_text_style(dpp::text_short)
			.set_id(Component_PingRole);

		if (m_ctx.guild->HasPingRole())
		{
			PingRoleEdit.set_default_value(std::to_string(m_ctx.guild->roles[static_cast<std::size_t>(GuildSettings::Roles::Ping)].value_or(0)));
		}

		OnNewSelect.set_label("Ping on New Request")
			.set_type(dpp::cot_selectmenu)
			.add_select_option(dpp::select_option("True", "1", "Ping when this event occurs.").set_default(m_ctx.guild->bPingOnNew))
			.add_select_option(dpp::select_option("False", "0", "Do not ping.").set_default(!m_ctx.guild->bPingOnNew))
			.set_id(Component_PingOnNew);

		OnUpdateSelect.set_label("Ping on Request Updated")
			.set_type(dpp::cot_selectmenu)
			.add_select_option(dpp::select_option("True", "1", "Ping when this event occurs.").set_default(m_ctx.guild->bPingOnUpdate))
			.add_select_option(dpp::select_option("False", "0", "Do not ping.").set_default(!m_ctx.guild->bPingOnUpdate))
			.set_id(Component_PingOnUpdate);

		OnDeleteSelect.set_label("Ping on Request Deleted")
			.set_type(dpp::cot_selectmenu)
			.add_select_option(dpp::select_option("True", "1", "Ping when this event occurs.").set_default(m_ctx.guild->bPingOnDelete))
			.add_select_option(dpp::select_option("False", "0", "Do not ping.").set_default(!m_ctx.guild->bPingOnDelete))
			.set_id(Component_PingOnDelete);

		OnCompleteSelect.set_label("Ping on Request Completed")
			.set_type(dpp::cot_selectmenu)
			.add_select_option(dpp::select_option("True", "1", "Ping when this event occurs.").set_default(m_ctx.guild->bPingOnComplete))
			.add_select_option(dpp::select_option("False", "0", "Do not ping.").set_default(!m_ctx.guild->bPingOnComplete))
			.set_id(Component_PingOnComplete);
	}
	// For channel announcement rules
	else if (m_strCommand == Command_ConfigChannels)
	{
		NewChannelEdit.set_label("New Request Announcements")
			.set_type(dpp::cot_text)
			.set_placeholder("channel id")
			.set_min_length(0)
			.set_max_length(40)
			.set_text_style(dpp::text_short)
			.set_id(Component_NewChannel);

		if (m_ctx.guild->idNewJobChannel.has_value())
		{
			NewChannelEdit.set_default_value(std::to_string(m_ctx.guild->idNewJobChannel.value_or(0)));
		}

		UpdateChannelEdit.set_label("Request Updated Announcements")
			.set_type(dpp::cot_text)
			.set_placeholder("channel id")
			.set_min_length(0)
			.set_max_length(40)
			.set_text_style(dpp::text_short)
			.set_id(Component_EditChannel);

		if (m_ctx.guild->idUpdateJobChannel.has_value())
		{
			UpdateChannelEdit.set_default_value(std::to_string(m_ctx.guild->idUpdateJobChannel.value_or(0)));
		}

		DeleteChannelEdit.set_label("Request Deleted Announcements")
			.set_type(dpp::cot_text)
			.set_placeholder("channel id")
			.set_min_length(0)
			.set_max_length(40)
			.set_text_style(dpp::text_short)
			.set_id(Component_DeletedChannel);

		if (m_ctx.guild->idDeleteJobChannel.has_value())
		{
			DeleteChannelEdit.set_default_value(std::to_string(m_ctx.guild->idDeleteJobChannel.value_or(0)));
		}

		CompleteChannelEdit.set_label("Request Completed Announcements")
			.set_type(dpp::cot_text)
			.set_placeholder("channel id")
			.set_min_length(0)
			.set_max_length(40)
			.set_text_style(dpp::text_short)
			.set_id(Component_CompletedChannel);

		if (m_ctx.guild->idCompleteJobChannel.has_value())
		{
			CompleteChannelEdit.set_default_value(std::to_string(m_ctx.guild->idCompleteJobChannel.value_or(0)));
		}
	}
	// For roles
	else if (m_strCommand == Command_ConfigRoles)
	{
		RoleSelect.set_label("Select Role to Set")
			.set_type(dpp::cot_selectmenu)
			.add_select_option(dpp::select_option("General Worker", std::to_string(static_cast<int>(GuildSettings::Roles::Ping)), ""))
			.add_select_option(dpp::select_option("Item Crafter", std::to_string(static_cast<int>(GuildSettings::Roles::Crafter)), ""))
			.add_select_option(dpp::select_option("Base Builder", std::to_string(static_cast<int>(GuildSettings::Roles::Builder)), ""))
			.add_select_option(dpp::select_option("Component Dealer", std::to_string(static_cast<int>(GuildSettings::Roles::Comp)), ""))
			.add_select_option(dpp::select_option("Resource Gatherer", std::to_string(static_cast<int>(GuildSettings::Roles::Gatherer)), ""))
			.add_select_option(dpp::select_option("Refinery Worker", std::to_string(static_cast<int>(GuildSettings::Roles::Refiner)), ""))
			.add_select_option(dpp::select_option("Hazardous Materials Collector", std::to_string(static_cast<int>(GuildSettings::Roles::Hazmat)), ""))
			.add_select_option(dpp::select_option("Manager", std::to_string(static_cast<int>(GuildSettings::Roles::Manager)), ""))
			.set_id(Component_RoleSelect);

		RoleEdit.set_label("Request Completed Announcements")
			.set_type(dpp::cot_text)
			.set_placeholder("Role id")
			.set_min_length(0)
			.set_max_length(40)
			.set_text_style(dpp::text_short)
			.set_id(Component_RoleEdit);
	}
}

//---------------------------------------------------------------------------------------------------------------------
// \brief
//---------------------------------------------------------------------------------------------------------------------
void AdminConfigDialog::AddChildrenComponents()
{
	// For ping rules
	if (m_strCommand == Command_ConfigPing)
	{
		add_component(PingRoleEdit);
		add_component(OnNewSelect);
		add_component(OnUpdateSelect);
		add_component(OnDeleteSelect);
		add_component(OnCompleteSelect);
	}
	// For channel announcement rules
	else if (m_strCommand == Command_ConfigChannels)
	{
		add_component(NewChannelEdit);
		add_component(UpdateChannelEdit);
		add_component(DeleteChannelEdit);
		add_component(CompleteChannelEdit);
	}
	// For roles
	else if (m_strCommand == Command_ConfigRoles)
	{
		add_component(RoleSelect);
		add_component(RoleEdit);
	}
}